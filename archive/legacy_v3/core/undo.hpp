// Fantasmagorie v2 - Undo/Redo System
// Automatic state management with command pattern
#pragma once

#include <functional>
#include <vector>
#include <string>
#include <memory>

namespace fanta {

// ============================================================================
// Command Interface
// ============================================================================

class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual std::string description() const = 0;
};

// ============================================================================
// Value Change Command
// ============================================================================

template<typename T>
class ValueChangeCommand : public Command {
public:
    ValueChangeCommand(T* target, T old_value, T new_value, const char* desc)
        : target_(target), old_value_(old_value), new_value_(new_value), desc_(desc) {}
    
    void execute() override { *target_ = new_value_; }
    void undo() override { *target_ = old_value_; }
    std::string description() const override { return desc_; }
    
private:
    T* target_;
    T old_value_;
    T new_value_;
    std::string desc_;
};

// ============================================================================
// Command Stack (Undo/Redo Manager)
// ============================================================================

class CommandStack {
public:
    static CommandStack& instance() {
        static CommandStack s;
        return s;
    }
    
    void push(std::unique_ptr<Command> cmd) {
        // Clear redo stack when new command is pushed
        redo_stack_.clear();
        cmd->execute();
        undo_stack_.push_back(std::move(cmd));
        
        // Limit stack size
        if (undo_stack_.size() > max_size_) {
            undo_stack_.erase(undo_stack_.begin());
        }
    }
    
    bool can_undo() const { return !undo_stack_.empty(); }
    bool can_redo() const { return !redo_stack_.empty(); }
    
    void undo() {
        if (!can_undo()) return;
        auto cmd = std::move(undo_stack_.back());
        undo_stack_.pop_back();
        cmd->undo();
        redo_stack_.push_back(std::move(cmd));
    }
    
    void redo() {
        if (!can_redo()) return;
        auto cmd = std::move(redo_stack_.back());
        redo_stack_.pop_back();
        cmd->execute();
        undo_stack_.push_back(std::move(cmd));
    }
    
    void clear() {
        undo_stack_.clear();
        redo_stack_.clear();
    }
    
    size_t undo_count() const { return undo_stack_.size(); }
    size_t redo_count() const { return redo_stack_.size(); }
    
private:
    CommandStack() = default;
    std::vector<std::unique_ptr<Command>> undo_stack_;
    std::vector<std::unique_ptr<Command>> redo_stack_;
    size_t max_size_ = 100;
};

// ============================================================================
// Global Undo/Redo Functions
// ============================================================================

inline void undo() { CommandStack::instance().undo(); }
inline void redo() { CommandStack::instance().redo(); }
inline bool can_undo() { return CommandStack::instance().can_undo(); }
inline bool can_redo() { return CommandStack::instance().can_redo(); }

// ============================================================================
// Undoable Value Wrapper
// ============================================================================

template<typename T>
class Undoable {
public:
    Undoable(T* value, const char* name = "Value Change") 
        : value_(value), old_value_(*value), name_(name) {}
    
    ~Undoable() {
        if (*value_ != old_value_) {
            auto cmd = std::make_unique<ValueChangeCommand<T>>(
                value_, old_value_, *value_, name_
            );
            // Don't execute - value already changed
            CommandStack::instance().undo_stack_.push_back(std::move(cmd));
        }
    }
    
    T& get() { return *value_; }
    const T& get() const { return *value_; }
    
    operator T&() { return *value_; }
    
private:
    T* value_;
    T old_value_;
    const char* name_;
};

} // namespace fanta
