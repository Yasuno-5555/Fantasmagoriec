#pragma once
#include <cmath>
#include <algorithm>
#include <vector>
#include <string>
#include <functional>
#include <map>
#include <memory>
#include "core/types_core.hpp"

namespace fanta {

// --- Legacy Phase N Spring (Internal) ---
namespace internal {
    struct AnimationState {
        float current[4] = {0,0,0,0};
        float target[4] = {0,0,0,0};
        float velocity[4] = {0,0,0,0};
        bool active = false;
        
        float stiffness = 150.0f;
        float damping = 20.0f;
        float mass = 1.0f;
        
        void set_target(float v0, float v1, float v2, float v3) {
            float eps = 0.001f;
            if (std::abs(target[0] - v0) > eps || 
                std::abs(target[1] - v1) > eps || 
                std::abs(target[2] - v2) > eps || 
                std::abs(target[3] - v3) > eps) {
                target[0] = v0; target[1] = v1; target[2] = v2; target[3] = v3;
                active = true;
            }
        }
        
        void update(float dt) {
            if (!active) return;
            bool still_moving = false;
            float eps = 0.001f;
            for (int i = 0; i < 4; ++i) {
                float displacement = current[i] - target[i];
                float force = -stiffness * displacement;
                float damp = -damping * velocity[i];
                float accel = (force + damp) / mass;
                velocity[i] += accel * dt;
                current[i] += velocity[i] * dt;
                if (std::abs(displacement) > eps || std::abs(velocity[i]) > eps) still_moving = true;
            }
            active = still_moving;
            if (!active) {
                for(int i=0; i<4; ++i) { current[i] = target[i]; velocity[i] = 0; }
            }
        }
    };
} // namespace internal

// --- Modern Timeline Animation System ---
namespace animation {

    using EasingFn = float(*)(float);

    namespace easing {
        inline float linear(float t) { return t; }
        inline float ease_in_quad(float t) { return t * t; }
        inline float ease_out_quad(float t) { return 1.0f - (1.0f - t) * (1.0f - t); }
        inline float ease_in_out_quad(float t) { return t < 0.5f ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f; }
        inline float ease_out_bounce(float t) {
            const float n1 = 7.5625f, d1 = 2.75f;
            if (t < 1.0f / d1) return n1 * t * t;
            else if (t < 2.0f / d1) { t -= 1.5f / d1; return n1 * t * t + 0.75f; }
            else if (t < 2.5f / d1) { t -= 2.25f / d1; return n1 * t * t + 0.9375f; }
            else { t -= 2.625f / d1; return n1 * t * t + 0.984375f; }
        }
    }

    struct Keyframe {
        float time; // Normalized 0-1
        float value;
        EasingFn easing;
        Keyframe(float t, float v, EasingFn e = easing::linear) : time(t), value(v), easing(e) {}
    };

    class KeyframeTrack {
    public:
        std::string name;
        std::vector<Keyframe> keyframes;
        float duration_ms;

        KeyframeTrack(const std::string& n, float d) : name(n), duration_ms(d) {}

        KeyframeTrack& add(float time, float value, EasingFn easing = easing::linear) {
            keyframes.push_back(Keyframe(time, value, easing));
            std::sort(keyframes.begin(), keyframes.end(), [](const Keyframe& a, const Keyframe& b){ return a.time < b.time; });
            return *this;
        }

        float sample(float t) const {
            if (keyframes.empty()) return 0.0f;
            if (keyframes.size() == 1) return keyframes[0].value;
            t = std::clamp(t, 0.0f, 1.0f);
            
            size_t prev_idx = 0, next_idx = 0;
            for(size_t i=0; i<keyframes.size(); ++i) {
                if(keyframes[i].time <= t) prev_idx = i;
                if(keyframes[i].time >= t) { next_idx = i; break; }
                next_idx = i;
            }

            if(prev_idx == next_idx) return keyframes[prev_idx].value;

            const auto& prev = keyframes[prev_idx];
            const auto& next = keyframes[next_idx];
            
            float local_t = (t - prev.time) / (next.time - prev.time);
            float eased_t = next.easing(local_t);
            return prev.value + (next.value - prev.value) * eased_t;
        }
    };

