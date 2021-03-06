#ifndef SPHEREH
#define SPHEREH

#include "hitable.h"

__device__ static void get_sphere_uv(const vec3& p, float& u, float& v) {
    float phi = atan2f(p.z(), p.x());
    float thelta = asinf(p.y());
    u = 1 - (phi + CUDART_PI) / (CUDART_PI * 2);
    v = (thelta + CUDART_PIO2) / CUDART_PI;
}

class sphere: public hitable  {
    public:
        __device__ sphere() {}
        __device__ sphere(vec3 cen, float r, material *m) : center(cen), radius(r), mat_ptr(m)  {};
        __device__ bool hit(const ray& r, float tmin, float tmax, hit_record& rec) const override;
        __device__ bool bounding_box(float t0, float t1, aabb& box) const override;
        vec3 center;
        float radius;
        material *mat_ptr;
};

__device__ bool sphere::hit(const ray& r, float t_min, float t_max, hit_record& rec) const {
    vec3 oc = r.origin() - center;
    float a = dot(r.direction(), r.direction());
    float b = dot(oc, r.direction());
    float c = dot(oc, oc) - radius*radius;
    float discriminant = b*b - a*c;
    if (discriminant > 0) {
        float temp = (-b - sqrt(discriminant))/a;
        if (temp < t_max && temp > t_min) {
            rec.t = temp;
            rec.p = r.point_at_parameter(rec.t);
            rec.normal = (rec.p - center) / radius;
            rec.mat_ptr = mat_ptr;
            get_sphere_uv(rec.normal, rec.u, rec.v);
            return true;
        }
        temp = (-b + sqrt(discriminant)) / a;
        if (temp < t_max && temp > t_min) {
            rec.t = temp;
            rec.p = r.point_at_parameter(rec.t);
            rec.normal = (rec.p - center) / radius;
            rec.mat_ptr = mat_ptr;
            get_sphere_uv(rec.normal, rec.u, rec.v);
            return true;
        }
    }
    return false;
}

__device__ bool sphere::bounding_box(float t0, float t1, aabb& box) const {
    box = aabb(center - vec3(radius, radius, radius), center + vec3(radius, radius, radius));
    return true;
}

class moving_sphere :public hitable
{
public:
    __device__ moving_sphere() {}
    __device__ moving_sphere(vec3 cen0, vec3 cen1, float time0, float time1, float r, material* m) : center0(cen0), center1(cen1), time0(time0), time1(time1), radius(r), mat_ptr(m) {};
    __device__ bool hit(const ray& r, float tmin, float tmax, hit_record& rec) const override;
    __device__ vec3 center(float time) const {
        return center0 + ((time - time0) / (time1 - time0)) * (center1 - center0);
    }
    __device__ bool bounding_box(float t0, float  t1, aabb& box) const override {
        box = aabb(center0 - vec3(radius, radius, radius), center0 + vec3(radius, radius, radius));
        aabb box2 = aabb(center1 - vec3(radius, radius, radius), center1 + vec3(radius, radius, radius));
        box = surrounding_box(box, box2);
        return true;
    }
    float time0;
    float time1;
    vec3 center0;
    vec3 center1;
    float radius;
    material* mat_ptr;
};

__device__ bool moving_sphere:: hit(const ray& r, float t_min, float t_max, hit_record& rec) const
{
    vec3 center = moving_sphere::center(r.time());
    vec3 oc = r.origin() - center;
    float a = dot(r.direction(), r.direction());
    float b = dot(oc, r.direction());
    float c = dot(oc, oc) - radius * radius;
    float discriminant = b * b - a * c;
    if (discriminant > 0) {
        float temp = (-b - sqrt(discriminant)) / a;
        if (temp < t_max && temp > t_min) {
            rec.t = temp;
            rec.p = r.point_at_parameter(rec.t);
            rec.normal = (rec.p - center) / radius;
            rec.mat_ptr = mat_ptr;
            get_sphere_uv(rec.normal, rec.u, rec.v);
            return true;
        }
        temp = (-b + sqrt(discriminant)) / a;
        if (temp < t_max && temp > t_min) {
            rec.t = temp;
            rec.p = r.point_at_parameter(rec.t);
            rec.normal = (rec.p - center) / radius;
            rec.mat_ptr = mat_ptr;
            get_sphere_uv(rec.normal, rec.u, rec.v);
            return true;
        }
    }
    return false;
}

#endif
