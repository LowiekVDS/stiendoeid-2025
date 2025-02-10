#pragma once

#include "Arduino.h"

#include <vector>
#include <stdint.h>
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

// Returns number of read bytes
int ParseCurvePointVectorOfStructsFromBytes(const uint8_t* bytes, int size, std::vector<CurvePoint>& structs);

} // namespace effects