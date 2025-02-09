#pragma once

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

// Returns a new pointer to the end of the parsed bytes
template<typename T>
int ParseVectorOfStructsFromBytes(const uint8_t* bytes, int size, std::vector<T>& structs);

} // namespace effects