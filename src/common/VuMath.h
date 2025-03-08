#pragma once
#include "Common.h"

namespace Vu {
    static float3 QuatMul(const quat& q, const float3& v) {
        // Extract the vector part of the quaternion
        float3 q_xyz = float3(q.x, q.y, q.z);

        // Compute the cross product of q.xyz and v
        float3 t = 2.0f * glm::cross(q_xyz, v);

        // Return the transformed vector
        return v + q.w * t + glm::cross(q_xyz, t);
    }
}
