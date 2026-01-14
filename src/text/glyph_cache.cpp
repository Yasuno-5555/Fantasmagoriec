#include "text/glyph_cache.hpp"
#include "text/font_manager.hpp"
#include "backend/backend.hpp"
#include <iostream>
#include <cstring>

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

    void GlyphCache::init(Backend* backend) {
        if (!backend) return;
        texture = backend->create_texture(ATLAS_WIDTH, ATLAS_HEIGHT, TextureFormat::R8);
    }

    bool GlyphCache::get_glyph_uv(FontID font, uint32_t glyph_index, 
                                  float& u0, float& v0, float& u1, float& v1,
                                  float& glyph_w, float& glyph_h, float& glyph_advance) {
        
        CacheKey key = { font, glyph_index };
        auto it = cache.find(key);
        
        if (it != cache.end()) {
            const CachedGlyph& g = it->second;
            u0 = g.u0; v0 = g.v0;
            u1 = g.u1; v1 = g.v1;
            glyph_w = g.px_w;
            glyph_h = g.px_h;
            glyph_advance = g.px_adv;
            return true;
        }
        
        // Not in cache, generate
        std::vector<uint8_t> sdf_buffer;
        int w, h;
        
        if (!FontManager::Get().generate_sdf(font, glyph_index, SDF_SIZE, sdf_buffer, w, h)) {
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
        
        // Phase 12: Get true advance from FreeType
        const auto& metrics = FontManager::Get().get_metrics(font, glyph_index);
        g.px_adv = metrics.advance;

        g.u0 = (float)x / ATLAS_WIDTH;
        g.v0 = (float)y / ATLAS_HEIGHT;
        g.u1 = (float)(x + w) / ATLAS_WIDTH;
        g.v1 = (float)(y + h) / ATLAS_HEIGHT;
        
        cache[key] = g;
        
        u0 = g.u0; v0 = g.v0;
        u1 = g.u1; v1 = g.v1;
        glyph_w = g.px_w;
        glyph_h = g.px_h;
        glyph_advance = g.px_adv;
        
        return true;
    }

    void GlyphCache::update_texture() {
        if (!is_dirty || !texture) return;
        
        texture->upload(pixels.data(), ATLAS_WIDTH, ATLAS_HEIGHT);
        is_dirty = false;
    }

} // namespace internal
} // namespace fanta
