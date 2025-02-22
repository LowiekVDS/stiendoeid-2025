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

int ParseCurvePointVectorOfStructsFromBytes(const uint8_t* bytes, int size, std::vector<CurvePoint>& structs) {
    if (size <= 1) {
        return 0;
    }

    constexpr int struct_size = 2 * sizeof(float);

    int num_structs = bytes[0];
    int structs_length = num_structs * struct_size;
    if (size < structs_length + 1) {
        return 0;
    }

    structs.clear();
    for (int i = 0; i < num_structs; ++i) {
        int offset = 1 + i * struct_size;
        CurvePoint s;
        memcpy(&s.X, bytes + offset, sizeof(float));
        memcpy(&s.Y, bytes + offset + sizeof(float), sizeof(float));
        structs.push_back(s);
    }

    return structs_length + 1;
}

} // namespace effect