    enum class LoopMode { Once, Loop, PingPong };
    enum class PlaybackState { Stopped, Playing, Paused };
    enum class Direction { Forward, Reverse };

    class Timeline {
    public:
        std::map<std::string, KeyframeTrack> tracks;
        float duration_ms;
        float current_time_ms = 0;
        PlaybackState state = PlaybackState::Stopped;
        Direction direction = Direction::Forward;
        LoopMode loop_mode = LoopMode::Once;
        float speed = 1.0f;

        Timeline(float d) : duration_ms(d) {}

        void add_track(const KeyframeTrack& track) { tracks.insert({track.name, track}); }
        void play() { state = PlaybackState::Playing; }
        void stop() { state = PlaybackState::Stopped; current_time_ms = 0; direction = Direction::Forward; }
        
        void update(float dt_ms) {
            if(state != PlaybackState::Playing) return;
            float delta = dt_ms * speed;
            
            if(direction == Direction::Forward) {
                current_time_ms += delta;
                if(current_time_ms >= duration_ms) {
                    if(loop_mode == LoopMode::Once) { current_time_ms = duration_ms; state = PlaybackState::Stopped; }
                    else if(loop_mode == LoopMode::Loop) current_time_ms = std::fmod(current_time_ms, duration_ms);
                    else if(loop_mode == LoopMode::PingPong) { current_time_ms = duration_ms; direction = Direction::Reverse; }
                }
            } else {
                current_time_ms -= delta;
                if(current_time_ms <= 0) {
                     if(loop_mode == LoopMode::Once) { current_time_ms = 0; state = PlaybackState::Stopped; }
                     else if(loop_mode == LoopMode::Loop) current_time_ms = duration_ms;
                     else if(loop_mode == LoopMode::PingPong) { current_time_ms = 0; direction = Direction::Forward; }
                }
            }
        }

        float get(const std::string& name) const {
            auto it = tracks.find(name);
            if(it == tracks.end()) return 0.0f;
            return it->second.sample(current_time_ms / duration_ms);
        }
    };

    // --- Animation Groups ---
    class Animation {
    public:
        virtual ~Animation() = default;
        virtual void start() = 0;
        virtual void update(float dt_ms) = 0;
        virtual bool is_complete() const = 0;
    };

    class SequentialGroup : public Animation {
        std::vector<std::shared_ptr<Animation>> animations;
        size_t current_index = 0;
        bool running = false;
        bool completed = false;
    public:
        void add(std::shared_ptr<Animation> anim) { animations.push_back(anim); }
        void start() override { running = true; completed = false; current_index = 0; if(!animations.empty()) animations[0]->start(); }
        void update(float dt_ms) override {
            if(!running || completed || animations.empty()) return;
            auto& current = animations[current_index];
            current->update(dt_ms);
            if(current->is_complete()) {
                current_index++;
                if(current_index >= animations.size()) { completed = true; running = false; }
                else animations[current_index]->start();
            }
        }
        bool is_complete() const override { return completed; }
    };
    
    class ParallelGroup : public Animation {
        std::vector<std::shared_ptr<Animation>> animations;
        bool running = false;
        bool completed = false;
    public:
        void add(std::shared_ptr<Animation> anim) { animations.push_back(anim); }
        void start() override { running = true; completed = false; for(auto& a : animations) a->start(); }
        void update(float dt_ms) override {
            if(!running || completed) return;
            bool all_done = true;
            for(auto& a : animations) {
                a->update(dt_ms);
                if(!a->is_complete()) all_done = false;
            }
            if(all_done) { completed = true; running = false; }
        }
        bool is_complete() const override { return completed; }
    };

} // namespace animation
} // namespace fanta
