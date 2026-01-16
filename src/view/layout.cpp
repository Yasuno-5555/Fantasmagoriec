// UI Layer: Flexbox Layout Engine (2-Pass)
// Pass 1: Measure (Bottom-Up) - Children tell parent their size
// Pass 2: Arrange (Top-Down) - Parent assigns positions to children
// Pure User Land - V5 Core untouched

#include "view/definitions.hpp"
#include "core/contexts_internal.hpp"
#include "text/font_manager.hpp"
#include "core/utf8.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <algorithm>
#include <cstring>
#include <cmath>

namespace fanta {
namespace ui {

    // --- Pass 1: Measure (Bottom-Up) ---
    // Each node determines its "intrinsic" or "desired" size
    // Returns {width, height} that this node would like to have
    
    void MeasureRecursive(ViewHeader* node) {
        if (!node) return;
        
        float content_w = 0, content_h = 0;
        
        // Measure children first (bottom-up)
        for (ViewHeader* child = node->first_child; child; child = child->next_sibling) {
            MeasureRecursive(child);
            
            if (child->is_absolute) continue;

            float child_w = child->measured_size.w + child->margin * 2;
            float child_h = child->measured_size.h + child->margin * 2;
            
            if (node->is_row) {
                // Row: sum widths, max height
                content_w += child_w;
                content_h = std::max(content_h, child_h);
            } else {
                // Column: max width, sum heights
                content_w = std::max(content_w, child_w);
                content_h += child_h;
            }
        }
        
        // Add own padding
        content_w += node->padding * 2;
        content_h += node->padding * 2;
        
        // Text node sizing (The Truth of Metrics)
        if (node->type == ViewType::Text) {
            auto* txt = static_cast<TextView*>(node);
            if (txt->text) {
                float text_w = 0;
                float scale = txt->font_size / (float)internal::FontManager::BASE_SDF_SIZE;
                
                // For each char, sum advance
                const char* p = txt->text;
                while (*p) {
                    uint32_t codepoint = internal::NextUTF8(p);
                    auto [font_id, glyph_idx] = internal::FontManager::Get().get_glyph_index(codepoint);
                    const auto& m = internal::FontManager::Get().get_metrics(font_id, glyph_idx);
                    text_w += m.advance * scale;
                }
                
                content_w = std::max(content_w, text_w);
                content_h = std::max(content_h, txt->font_size * 1.2f);
            }
        }
        
        // Button node sizing
        if (node->type == ViewType::Button) {
            auto* btn = static_cast<ButtonView*>(node);
            if (btn->label) {
                float text_w = 0;
                float scale = btn->font_size / (float)internal::FontManager::BASE_SDF_SIZE;
                const char* p = btn->label;
                while (*p) {
                    uint32_t codepoint = internal::NextUTF8(p);
                    auto [font_id, glyph_idx] = internal::FontManager::Get().get_glyph_index(codepoint);
                    const auto& m = internal::FontManager::Get().get_metrics(font_id, glyph_idx);
                    text_w += m.advance * scale;
                }
                content_w = std::max(content_w, text_w + 24); // padding
                content_h = std::max(content_h, btn->font_size * 1.2f + 16); // padding
            }
        }
        else if (node->type == ViewType::TextInput) {
            auto* input = static_cast<TextInputView*>(node);
            content_w = std::max(content_w, 200.0f); // Default width
            content_h = std::max(content_h, input->font_size * 1.2f + 12);
        }
        else if (node->type == ViewType::TextArea) {
            auto* area = static_cast<TextAreaView*>(node);
            float max_line_w = 0;
            int line_count = 1;
            float current_line_w = 0;
            
            float scale = area->font_size / (float)internal::FontManager::BASE_SDF_SIZE;
            const char* p = (area->buffer && area->buffer[0]) ? area->buffer : area->placeholder;
            
            if (p) {
                while (*p) {
                    if (*p == '\n') {
                        max_line_w = std::max(max_line_w, current_line_w);
                        current_line_w = 0;
                        line_count++;
                        p++; // Skip newline byte
                        continue;
                    }
                    uint32_t codepoint = internal::NextUTF8(p);
                    auto [font_id, glyph_idx] = internal::FontManager::Get().get_glyph_index(codepoint);
                    const auto& m = internal::FontManager::Get().get_metrics(font_id, glyph_idx);
                    current_line_w += m.advance * scale;
                }
                max_line_w = std::max(max_line_w, current_line_w);
            }
            
            content_w = std::max(content_w, max_line_w + 24.0f); // padding
            content_h = std::max(content_h, line_count * area->line_height + 24.0f);
            
            // Minimum Defaults
            content_w = std::max(content_w, 300.0f);
            content_h = std::max(content_h, 3 * area->line_height + 24.0f);
        }
        else if (node->type == ViewType::Toggle) {
            content_w = std::max(content_w, 100.0f); // Track + Label space
            content_h = std::max(content_h, 24.0f);
        }
        else if (node->type == ViewType::Slider) {
            content_w = std::max(content_w, 150.0f);
            content_h = std::max(content_h, 30.0f);
        }
        else if (node->type == ViewType::Plot) {
            content_w = std::max(content_w, 200.0f);
            content_h = std::max(content_h, 120.0f);
        }
        else if (node->type == ViewType::Table) {
            auto* table = static_cast<TableView*>(node);
            content_w = std::max(content_w, 200.0f);
            content_h = std::max(content_h, table->row_height * std::min(table->row_count, 10));
        }
        else if (node->type == ViewType::TreeNode) {
            auto* tree = static_cast<TreeNodeView*>(node);
            float indent = tree->depth * tree->indent_per_level;
            content_w = std::max(content_w, indent + 100.0f);
            content_h = std::max(content_h, 24.0f);
        }
        else if (node->type == ViewType::ColorPicker) {
            auto* picker = static_cast<ColorPickerView*>(node);
            content_w = std::max(content_w, picker->sv_size + 8 + picker->hue_bar_width);
            content_h = std::max(content_h, picker->sv_size + 8 + 30);  // SV + gap + preview
        }
        else if (node->type == ViewType::MenuBar) {
            auto* bar = static_cast<MenuBarView*>(node);
            content_h = std::max(content_h, bar->bar_height);
        }
        else if (node->type == ViewType::MenuItem) {
            content_w = std::max(content_w, 150.0f);
            content_h = std::max(content_h, 24.0f);
        }
        
        // Apply explicit size constraints
        node->content_size.w = content_w;
        node->content_size.h = content_h;
        
        node->measured_size.w = (node->width > 0) ? node->width : content_w;
        node->measured_size.h = (node->height > 0) ? node->height : content_h;
    }

