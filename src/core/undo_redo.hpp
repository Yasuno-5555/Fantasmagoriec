#pragma once

#include <memory>
#include <vector>
#include <string>
#include <functional>

namespace fanta {

// Base Command Interface
class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual std::string name() const { return "Command"; }
};

// Undo/Redo Manager
class UndoManager {
public:
    static UndoManager& instance();
    
    // Execute a command and add to history
    void execute(std::unique_ptr<Command> cmd);
    
    // Undo last command
    bool undo();
    
    // Redo last undone command
    bool redo();
    
    // Check if undo/redo available
    bool canUndo() const;
    bool canRedo() const;
    
    // Get command names for menu display
    std::string getUndoName() const;
    std::string getRedoName() const;
    
    // Clear all history
    void clear();
    
    // Set max history size (default: 100)
    void setMaxHistory(size_t max);
    
private:
    UndoManager() = default;
    
    std::vector<std::unique_ptr<Command>> undo_stack;
    std::vector<std::unique_ptr<Command>> redo_stack;
    size_t max_history = 100;
};

// Convenience functions
void PushCommand(std::unique_ptr<Command> cmd);
void Undo();
void Redo();
bool CanUndo();
bool CanRedo();

// Lambda-based Command for quick use
class LambdaCommand : public Command {
public:
    LambdaCommand(
        std::string name,
        std::function<void()> do_fn,
        std::function<void()> undo_fn
    ) : cmd_name(std::move(name)), do_func(std::move(do_fn)), undo_func(std::move(undo_fn)) {}
    
    void execute() override { if (do_func) do_func(); }
    void undo() override { if (undo_func) undo_func(); }
    std::string name() const override { return cmd_name; }
    
private:
    std::string cmd_name;
    std::function<void()> do_func;
    std::function<void()> undo_func;
};

} // namespace fanta
