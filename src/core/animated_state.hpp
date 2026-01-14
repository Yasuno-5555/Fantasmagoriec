#pragma once
#include "animation.hpp"

namespace fanta {
namespace internal {

    template<typename T>
    struct PropertyAnimation {
        T start_value{};
        T current_value{};
        T target_value{};
        float elapsed = 0.0f;
        float duration = 0.0f;
        CurveType curve = CurveType::EaseOut;
        bool active = false;

        void update(float dt) {
            if (!active) return;
            elapsed += dt;
            float t = std::clamp(elapsed / duration, 0.0f, 1.0f);
            float weight = Curves::apply(curve, t);
            current_value = Lerp(start_value, target_value, weight);
            if (t >= 1.0f) active = false;
        }

        void animate_to(const T& target, float d, CurveType c) {
            if (target == target_value && active) return;
            start_value = current_value;
            target_value = target;
            duration = d;
            curve = c;
            elapsed = 0.0f;
            active = true;
        }

        void set_immediate(const T& v) {
            current_value = target_value = v;
            active = false;
        }
    };

} // internal
} // fanta
