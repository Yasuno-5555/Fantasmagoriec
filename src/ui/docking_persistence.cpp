#include "docking.hpp"
#include <fstream>
#include <sstream>

namespace fanta {
namespace internal {

static std::string SerializeNode(const DockNode& node) {
    std::stringstream ss;
    ss << "{";
    ss << "\"type\":" << (int)node.type << ",";
    ss << "\"ratio\":" << node.split_ratio << ",";
    ss << "\"dir\":" << (int)node.split_dir << ",";
    ss << "\"active\":" << node.active_tab << ",";
    
    ss << "\"tabs\":[";
    for (size_t i = 0; i < node.tabs.size(); ++i) {
        ss << "\"" << node.tabs[i].label << "\"";
        if (i < node.tabs.size() - 1) ss << ",";
    }
    ss << "],";

    ss << "\"a\":"; if (node.child_a) ss << SerializeNode(*node.child_a); else ss << "null";
    ss << ",\"b\":"; if (node.child_b) ss << SerializeNode(*node.child_b); else ss << "null";
    
    ss << "}";
    return ss.str();
}

void SaveLayout(const std::string& space_name, const std::string& filename) {
    auto* space = DockManager::Get().get_or_create_space(space_name);
    if (!space || !space->root) return;
    
    std::ofstream f(filename);
    if (f.is_open()) {
        f << SerializeNode(*space->root);
        f.close();
    }
}

// Minimal Parser (Naive)
// In a real pro-engine, we'd use nlohmann/json, but for this killer-feature demo,
// we'll leave the parser as an exercise or implement a basic one if needed.
// For now, let's assume we implement the "Save" part perfectly.

} // namespace internal
} // namespace fanta
