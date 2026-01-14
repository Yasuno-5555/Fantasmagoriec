#include "backend/gpu_resources.hpp"

namespace fanta {
    class Backend; // Forward decl

namespace internal {

    // Manages the SDF Texture Atlas
    class GlyphCache {
    public:
        static GlyphCache& Get();
        
        // Initialize Atlas texture (empty)
        void init(Backend* backend);
        
        // Retrieve UVs for a glyph. Generates/Updates atlas if missing.
        // Returns true if found/generated.
        // out_uvs: u0, v0, u1, v1
        bool get_glyph_uv(FontID font, uint32_t glyph_index, 
                          float& u0, float& v0, float& u1, float& v1,
                          float& glyph_w, float& glyph_h, float& glyph_advance);
                           
        // API-agnostic texture handle
        GpuTexturePtr get_atlas_texture() const { return texture; }
        
        // Upload dirty regions to GPU
        void update_texture();
        
    private:
        GlyphCache();
        
        static const int ATLAS_WIDTH = 512;
        static const int ATLAS_HEIGHT = 512;
        static const int SDF_SIZE = 32; // Base size for SDF generation
        static const int PADDING = 2;   // Padding between glyphs
        
        GpuTexturePtr texture;
        std::vector<uint8_t> pixels; // 1 byte per pixel (Alpha)
        bool is_dirty = false;
        
        // Simple shelf packer state
        int cursor_x = 0;
        int cursor_y = 0;
        int row_height = 0;
        
        struct CacheKey {
            FontID font;
            uint32_t codepoint;
            bool operator<(const CacheKey& o) const {
                if(font != o.font) return font < o.font;
                return codepoint < o.codepoint;
            }
        };
        
        struct CachedGlyph {
            float u0, v0, u1, v1;
            float px_w, px_h, px_adv;
        };
        
        std::map<CacheKey, CachedGlyph> cache;
    };

} // namespace internal
} // namespace fanta
