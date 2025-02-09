#include "Arduino.h"

#include "tools.hpp"
#include "colors.hpp"

namespace effects {

namespace {

struct HSVColor {
    float h;  // Hue [0, 360]
    float s;  // Saturation [0, 1]
    float v;  // Value [0, 1]  
};

HSVColor blendHSV(HSVColor color1, HSVColor color2, float t, float weight = 0.5) {
    // Adjust transition based on weight
    if (weight != 0.5) {
        if (t < weight) {
            t = 0.5 * (t / weight);
        } else {
            t = 0.5 + 0.5 * ((t - weight) / (1 - weight));
        }
    }

    // Blend Hue circularly
    float h1 = color1.h;
    float h2 = color2.h;
    float delta_h = fmod(h2 - h1 + 360, 360);  // Normalize hue difference

    if (delta_h > 180) {
        delta_h -= 360;  // Take shortest path
    }

    float blended_h = fmod(h1 + t * delta_h, 360);
    float blended_s = (1 - t) * color1.s + t * color2.s;
    float blended_v = (1 - t) * color1.v + t * color2.v;

    return {blended_h, blended_s, blended_v};
}

HSVColor rgbToHsv(RGBColor rgb) {
    float r = rgb.r / 255.0;
    float g = rgb.g / 255.0;
    float b = rgb.b / 255.0;

    float maxC = max(r, max(g, b));
    float minC = min(r, min(g, b));
    float delta = maxC - minC;

    float h = 0, s = 0, v = maxC;

    if (delta > 0.00001) {
        s = delta / maxC;

        if (maxC == r) {
            h = 60 * fmod(((g - b) / delta), 6);
        } else if (maxC == g) {
            h = 60 * (((b - r) / delta) + 2);
        } else if (maxC == b) {
            h = 60 * (((r - g) / delta) + 4);
        }

        if (h < 0) {
            h += 360;  // Ensure positive hue
        }
    }

    return {h, s, v};
}

// Function to convert HSV to RGB (for ESP32)
RGBColor hsvToRgb(HSVColor hsv) {
    float h = hsv.h, s = hsv.s, v = hsv.v;
    float c = v * s;
    float x = c * (1 - fabs(fmod(h / 60.0, 2) - 1));
    float m = v - c;

    float r, g, b;
    if (h < 60) {
        r = c, g = x, b = 0;
    } else if (h < 120) {
        r = x, g = c, b = 0;
    } else if (h < 180) {
        r = 0, g = c, b = x;
    } else if (h < 240) {
        r = 0, g = x, b = c;
    } else if (h < 300) {
        r = x, g = 0, b = c;
    } else {
        r = c, g = 0, b = x;
    }

    return {
        static_cast<uint8_t>((r + m) * 255),
        static_cast<uint8_t>((g + m) * 255),
        static_cast<uint8_t>((b + m) * 255)
    };
}

};

/**
 * position: float from 0 to 1
 */
RGBColor GetColorAt(const ColorGradient& gradient, float position) {
    if (gradient.color_points.size() == 0) {
        return {0, 0, 0}; // Default black if no points
    }

    // If only one color point, return it
    if (gradient.color_points.size() == 1) {
        return gradient.color_points.at(0).color;
    }

    ColorPoint prev = gradient.color_points[0];
    ColorPoint next = gradient.color_points[1];
    for (int i = 1; i < gradient.color_points.size(); ++i) {
        next = gradient.color_points[i];
        if (position < next.position) {
            float local_t = (position - prev.position) / (next.position - prev.position);
            HSVColor prev_hsv = rgbToHsv(prev.color);
            HSVColor next_hsv = rgbToHsv(next.color);
            HSVColor blended_hsv = blendHSV(prev_hsv, next_hsv, local_t, next.focus);
            return hsvToRgb(blended_hsv);
        }
        prev = next;
    }

    return next.color;
}

/**
 * position: float from 0 to 1
 */
RGBColor ApplyBrightnessCurve(const RGBColor& color, const std::vector<CurvePoint>& brightness_curve, float position) {
    if (brightness_curve.size() == 0) {
        return color;
    }

    if (brightness_curve.size() == 1) {
        float factor = brightness_curve[0].Y;
        return {
            static_cast<uint8_t>(color.r * factor),
            static_cast<uint8_t>(color.g * factor),
            static_cast<uint8_t>(color.b * factor)
        };
    }

    // Find prev and next curve points
    CurvePoint prev = brightness_curve[0];
    CurvePoint next = brightness_curve[1];
    for (int i = 0; i < brightness_curve.size(); ++i) {
        next = brightness_curve[i];
        if (position <= next.X) {
            float range = next.X - prev.X;
            float factor = (range > 0.0f) ? (position - prev.X) / range : 0.0f;
            factor = interpolate(prev.Y, next.Y, factor);
            return {
                static_cast<uint8_t>(color.r * factor),
                static_cast<uint8_t>(color.g * factor),
                static_cast<uint8_t>(color.b * factor)
            };
        }
        prev = next;
    }

    const float factor = brightness_curve.back().Y;
    return {
        static_cast<uint8_t>(color.r * factor),
        static_cast<uint8_t>(color.g * factor),
        static_cast<uint8_t>(color.b * factor)
    };
}

RGBColor GetCRGBColorFromGradientLevelPair(const GradientLevelPair& gradient_level_pair, float position) {
    RGBColor color = GetColorAt(gradient_level_pair.colorGradient, position);
    RGBColor adjusted_color = ApplyBrightnessCurve(color, gradient_level_pair.brightness, position);
    return adjusted_color;
};

int ParseGradientLevelPairFromBytes(const uint8_t* bytes, int size, GradientLevelPair& gradient_level_pair) {
    int offset = ParseVectorOfStructsFromBytes<ColorPoint>(bytes, size, gradient_level_pair.colorGradient.color_points);
    if (offset == 0) {
        return 0;
    }
    offset += ParseVectorOfStructsFromBytes<CurvePoint>(bytes + offset, size - offset, gradient_level_pair.brightness);
    if (offset == 0) {
        gradient_level_pair.colorGradient.color_points.clear();
    }
    return offset;
}

} // namespace effect
