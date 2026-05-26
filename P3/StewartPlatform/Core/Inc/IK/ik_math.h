#ifndef __IK_MATH_H
#define __IK_MATH_H

// math uses z-y-x cordinate convention!!!!

#include "arm_math.h"
#include <math.h>


typedef struct {
    float x, y, z;
} Vec3_t;

// VecA - VecB
static inline Vec3_t vec3_sub(Vec3_t a, Vec3_t b) {
    return (Vec3_t){ 
        a.x-b.x,
        a.y-b.y,
        a.z-b.z 
    };
}

// VecA + VecB
static inline Vec3_t vec3_add(Vec3_t a, Vec3_t b) {
    return (Vec3_t){ 
        a.x+b.x,
        a.y+b.y,
        a.z+b.z 
    };
}

static inline float vec3_len(Vec3_t vec) {
    return sqrtf(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
}

typedef struct {
    float m[3][3];
} Mat3_t;


static inline Vec3_t mat3_mul_vec3(Mat3_t *mat, Vec3_t vec) {
    Vec3_t vec = {
        .x = ((mat->m[0][0] * vec.x) + (mat->m[0][1] * vec.y) + (mat->m[0][2] * vec.z)),
        .y = ((mat->m[1][0] * vec.x) + (mat->m[1][1] * vec.y) + (mat->m[1][2] * vec.z)),
        .z = ((mat->m[2][0] * vec.x) + (mat->m[2][1] * vec.y) + (mat->m[2][2] * vec.z)),
    };
    return vec;
}

// euler_angles: x = roll, y = pitch, z = yaw
static inline Mat3_t mat3_from_euler(Vec3_t euler_angles) {

    float cp = cosf(euler_angles.y);
    float sp = sinf(euler_angles.y);
    float cr = cosf(euler_angles.x);
    float sr = sinf(euler_angles.x);
    float cy = cosf(euler_angles.z);
    float sy = sinf(euler_angles.z);

    Mat3_t rot = {
        .m = {
            { cy*cp, cy*sp*sr - sy*cr, cy*sp*cr + sy*sr },
            { sy*cp, sy*sp*sr + cy*cr, sy*sp*cr - cy*sr },
            { -sp, cp*sr, cp*cr },
        }
    };
    return rot;
}

// simple 3x3 identity matrix
static inline Mat3_t mat3_identity() {
    return (Mat3_t){
        .m = {
            { 1, 0, 0 },
            { 0, 1, 0 },
            { 0, 0, 1 },
        }
    };
}

#endif