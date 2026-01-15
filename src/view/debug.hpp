#pragma once
#include "core/contexts_internal.hpp"
#include <cstdarg>
#include <cstdio>
#include <iostream>

namespace fanta {
namespace ui {

    struct DebugOverlay {
        static void text(const char* fmt, ...) {
            auto& dbg = internal::GetEngineContext().debug;
            char buf[1024];
            va_list args;
            va_start(args, fmt);
            vsnprintf(buf, sizeof(buf), fmt, args);
            va_end(args);
            
            dbg.commands.push_back({internal::DebugOverlayCmd::Text, buf});
        }

        static void bar(const char* label, float value, float min = 0.0f, float max = 1.0f) {
            auto& dbg = internal::GetEngineContext().debug;
            dbg.commands.push_back({internal::DebugOverlayCmd::Bar, label, value, min, max});
        }

        static void rect(float x, float y, float w, float h, ColorF c = {1, 0, 1, 1}) {
            auto& dbg = internal::GetEngineContext().debug;
            dbg.commands.push_back({internal::DebugOverlayCmd::Rect, "", 0, 0, 0, x, y, w, h, c});
        }
    };

    inline void debug_dump(internal::ID id) {
        auto& ctx = internal::GetEngineContext();
        auto it_anim = ctx.persistent.animations.find(id.value);
        auto rect = ctx.persistent.get_rect(id);
        
        std::cout << "[DebugDump] ID: " << id.value << "\n";
        std::cout << "  Rect: x=" << rect.x << " y=" << rect.y << " w=" << rect.w << " h=" << rect.h << "\n";
        
        if (it_anim != ctx.persistent.animations.end()) {
            const auto& anim = it_anim->second;
            std::cout << "  Anim: current=[" << anim.current[0] << "," << anim.current[1] << "," << anim.current[2] << "," << anim.current[3] << "]\n";
            std::cout << "        target=[" << anim.target[0] << "," << anim.target[1] << "," << anim.target[2] << "," << anim.target[3] << "]\n";
        }
        
        if (ctx.interaction.hot_id.value == id.value) std::cout << "  State: HOT\n";
        if (ctx.interaction.active_id.value == id.value) std::cout << "  State: ACTIVE\n";
        if (ctx.persistent.focused_id.value == id.value) std::cout << "  State: FOCUSED\n";
        
        std::cout << std::endl;
    }

} // namespace ui
} // namespace fanta
