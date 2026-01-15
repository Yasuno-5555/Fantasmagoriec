#pragma once
#include <vector>
#include <functional>
#include <string>
#include <memory>

namespace core {

    struct Command {
        std::string name;
        std::function<void()> undo;
        std::function<void()> redo;
    };

    class CommandStack {
    public:
        void push(const std::string& name, std::function<void()> undo_fn, std::function<void()> redo_fn) {
            // If we are not at the top, clear redo history
            if (current != commands.size()) {
                commands.resize(current);
            }
            
            Command cmd;
            cmd.name = name;
            cmd.undo = undo_fn;
            cmd.redo = redo_fn; // Redo is usually the action just performed, or a lambda effectively re-doing it
            
            commands.push_back(cmd);
            current++;
            
            // Limit stack?
            if (commands.size() > 100) {
                 commands.erase(commands.begin());
                 current--;
            }
        }
        
        void undo() {
            if (current > 0) {
                current--;
                commands[current].undo();
            }
        }
        
        void redo() {
            if (current < commands.size()) {
                commands[current].redo();
                current++;
            }
        }
        
        bool can_undo() const { return current > 0; }
        bool can_redo() const { return current < commands.size(); }
        
        const std::string& get_undo_name() const {
             static std::string empty = "";
             if (can_undo()) return commands[current-1].name;
             return empty;
        }

    private:
        std::vector<Command> commands;
        size_t current = 0; // Points to the next slot for "push" (or the undoable items count)
    };
    
    // Global instance or managed by App?
    // Let's keep it header-only for now or managed by Application class.
}
