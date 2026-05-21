#ifndef __STEWART_IK_H
#define __STEWART_IK_H

#include "ik_math.h"
#include <stdint.h>
#define LEGS 6

typedef struct {
    Vec3_t base[LEGS]; // connection points from base reference
    Vec3_t platform[LEGS]; // connection points from platform reference

    float home_leg_len[LEGS];
    float min_leg_len[LEGS];
    float max_leg_len[LEGS];

    uint32_t steps_per_mm;
} StewartPlatform_t;


typedef struct {
    uint32_t steps[LEGS]; // absolute step offset for each leg
    float leg_lens[LEGS]; // absolute target lengths for each leg
    int8_t clamped; // 1 if any leg was clamped to min/max, 0 otherwise
} StewartIKResult_t;


int8_t stewart_platform_ik(const StewartPlatform_t *plat, Vec3_t trans_vec, const Mat3_t *rot_mat, StewartIKResult_t *ik_res);

// wrapper for above function, instead of manually building rot_mat, instead pass roll, pitch, and yaw, and let the program do it
int8_t stewart_platform_ik_angles(const StewartPlatform_t *plat, Vec3_t trans_vec, Vec3_t rot_vec, StewartIKResult_t *ik_res);


#endif