    // --- Pass 2: Arrange (Top-Down) ---
    // Parent assigns final rect to each child based on flex_grow and layout direction
    
    void ArrangeRecursive(ViewHeader* node, float x, float y, float avail_w, float avail_h) {
        if (!node) return;
        
        // Set own computed rect
        node->computed_rect.x = x;
        node->computed_rect.y = y;
        node->computed_rect.w = avail_w;
        node->computed_rect.h = avail_h;
        
        // Store in persistent state for next-frame hit testing
        internal::GetEngineContext().persistent.store_rect(node->id, x, y, avail_w, avail_h);
        
        // No children? Done.
        if (!node->first_child) return;
        
        // Calculate inner area (after padding)
        float inner_x = x + node->padding;
        float inner_y = y + node->padding;
        float inner_w = avail_w - node->padding * 2;
        float inner_h = avail_h - node->padding * 2;
        
        // --- Wrap Mode ---
        if (node->wrap) {
            float main_avail = node->is_row ? inner_w : inner_h;
            float line_cursor = 0;
            float cross_cursor = 0;
            float line_max_cross = 0;
            
            for (ViewHeader* c = node->first_child; c; c = c->next_sibling) {
                float c_main = node->is_row ? c->measured_size.w : c->measured_size.h;
                float c_cross = node->is_row ? c->measured_size.h : c->measured_size.w;
                c_main += c->margin * 2;
                c_cross += c->margin * 2;
                
                // Wrap to next line?
                if (line_cursor + c_main > main_avail && line_cursor > 0) {
                    cross_cursor += line_max_cross;
                    line_cursor = 0;
                    line_max_cross = 0;
                }
                
                if (node->is_row) {
                    ArrangeRecursive(c, inner_x + line_cursor + c->margin, inner_y + cross_cursor + c->margin, 
                                     c->measured_size.w, c->measured_size.h);
                } else {
                    ArrangeRecursive(c, inner_x + cross_cursor + c->margin, inner_y + line_cursor + c->margin,
                                     c->measured_size.w, c->measured_size.h);
                }
                
                line_cursor += c_main;
                line_max_cross = std::max(line_max_cross, c_cross);
            }
            return;
        }

        // --- SPLITTER LOGIC ---
        if (node->type == ViewType::Splitter) {
            auto* split = static_cast<SplitterView*>(node);
            float ratio = split->split_ratio ? *split->split_ratio : 0.5f;
            float handle = split->handle_thickness; // 8.0f
            
            ViewHeader* c1 = node->first_child;
            ViewHeader* c2 = c1 ? c1->next_sibling : nullptr;
            
            if (c1 && c2) {
                // We only handle 2 children for now.
                float avail = split->is_vertical ? inner_h : inner_w;
                float size1 = (avail - handle) * ratio;
                float size2 = avail - handle - size1;
                
                if (split->is_vertical) {
                    // Top / Bottom
                    ArrangeRecursive(c1, inner_x + c1->margin, inner_y + c1->margin, inner_w - c1->margin*2, size1 - c1->margin*2);
                    ArrangeRecursive(c2, inner_x + c2->margin, inner_y + size1 + handle + c2->margin, inner_w - c2->margin*2, size2 - c2->margin*2);
                } else {
                    // Left / Right
                    ArrangeRecursive(c1, inner_x + c1->margin, inner_y + c1->margin, size1 - c1->margin*2, inner_h - c1->margin*2);
                    ArrangeRecursive(c2, inner_x + size1 + handle + c2->margin, inner_y + c2->margin, size2 - c2->margin*2, inner_h - c2->margin*2);
                }
            } else if (c1) {
                // Just one child, fill
                 ArrangeRecursive(c1, inner_x + c1->margin, inner_y + c1->margin, inner_w - c1->margin*2, inner_h - c1->margin*2);
            }
            return;
        }
        
        // Sum fixed sizes and flex_grow totals
        float total_fixed = 0;
        float total_flex_grow = 0;
        float total_flex_shrink = 0;
        int child_count = 0;
        
        for (ViewHeader* c = node->first_child; c; c = c->next_sibling) {
            if (c->is_absolute) continue;
            child_count++;
            float c_main = node->is_row ? c->measured_size.w : c->measured_size.h;
            c_main += c->margin * 2;
            
            total_fixed += c_main;
            total_flex_grow += c->flex_grow;
            total_flex_shrink += c->flex_shrink;
        }
        
        // Calculate remaining space
        float main_avail = node->is_row ? inner_w : inner_h;
        float remaining = main_avail - total_fixed;
        
        // Arrange children along main axis
        float cursor = 0;
        
        for (ViewHeader* c = node->first_child; c; c = c->next_sibling) {
            // Absolute Positioning Handling
            if (c->is_absolute) {
                // Arrange absolute child at its requested position
                // Use measured size if width/height not set explicitly via builder?
                // measured_size contains max of explicit and content.
                ArrangeRecursive(c, c->left, c->top, c->measured_size.w, c->measured_size.h);
                continue;
            }

            float c_main, c_cross;
            float c_measured = node->is_row ? c->measured_size.w : c->measured_size.h;
            
            if (remaining >= 0 && c->flex_grow > 0 && total_flex_grow > 0) {
                // Grow: distribute extra space
                c_main = c_measured + (c->flex_grow / total_flex_grow) * remaining;
            } else if (remaining < 0 && c->flex_shrink > 0 && total_flex_shrink > 0) {
                // Shrink: reduce size proportionally
                float shrink_amount = (-remaining) * (c->flex_shrink / total_flex_shrink);
                c_main = std::max(0.0f, c_measured - shrink_amount);
            } else {
                c_main = c_measured;
            }
            
            // Cross axis size (with alignment)
            float cross_avail = node->is_row ? (inner_h - c->margin * 2) : (inner_w - c->margin * 2);
            float c_measured_cross = node->is_row ? c->measured_size.h : c->measured_size.w;
            
            // Explicit size takes precedence
            float explicit_cross = node->is_row ? c->height : c->width;
            if (explicit_cross > 0) {
                c_cross = explicit_cross;
            } else if (c->align == Align::Stretch) {
                c_cross = cross_avail;
            } else {
                c_cross = c_measured_cross;
            }
            
            // Cross-axis offset based on alignment
            float cross_offset = 0;
            if (c->align == Align::Center) {
                cross_offset = (cross_avail - c_cross) * 0.5f;
            } else if (c->align == Align::End) {
                cross_offset = cross_avail - c_cross;
            }
            
            if (node->is_row) {
                float c_x = inner_x + cursor + c->margin;
                float c_y = inner_y + c->margin + cross_offset;
                ArrangeRecursive(c, c_x, c_y, c_main, c_cross);
                cursor += c_main + c->margin * 2;
            } else {
                float c_x = inner_x + c->margin + cross_offset;
                float c_y = inner_y + cursor + c->margin;
                ArrangeRecursive(c, c_x, c_y, c_cross, c_main);
                cursor += c_main + c->margin * 2;
            }
        }
    }

    // --- Public API ---
    
    void ComputeFlexLayout(ViewHeader* root, float screen_w, float screen_h) {
        if (!root) return;
        
        // Pass 1: Measure (bottom-up)
        MeasureRecursive(root);
        
        // Pass 2: Arrange (top-down)
        ArrangeRecursive(root, 0, 0, screen_w, screen_h);
    }

} // namespace ui
} // namespace fanta
