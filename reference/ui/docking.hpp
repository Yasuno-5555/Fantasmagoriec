#pragma once
#include <fanta_id.h>
#include <vector>
#include <string>
#include <memory>
#include <functional>

namespace fanta {
namespace internal {

enum class DockType { Split, Tabs, Content };
enum class SplitDir { Horizontal, Vertical };

struct DockNode {
    DockType type = DockType::Content;
    ID id;
    
    // Split specific
    SplitDir split_dir = SplitDir::Horizontal;
    float split_ratio = 0.5f;
    std::unique_ptr<DockNode> child_a;
    std::unique_ptr<DockNode> child_b;
    
    // Tabs specific
    struct Tab {
        std::string label;
        std::function<void()> builder;
    };
    std::vector<Tab> tabs;
    int active_tab = 0;

    DockNode() : id(ID::Generate()) {}
};

class DockManager {
public:
    static DockManager& Get() {
        static DockManager instance;
        return instance;
    }

    struct DockSpace {
        std::string name;
        std::unique_ptr<DockNode> root;
    };

    DockSpace* get_or_create_space(const std::string& name) {
        for (auto& ds : m_spaces) {
            if (ds.name == name) return &ds;
        }
        m_spaces.push_back({name, std::make_unique<DockNode>()});
        return &m_spaces.back();
    }

private:
    std::vector<DockSpace> m_spaces;
};

} // namespace internal
} // namespace fanta
