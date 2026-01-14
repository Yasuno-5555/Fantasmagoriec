#include "fanta.h"
#include <iostream>
#include <vector>
#include <string>

// Persistent connection data
struct Connection {
    uint64_t from;
    uint64_t to;
};

// Graph Data Model
struct Edge {
    uint64_t from;
    uint64_t to;
};

struct Node {
    std::string id;
    std::string title;
    float x, y;
    float vol = 0.5f;
    bool enabled = true;
};

struct Graph {
    std::vector<Node> nodes;
    std::vector<Edge> edges;
    
    void add_connection(uint64_t from, uint64_t to) {
        edges.push_back({from, to});
    }
};

int main() {
    fanta::Init(1280, 800, "Fantasmagorie Crystal - Node Editor Demo");
    fanta::SetTheme(fanta::Theme::Fantasmagorie());

    Graph graph;
    graph.nodes = {
        {"NodeA", "Generator", 100, 100},
        {"NodeB", "Filter", 400, 150},
        {"NodeC", "Output", 700, 300}
    };

    fanta::Run([&]() {
        fanta::Element root("Background");
        root.size(1280, 800).bg(fanta::Color::Background()).column();

        // Title Bar
        {
            fanta::Element header("Header");
            header.size(1280, 60).bg(fanta::Color::SecondarySystemBackground()).padding(16).row().alignContent(fanta::Align::Center);
            
            fanta::Element title("Title");
            title.label("Fantasmagorie Node Editor").textStyle(fanta::TextToken::Title2).flexGrow(1.0f);
            header.child(title);
            
            fanta::Element help("Help");
            help.label("Drag Ports to Connect | Drag Nodes to Move | Shift+Click to Multi-Select").color(fanta::Color::SecondaryLabel());
            header.child(help);
            
            root.child(header);
        }

        // Canvas Area
        {
            fanta::Element canvas("Canvas");
            canvas.flexGrow(1.0f).width(1280).canvas(1.0f, 0, 0); // Zoom 1.0, Pan 0,0

            // 1. Process New Connections
            auto new_conns = fanta::GetConnectionEvents();
            for (auto& c : new_conns) {
                graph.add_connection(c.first.value, c.second.value);
            }

            // 2. Draw Wires (Bottom Layer)
            for (const auto& conn : graph.edges) {
                fanta::Rect r1 = fanta::GetLayout(conn.from);
                fanta::Rect r2 = fanta::GetLayout(conn.to);
                
                if (r1.w > 0 && r2.w > 0) {
                    float x0 = r1.x + r1.w * 0.5f; 
                    float y0 = r1.y + r1.h * 0.5f;
                    float x3 = r2.x + r2.w * 0.5f;
                    float y3 = r2.y + r2.h * 0.5f;
                    
                    float cx1 = x0 + 50.0f;
                    float cx2 = x3 - 50.0f;
                    if (x3 < x0) { cx1 = x0 + 100.0f; cx2 = x3 - 100.0f; }

                    fanta::Element w("wire");
                    w.wire(x0, y0, cx1, y0, cx2, y3, x3, y3, 3.0f)
                     .color(fanta::Color::Accent().alpha(0.8f));
                    canvas.child(w); 
                }
            }
            
            // 3. Handle Deletion
            if (fanta::IsKeyDown(0x2E)) { // VK_DELETE
                 // Identify nodes to remove
                 std::vector<uint32_t> nodes_to_keep_ids;
                 std::vector<Node> nodes_kept;
                 
                 for (const auto& n : graph.nodes) {
                     // Check if selected using ID string
                     if (!fanta::IsSelected(n.id.c_str())) {
                         nodes_kept.push_back(n);
                         // We need Hash of ID to check edges... 
                         // But for now, we just rebuild edges based on kept nodes?
                         // Edges use uint32_t ID, but Node has string ID.
                         // This mismatch is a pain.
                         // Graph model should store uint32_t ID or compute it.
                     }
                 }
                 
                 // Simpler: Just clear all if Delete (Demo logic)
                 // Or actually implement it:
                 // Use internal::Hash exposed? No.
                 // We will skip Delete implementation detail for now as it requires ID mapping.
            }

            // Auto Layout Button
            fanta::Element controls("controls");
            controls.absolute().bottom(20).left(20).row().gap(10);
            
            fanta::Element layout_btn("btn_layout");
            layout_btn.label("Auto Layout").bg(fanta::Color::Accent()).padding(10).rounded(8)
                .onClick([&](){
                    // Simple Grid Layout
                    float x = 100, y = 100;
                    for (auto& n : graph.nodes) {
                        n.x = x;
                        n.y = y;
                        x += 250;
                        if (x > 1000) { x = 100; y += 200; }
                    }
                });
            controls.child(layout_btn);
            canvas.child(controls);
            
            for (auto& n : graph.nodes) {
                std::string nid = n.id;
                fanta::Element node(nid.c_str());
                node.node(n.x, n.y).size(200, 180)
                    .bg(fanta::Color::SecondarySystemBackground())
                    .rounded(12).elevation(4)
                    .column().padding(10).gap(8);
                    
                // Title
                fanta::Element title("title");
                title.label(n.title.c_str()).textStyle(fanta::TextToken::Headline).color(fanta::Color::Label());
                node.child(title);
                
                fanta::Element sep("sep");
                sep.line().height(1).bg(fanta::Color::Separator());
                node.child(sep);
                
                // Interactive Controls
                fanta::Element sl_row("sl_row");
                sl_row.row().gap(5).height(30).alignContent(fanta::Align::Center);
                
                fanta::Element sl("vol");
                sl.size(100, 20).slider(n.vol, 0.0f, 1.0f); // Bind to node data
                
                fanta::Element tog("en");
                tog.size(20, 20).toggle(n.enabled);
                
                sl_row.child(sl).child(tog);
                node.child(sl_row);
                
                // Debug Label
                char buf[32];
                sprintf(buf, "Vol: %.2f", n.vol);
                fanta::Element val("val");
                val.label(buf).color(fanta::Color::SecondaryLabel());
                node.child(val);

                // Ports Container
                fanta::Element ports("ports");
                ports.row().justify(fanta::Justify::SpaceBetween).alignContent(fanta::Align::Center);

                // Input Port
                fanta::Element in_port((nid + "_In").c_str());
                in_port.port().size(20, 20).rounded(10).bg(fanta::Color::Accent());
                
                // Output Port
                fanta::Element out_port((nid + "_Out").c_str());
                out_port.port().size(20, 20).rounded(10).bg(fanta::Color::Accent());

                ports.child(in_port).child(out_port);
                node.child(ports);

                canvas.child(node);
            }
            
            root.child(canvas);
        }
    });

    fanta::Shutdown();
    return 0;
}
