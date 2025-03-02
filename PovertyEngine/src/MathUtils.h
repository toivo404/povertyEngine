#pragma once
#include <cmath>

namespace MathUtils
{
    inline float MoveTowards(float current, float target, float maxDelta)
    {
        float difference = target - current;

        if (std::abs(difference) <= maxDelta)
        {
            // If within maxDelta, snap to the target
            return target;
        }

        // Move towards target by maxDelta
        return current + (difference > 0 ? maxDelta : -maxDelta);
    }
}
