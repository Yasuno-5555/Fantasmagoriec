// Fantasmagorie v2 - Main Header
// Include this file to use all Fantasmagorie features
#pragma once

// Core
#include "core/types.hpp"
#include "core/node.hpp"
#include "core/layout.hpp"
#include "core/context.hpp"
#include "core/undo.hpp"

// Render
#include "render/draw_list.hpp"
#include "render/backend.hpp"

// Widgets
#include "widgets/widgets.hpp"

// Style
#include "style/theme.hpp"
#include "style/system_theme.hpp"

// Platform
#include "platform/native.hpp"
#include "platform/mobile.hpp"

// Input
#include "input/gesture.hpp"

// Advanced Features
#include "script/engine.hpp"
#include "a11y/accessibility.hpp"
#include "profiler/profiler.hpp"
#include "plugin/plugin.hpp"

namespace fanta {

// Version
constexpr int VERSION_MAJOR = 2;
constexpr int VERSION_MINOR = 0;
constexpr int VERSION_PATCH = 0;
constexpr const char* VERSION_STRING = "2.0.0";

} // namespace fanta
