#ifndef __IK_MATH_H
#define __IK_MATH_H


#include "arm_math.h"
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
    float output;
    arm_sqrt_f32(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z, &output);
    return output;
}

typedef struct {
    float m[3][3];
} Mat3_t;


static inline Vec3_t mat3_mul_vec3(Mat3_t mat, Vec3_t vec) {
    
}

#endif