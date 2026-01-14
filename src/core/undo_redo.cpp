#include "undo_redo.hpp"

namespace fanta {

UndoManager& UndoManager::instance() {
    static UndoManager mgr;
    return mgr;
}

void UndoManager::execute(std::unique_ptr<Command> cmd) {
    if (!cmd) return;
    
    cmd->execute();
    undo_stack.push_back(std::move(cmd));
    
    // Clear redo stack on new command
    redo_stack.clear();
    
    // Trim history if needed
    while (undo_stack.size() > max_history) {
        undo_stack.erase(undo_stack.begin());
    }
}

bool UndoManager::undo() {
    if (undo_stack.empty()) return false;
    
    auto cmd = std::move(undo_stack.back());
    undo_stack.pop_back();
    
    cmd->undo();
    redo_stack.push_back(std::move(cmd));
    
    return true;
}

bool UndoManager::redo() {
    if (redo_stack.empty()) return false;
    
    auto cmd = std::move(redo_stack.back());
    redo_stack.pop_back();
    
    cmd->execute();
    undo_stack.push_back(std::move(cmd));
    
    return true;
}

bool UndoManager::canUndo() const {
    return !undo_stack.empty();
}

bool UndoManager::canRedo() const {
    return !redo_stack.empty();
}

std::string UndoManager::getUndoName() const {
    if (undo_stack.empty()) return "";
    return undo_stack.back()->name();
}

std::string UndoManager::getRedoName() const {
    if (redo_stack.empty()) return "";
    return redo_stack.back()->name();
}

void UndoManager::clear() {
    undo_stack.clear();
    redo_stack.clear();
}

void UndoManager::setMaxHistory(size_t max) {
    max_history = max;
    while (undo_stack.size() > max_history) {
        undo_stack.erase(undo_stack.begin());
    }
}

// Convenience functions
void PushCommand(std::unique_ptr<Command> cmd) {
    UndoManager::instance().execute(std::move(cmd));
}

void Undo() {
    UndoManager::instance().undo();
}

void Redo() {
    UndoManager::instance().redo();
}

bool CanUndo() {
    return UndoManager::instance().canUndo();
}

bool CanRedo() {
    return UndoManager::instance().canRedo();
}

} // namespace fanta
