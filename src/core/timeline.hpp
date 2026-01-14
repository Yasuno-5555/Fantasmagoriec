#pragma once

#include "../core/types_internal.hpp"
#include <vector>
#include <map>
#include <string>
#include <functional>
#include <cmath>

namespace fanta {
namespace internal {

// Easing functions
enum class EasingType {
    Linear,
    EaseIn,
    EaseOut,
    EaseInOut,
    Spring,
    Bounce
};

inline float ApplyEasing(float t, EasingType type) {
    switch (type) {
        case EasingType::Linear: return t;
        case EasingType::EaseIn: return t * t;
        case EasingType::EaseOut: return 1.0f - (1.0f - t) * (1.0f - t);
        case EasingType::EaseInOut: return t < 0.5f ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
        case EasingType::Spring: {
            float c4 = (2.0f * 3.14159f) / 3.0f;
            return t == 0 ? 0 : t == 1 ? 1 : std::pow(2.0f, -10.0f * t) * std::sin((t * 10.0f - 0.75f) * c4) + 1.0f;
        }
        case EasingType::Bounce: {
            if (t < 1.0f / 2.75f) return 7.5625f * t * t;
            else if (t < 2.0f / 2.75f) { t -= 1.5f / 2.75f; return 7.5625f * t * t + 0.75f; }
            else if (t < 2.5f / 2.75f) { t -= 2.25f / 2.75f; return 7.5625f * t * t + 0.9375f; }
            else { t -= 2.625f / 2.75f; return 7.5625f * t * t + 0.984375f; }
        }
    }
    return t;
}

// Keyframe
template<typename T>
struct Keyframe {
    float time;      // Time in seconds
    T value;
    EasingType easing = EasingType::EaseInOut;
};

// Animation Track (one property)
template<typename T>
struct Track {
    std::string property_name;
    std::vector<Keyframe<T>> keyframes;
    
    void add_keyframe(float time, T value, EasingType easing = EasingType::EaseInOut) {
        keyframes.push_back({time, value, easing});
        // Sort by time
        std::sort(keyframes.begin(), keyframes.end(), 
            [](const auto& a, const auto& b) { return a.time < b.time; });
    }
    
    T evaluate(float time) const {
        if (keyframes.empty()) return T{};
        if (keyframes.size() == 1) return keyframes[0].value;
        
        // Clamp to keyframe range
        if (time <= keyframes.front().time) return keyframes.front().value;
        if (time >= keyframes.back().time) return keyframes.back().value;
        
        // Find surrounding keyframes
        for (size_t i = 1; i < keyframes.size(); i++) {
            if (time < keyframes[i].time) {
                const auto& k0 = keyframes[i - 1];
                const auto& k1 = keyframes[i];
                float t = (time - k0.time) / (k1.time - k0.time);
                t = ApplyEasing(t, k0.easing);
                return lerp(k0.value, k1.value, t);
            }
        }
        
        return keyframes.back().value;
    }
    
    static T lerp(const T& a, const T& b, float t) {
        return a + (b - a) * t;
    }
};

// Timeline (collection of tracks)
struct Timeline {
    std::map<ID, Track<float>> float_tracks;
    std::map<ID, Track<Vec2>> vec2_tracks;
    std::map<ID, Track<ColorF>> color_tracks;
    
    float duration = 0;
    float current_time = 0;
    float playback_speed = 1.0f;
    bool playing = false;
    bool loop = false;
    
    void play() { playing = true; }
    void pause() { playing = false; }
    void stop() { playing = false; current_time = 0; }
    void seek(float time) { current_time = std::clamp(time, 0.0f, duration); }
    
    void update(float dt) {
        if (!playing) return;
        
        current_time += dt * playback_speed;
        
        if (current_time >= duration) {
            if (loop) {
                current_time = std::fmod(current_time, duration);
            } else {
                current_time = duration;
                playing = false;
            }
        }
    }
    
    float get_float(ID id) const {
        auto it = float_tracks.find(id);
        if (it != float_tracks.end()) {
            return it->second.evaluate(current_time);
        }
        return 0;
    }
    
    Vec2 get_vec2(ID id) const {
        auto it = vec2_tracks.find(id);
        if (it != vec2_tracks.end()) {
            return it->second.evaluate(current_time);
        }
        return {0, 0};
    }
    
    ColorF get_color(ID id) const {
        auto it = color_tracks.find(id);
        if (it != color_tracks.end()) {
            return it->second.evaluate(current_time);
        }
        return {0, 0, 0, 1};
    }
};

// Specializations for lerp
template<>
inline Vec2 Track<Vec2>::lerp(const Vec2& a, const Vec2& b, float t) {
    return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t};
}

template<>
inline ColorF Track<ColorF>::lerp(const ColorF& a, const ColorF& b, float t) {
    return {
        a.r + (b.r - a.r) * t,
        a.g + (b.g - a.g) * t,
        a.b + (b.b - a.b) * t,
        a.a + (b.a - a.a) * t
    };
}

// Global timeline accessor
Timeline& GetTimeline();

} // namespace internal
} // namespace fanta
