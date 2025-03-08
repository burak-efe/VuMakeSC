#pragma once
#include "Common.h"
#include "glm/gtx/quaternion.hpp"

namespace Vu {
    struct Transform {
        float3 Position = float3(0.0f, 0.0f, 0.0f);
        quat Rotation = glm::identity<quat>();
        float3 Scale = float3(1.0f, 1.0f, 1.0f);

        void Rotate(const float3& axis, float angle) {
            Rotation = glm::rotate(Rotation, angle, axis);
        }

        float4x4 ToTRS() {
            // Initialize transform to the identity matrix
            auto transform = glm::identity<float4x4>();

            transform = glm::translate(transform, Position);


            // Apply rotation from quaternion
            transform *= glm::toMat4(Rotation);

            // Apply scaling
            transform = glm::scale(transform, Scale);

            return transform;
        }


        static glm::quat safeQuatLookAt(
            glm::vec3 const& lookFrom,
            glm::vec3 const& lookTo,
            glm::vec3 const& up,
            glm::vec3 const& alternativeUp) {
            glm::vec3 direction = lookTo - lookFrom;
            float directionLength = glm::length(direction);

            // Check if the direction is valid; Also deals with NaN
            if (!(directionLength > 0.0001))
                return glm::quat(1, 0, 0, 0); // Just return identity

            // Normalize direction
            direction /= directionLength;

            // Is the normal up (nearly) parallel to direction?
            if (glm::abs(glm::dot(direction, up)) > .9999f) {
                // Use alternative up
                return glm::quatLookAt(direction, alternativeUp);
            } else {
                return glm::quatLookAt(direction, up);
            }
        }

        void SetEulerAngles(const float3& eulerAngles) {
            Rotation = glm::quat(eulerAngles);
        }
    };
}
