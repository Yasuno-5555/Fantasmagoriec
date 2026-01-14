#pragma once

/**
 * Fantasmagorie Crystal Build Configuration
 * This file is managed by the build system.
 */

// --- Feature Flags ---

#ifndef FANTA_FEATURE_MARKDOWN
#define FANTA_FEATURE_MARKDOWN 1
#endif

#ifndef FANTA_FEATURE_TIMELINE
#define FANTA_FEATURE_TIMELINE 1
#endif

#ifndef FANTA_FEATURE_GRID
#define FANTA_FEATURE_GRID 1
#endif

#ifndef FANTA_FEATURE_NODE_EDITOR
#define FANTA_FEATURE_NODE_EDITOR 1
#endif

#ifndef FANTA_FEATURE_SCRIPTING
#define FANTA_FEATURE_SCRIPTING 1
#endif

#ifndef FANTA_FEATURE_TOOLS
#define FANTA_FEATURE_TOOLS 1
#endif

// --- Backend Selection ---

#ifndef FANTA_BACKEND_OPENGL
#define FANTA_BACKEND_OPENGL 1
#endif

#ifdef _WIN32
#ifndef FANTA_BACKEND_D3D11
#define FANTA_BACKEND_D3D11 1
#endif
#endif

#ifndef FANTA_BACKEND_VULKAN
#define FANTA_BACKEND_VULKAN 1
#endif
