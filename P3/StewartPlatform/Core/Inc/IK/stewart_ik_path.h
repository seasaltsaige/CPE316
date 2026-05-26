#ifndef __STEWART_IK_PATH_H
#define __STEWART_IK_PATH_H

#include <stdint.h>
#include "ik_math.h"

#define CATMULL_MAX_POINTS 16
#define PATH_QUEUE_MAX 16

typedef struct {
    Vec3_t pos; // x, y, z, mm position
    Vec3_t rot; // x = roll, y = pitch, z = yaw
} Pose_t;

typedef struct {
    float t;
    float max_speed;
    float accel;
    float decel;
    uint8_t pause;
} MotionProfile_t;

// motion
MotionProfile_t create_motion_profile(float dur, float accel, float decel);
float motion_tick(MotionProfile_t *prof, float dt);
uint8_t motion_complete(const MotionProfile_t *prof);

// path segment handling

typedef enum {
    SEG_LINEAR, // straight path from a - b
    SEG_CIRCLE, // circle/ellipse through space
    SEG_CATMULL, // catmull-rom spline path definition
} SegmentType_t;

typedef struct {
    SegmentType_t type;
    MotionProfile_t profile;
    uint8_t looping; // only applies to circles or paths that start/end in the same 3d point 

    // Linear segment defs
    Pose_t linear_start;
    Pose_t linear_end;

    // Ellipse segment defs
    float circ_cx; 
    float circ_cy;

    float circ_rx;            // radius along major axis (mm)
    float circ_ry;            // radius along minor axis (mm)

    float circ_tilt;          // rotation of ellipse in XY plane (rad)
    float circ_z;             // fixed height
    float circ_roll;          // fixed orientation
    float circ_pitch;
    float circ_yaw;
    float circ_start_angle;   // starting angle (rad)


    // catmull-rom segment defs
    Pose_t catmull_points[CATMULL_MAX_POINTS];
    uint8_t catmull_point_count; // (0 to CATMULL_MAX_POINTS)
} PathSeg_t;

PathSeg_t segment_linear(Pose_t start, Pose_t end, float dur, float accel, float decel);
PathSeg_t segment_ellipse(Pose_t start, float cx, float cy, float rx, float ry, float tilt, float revs_per_sec, int looping);
PathSeg_t segment_catmull(Pose_t *points, uint8_t count, float dur, float accel, float decel);

Pose_t path_segment_evalutate(cosnt PathSeg_t *segment);

// path scheduler/move queue

typedef struct {
    PathSeg_t queue[PATH_QUEUE_MAX];

    uint8_t head; // current index in queue
    uint8_t seg_count; // total number of segments in queue
    Pose_t current_pose; // current evaluated pose
    uint8_t running; // 1 while running, 0 while paused
} PathScheduler_t;

// init and add sequences
void scheduler_init(PathScheduler_t *schedule, Pose_t start_pose);
int8_t scheduler_push(PathScheduler_t *schedule, PathSeg_t segment);
void scheduler_clear(PathScheduler_t *schedule);

// pause/resume current path
void scheduler_pause(PathScheduler_t *schedule);
void scheduler_resume(PathScheduler_t *schedule);

// number of path segments currently queued
uint8_t scheduler_queued_count(PathScheduler_t *schedule);

// main processing tick, called every dt seconds (1/200hz for example)
// pose_out is target position currently moving towards
uint8_t schedule_update(PathScheduler_t *schedule, float dt, Pose_t *pose_out);

#endif