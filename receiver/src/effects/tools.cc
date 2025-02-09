#include "tools.hpp"

namespace effects {

float interpolate(float start, float end, float factor) {
    return start + (end - start) * factor;
}

float GetCurrentCurveY(const std::vector<CurvePoint>& curve, float position) {
    if (curve.size() == 0) {
        return 0.0f;
    }

    // Find prev and next curve points
    CurvePoint prev = curve[0];
    CurvePoint next = prev;
    for (int i = 0; i < curve.size(); ++i) {
        next = curve[i];
        if (position <= next.X) {
            float range = next.X - prev.X;
            float factor = (range > 0.0f) ? (position - prev.X) / range : 0.0f;
            return interpolate(prev.Y, next.Y, factor);
        }
        prev = next;
    }

    return prev.Y;
}

} // namespace effect