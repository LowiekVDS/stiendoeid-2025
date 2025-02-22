#ifndef COLORS_HPP
#define COLORS_HPP

#include <stdint.h>
#include <vector>

#include "tools.hpp"

#define MAX_ALPHA_POINTS 10
#define MAX_COLOR_POINTS 10
#define MAX_CURVE_POINTS 10
#define MAX_COLORS 10

namespace effects {

struct RGBColor {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct ColorPoint {
    float focus;
    float position;
    RGBColor color;
};

struct ColorGradient {
    std::vector<ColorPoint> color_points;
};

struct GradientLevelPair {
    ColorGradient colorGradient;
    std::vector<CurvePoint> brightness;
};

/**
 * Returns the number of bytes read
 */
int ParseGradientLevelPairFromBytes(const uint8_t* bytes, int size, GradientLevelPair& gradient_level_pair);

/**
 * position: float from 0 to 1
 */
RGBColor GetColorAt(const ColorGradient& gradient, float position);

/**
 * position: float from 0 to 1
 */
RGBColor ApplyBrightnessCurve(const RGBColor& color, const std::vector<CurvePoint>& brightness_curve, float position);

RGBColor GetCRGBColorFromGradientLevelPair(const GradientLevelPair& gradient_level_pair, float position);

} // namespace effect

#endif // COLORS_HPP