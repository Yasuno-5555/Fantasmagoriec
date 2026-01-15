#include "backend/backend.hpp"
#include "text/font_manager.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <algorithm>

namespace fanta {
namespace internal {

    // Manages the SDF Texture Atlas
    class GlyphCache {
    public:
        static GlyphCache& Get() {
            static GlyphCache instance;
            return instance;
        }
        
        // Initialize Atlas texture (empty)
        void init(Backend* backend) {
            if (!backend) return;
            m_texture = backend->create_texture(ATLAS_WIDTH, ATLAS_HEIGHT, fanta::internal::TextureFormat::R8);
        }
        
        // Retrieve UVs for a glyph. Generates/Updates atlas if missing.
        bool get_glyph_uv(FontID font, uint32_t glyph_index, 
                          float& u0, float& v0, float& u1, float& v1,
                          float& glyph_w, float& glyph_h, float& glyph_advance) {
            
            CacheKey key = { font, glyph_index };
            auto it = m_cache.find(key);
            
            if (it != m_cache.end()) {
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
            if (m_cursor_x + w + PADDING > ATLAS_WIDTH) {
                m_cursor_x = 0;
                m_cursor_y += m_row_height + PADDING;
                m_row_height = 0;
            }
            
            if (m_cursor_y + h + PADDING > ATLAS_HEIGHT) {
                std::cerr << "Glyph Cache Overflow!" << std::endl;
                return false;
            }
            
            int x = m_cursor_x;
            int y = m_cursor_y;
            
            for (int i = 0; i < h; ++i) {
                for (int j = 0; j < w; ++j) {
                    m_pixels[(y + i) * ATLAS_WIDTH + (x + j)] = sdf_buffer[i * w + j];
                }
            }
            
            m_cursor_x += w + PADDING;
            m_row_height = std::max(m_row_height, h);
            m_is_dirty = true;
            
            CachedGlyph g;
            g.px_w = (float)w;
            g.px_h = (float)h;
            
            const auto& metrics = FontManager::Get().get_metrics(font, glyph_index);
            g.px_adv = metrics.advance;

            g.u0 = (float)x / ATLAS_WIDTH;
            g.v0 = (float)y / ATLAS_HEIGHT;
            g.u1 = (float)(x + w) / ATLAS_WIDTH;
            g.v1 = (float)(y + h) / ATLAS_HEIGHT;
            
            m_cache[key] = g;
            
            u0 = g.u0; v0 = g.v0;
            u1 = g.u1; v1 = g.v1;
            glyph_w = g.px_w;
            glyph_h = g.px_h;
            glyph_advance = g.px_adv;
            
            return true;
        }
                           
        GpuTexturePtr get_atlas_texture() const { return m_texture; }
        
        void update_texture() {
            if (!m_is_dirty || !m_texture) return;
            m_texture->upload(m_pixels.data(), ATLAS_WIDTH, ATLAS_HEIGHT);
            m_is_dirty = false;
        }
        
    private:
        GlyphCache() {
            m_pixels.resize(ATLAS_WIDTH * ATLAS_HEIGHT, 0);
        }
        
        static const int ATLAS_WIDTH = 512;
        static const int ATLAS_HEIGHT = 512;
        static const int SDF_SIZE = 32;
        static const int PADDING = 2;
        
        GpuTexturePtr m_texture;
        std::vector<uint8_t> m_pixels;
        bool m_is_dirty = false;
        
        int m_cursor_x = 0;
        int m_cursor_y = 0;
        int m_row_height = 0;
        
        struct CacheKey {
            FontID font;
            uint32_t glyph_index;
            bool operator<(const CacheKey& o) const {
                if(font != o.font) return font < o.font;
                return glyph_index < o.glyph_index;
            }
        };
        
        struct CachedGlyph {
            float u0, v0, u1, v1;
            float px_w, px_h, px_adv;
        };
        
        std::map<CacheKey, CachedGlyph> m_cache;
    };

} // namespace internal
} // namespace fanta
