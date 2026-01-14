#include "fanta.h"
#include <vector>
#include <string>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace fanta;

// Aesthetic Constants
const Color BG_COLOR = Color::Hex(0x101010FF);
const Color GRID_COLOR = Color::Hex(0x1A1A1AFF);
const Color NODE_BG = Color::Hex(0x2A2A2AFF);
const Color WIRE_COLOR = Color::Hex(0x4040FFFF);
const Color TEXT_COLOR = Color::Hex(0xFFFFFFFF);

struct NodeData {
    std::string id;
    std::string label;
    float x, y;
};

struct EdgeData {
    int from, to;
};

static std::vector<NodeData> g_nodes = {
    {"N1", "Source Node", 150, 200},
    {"N2", "Processing Hub", 500, 350},
    {"N3", "Sink Node", 850, 200}
};

static std::vector<EdgeData> g_edges = {
    {0, 1}, {1, 2}
};

static float g_pan_x = 0;
static float g_pan_y = 0;
static float g_zoom = 1.0f;

static int g_dragging_node = -1;
static float g_drag_offset_x = 0;
static float g_drag_offset_y = 0;

// Screenshot mode flag
static bool g_screenshot_mode = false;
static std::string g_screenshot_filename = "graph_demo.png";
static int g_frame_count = 0;

int main(int argc, char* argv[]) {
    // Check for arguments
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--screenshot") {
            g_screenshot_mode = true;
        } else if (std::string(argv[i]) == "--output" && i + 1 < argc) {
            g_screenshot_filename = argv[++i];
        }
    }

    Init(1280, 720, "Fantasmagorie Node Graph Demo");

    Run([]{
        // Screenshot mode: capture after a few frames for rendering to settle
        if (g_screenshot_mode) {
            if (g_frame_count == 3) {
                CaptureScreenshot(g_screenshot_filename.c_str());
            }
            if (g_frame_count == 4) {
                 exit(0);
            }
        }
        g_frame_count++;

        float mx, my;
        GetMousePos(mx, my);
        bool down = IsMouseDown();
        bool pressed = IsMouseJustPressed();

        // Control Logic
        if (IsKeyDown('W')) g_pan_y += 5.0f;
        if (IsKeyDown('S')) g_pan_y -= 5.0f;
        if (IsKeyDown('A')) g_pan_x += 5.0f;
        if (IsKeyDown('D')) g_pan_x -= 5.0f;
        
        if (IsKeyDown('Q')) g_zoom *= 1.01f;
        if (IsKeyDown('E')) g_zoom *= 0.99f;
        g_zoom = std::clamp(g_zoom, 0.1f, 5.0f);

        // Transform mouse to world space
        float wx = (mx - g_pan_x) / g_zoom;
        float wy = (my - g_pan_y) / g_zoom;

        // Node Dragging
        if (pressed) {
            for(int i = (int)g_nodes.size() - 1; i >= 0; --i) {
                // Check hit (nodes are 240x120 in this demo)
                if (wx >= g_nodes[i].x && wx < g_nodes[i].x + 240 &&
                    wy >= g_nodes[i].y && wy < g_nodes[i].y + 120) {
                    g_dragging_node = i;
                    g_drag_offset_x = wx - g_nodes[i].x;
                    g_drag_offset_y = wy - g_nodes[i].y;
                    break;
                }
            }
        }
        
        if (!down) {
            g_dragging_node = -1;
        }

        if (g_dragging_node != -1) {
            g_nodes[g_dragging_node].x = wx - g_drag_offset_x;
            g_nodes[g_dragging_node].y = wy - g_drag_offset_y;
        }

        // --- UI Construction ---
        Element("Root").label("").bg(BG_COLOR)
            // Sidebar or HUD (Regular UI)
            .child(Element("HUD").label("").width(300).bg(Color::Hex(0x00000088))
                .child(Element("T1").label("Node Graph Editor").color(TEXT_COLOR))
                .child(Element("T2").label("WASD: Pan | QE: Zoom").color(Color::Hex(0xAAAAAAFF)).fontSize(14))
                .child(Element("T3").label(("Zoom: " + std::to_string(g_zoom)).c_str()).color(Color::Hex(0xAAAAAAFF)).fontSize(14))
            )

            // Canvas (The "World")
            .child(Element("GraphCanvas").label("").canvas(g_zoom, g_pan_x, g_pan_y)
                .bg(GRID_COLOR)
                
                // Wires layer
                .child([&](){
                    Element e("Wires"); e.label("");
                    for(const auto& edge : g_edges) {
                        auto& n1 = g_nodes[edge.from];
                        auto& n2 = g_nodes[edge.to];
                        
                        // Output pin at right center, Input pin at left center
                        float x1 = n1.x + 240, y1 = n1.y + 60;
                        float x2 = n2.x, y2 = n2.y + 60;
                        
                        // Bezier tangents
                        float dx = std::abs(x2 - x1) * 0.5f;
                        e.child(Element((n1.id + n2.id).c_str())
                            .wire(x1, y1, x1 + dx, y1, x2 - dx, y2, x2, y2, 4.0f)
                            .color(WIRE_COLOR));
                    }
                    return e;
                }())

                // Nodes layer
                .child([&](){
                    Element e("Nodes"); e.label("");
                    for(const auto& n : g_nodes) {
                        e.child(Element(n.id.c_str()).label(n.label.c_str())
                            .node(n.x, n.y)
                            .size(240, 120)
                            .bg(NODE_BG)
                            .rounded(12)
                            .elevation(8.0f)
                            .color(TEXT_COLOR));
                    }
                    return e;
                }())
            );
    });

    Shutdown();
    return 0;
}
