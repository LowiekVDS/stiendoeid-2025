#pragma once

#include <vector>

namespace effects {

struct CurvePoint {
    float X;
    float Y;
};

/**
 * Start: X-position of start
 * End: X-position of end
 * Factor: dy / dx
 */
float interpolate(float start, float end, float factor);

float GetCurrentCurveY(const std::vector<CurvePoint>& curve, float position);

} // namespace effects