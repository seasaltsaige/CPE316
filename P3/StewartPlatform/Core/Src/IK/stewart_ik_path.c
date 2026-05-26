#include "IK/stewart_ik_path.h"
#include "IK/ik_math.h"
#include <math.h>

// simple clamp function
static float clampf(float val, float low, float high) {
    if (val < low) return low;
    if (val > high) return high;
    return val;
}

// simple lerp helper function
static float lerpf(float a, float b, float dt) {
    return a + ((b - a) * dt);
}

// position lerp using above lerp helper
static Pose_t pose_lerp(Pose_t pos_a, Pose_t pos_b, float dt) {
    Pose_t out = {
        .pos = {
            .x = lerpf(pos_a.pos.x, pos_b.pos.x, dt),
            .y = lerpf(pos_a.pos.y, pos_b.pos.y, dt),
            .z = lerpf(pos_a.pos.z, pos_b.pos.z, dt),
        },
        .rot = {
            .x = lerpf(pos_a.rot.x, pos_b.rot.x, dt), // roll
            .y = lerpf(pos_a.rot.y, pos_b.rot.y, dt), // pitch
            .z = lerpf(pos_a.rot.z, pos_b.rot.z, dt), // yaw
        },
    };
    return out;
}

static float catmullf(float p0, float p1, float p2, float p3, float dt) {
    float t_2 = dt*dt;
    float t_3 = t_2*dt;

    return 0.5f * ( (2*p1) + (-p0 +p2)*dt + (2*p0 - 5*p1 + 4*p2 - p3)*t_2 + (-p0 + 3*p1 - 3*p2 + p3)*t_3 );
}