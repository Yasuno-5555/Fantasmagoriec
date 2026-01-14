#include "text/glyph_cache.hpp"
#include "text/font_manager.hpp"
#include <iostream>
#include <cstring>

// Include GLAD for OpenGL calls
#include <glad/gl.h>

namespace fanta {
namespace internal {

    GlyphCache& GlyphCache::Get() {
        static GlyphCache instance;
        return instance;
    }

    GlyphCache::GlyphCache() {
        // Initialize atlas buffer (Black/Transparent)
        pixels.resize(ATLAS_WIDTH * ATLAS_HEIGHT, 0);
    }

    void GlyphCache::init() {
        // Create OpenGL Texture
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        
        // Single channel (Red) for SDF
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, ATLAS_WIDTH, ATLAS_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
        
        // Linear filtering is CRITICAL for SDF
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    bool GlyphCache::get_glyph_uv(FontID font, uint32_t codepoint, 
                                  float& u0, float& v0, float& u1, float& v1,
                                  float& atlas_w, float& atlas_h) {
        
        atlas_w = (float)ATLAS_WIDTH;
        atlas_h = (float)ATLAS_HEIGHT;
        
        CacheKey key = { font, codepoint };
        auto it = cache.find(key);
        
        if (it != cache.end()) {
            const CachedGlyph& g = it->second;
            u0 = g.u0; v0 = g.v0;
            u1 = g.u1; v1 = g.v1;
            return true;
        }
        
        // Not in cache, generate
        std::vector<uint8_t> sdf_buffer;
        int w, h;
        
        if (!FontManager::Get().generate_sdf(font, codepoint, SDF_SIZE, sdf_buffer, w, h)) {
            return false;
        }
        
        // Pack into Atlas
        // Simple Shelf Packing
        if (cursor_x + w + PADDING > ATLAS_WIDTH) {
            // New Row
            cursor_x = 0;
            cursor_y += row_height + PADDING;
            row_height = 0;
        }
        
        if (cursor_y + h + PADDING > ATLAS_HEIGHT) {
            std::cerr << "Glyph Cache Overflow!" << std::endl;
            return false; // Dynamic resize not implemented for Phase 4
        }
        
        // Place at cursor
        int x = cursor_x;
        int y = cursor_y;
        
        // Copy to atlas pixels
        for (int i = 0; i < h; ++i) {
            for (int j = 0; j < w; ++j) {
                pixels[(y + i) * ATLAS_WIDTH + (x + j)] = sdf_buffer[i * w + j];
            }
        }
        
        // Update Packer
        cursor_x += w + PADDING;
        row_height = std::max(row_height, h);
        
        is_dirty = true;
        
        // Register in Cache
        CachedGlyph g;
        g.px_w = (float)w;
        g.px_h = (float)h;
        g.u0 = (float)x / ATLAS_WIDTH;
        g.v0 = (float)y / ATLAS_HEIGHT;
        g.u1 = (float)(x + w) / ATLAS_WIDTH;
        g.v1 = (float)(y + h) / ATLAS_HEIGHT;
        
        cache[key] = g;
        
        u0 = g.u0; v0 = g.v0;
        u1 = g.u1; v1 = g.v1;
        
        return true;
    }

    void GlyphCache::update_texture() {
        if (!is_dirty || texture_id == 0) return;
        
        glBindTexture(GL_TEXTURE_2D, texture_id);
        
        // Upload entire buffer for simplicity (Phase 4)
        // Optimization: use glTexSubImage2D for the new region only
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ATLAS_WIDTH, ATLAS_HEIGHT, GL_RED, GL_UNSIGNED_BYTE, pixels.data());
        
        glBindTexture(GL_TEXTURE_2D, 0);
        is_dirty = false;
    }

} // namespace internal
} // namespace fanta
