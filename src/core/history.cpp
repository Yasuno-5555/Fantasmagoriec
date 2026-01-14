#include "fanta.h"
#include "core/contexts_internal.hpp"

#include <iostream>
#include <vector>
#include <memory> 

namespace fanta {

    // Macro Compat
    #define g_ctx (fanta::internal::GetEngineContext())

    // Debug Logger
    using internal::Log;

    void Undo() {
        if (g_ctx.store.undo_stack.empty()) return;
        
        internal::StateDelta undo_delta = g_ctx.store.undo_stack.back();
        g_ctx.store.undo_stack.pop_back();
        
        internal::StateDelta redo_delta;
        for (auto const& [id, before_data] : undo_delta.changes) {
            // Capture current (to redo)
            redo_delta.changes[id] = g_ctx.store.persistent_states[id];
            // Apply before
            g_ctx.store.persistent_states[id] = before_data;
        }
        g_ctx.store.redo_stack.push_back(redo_delta);
        Log("Undo Applied");
    }

    void Redo() {
        if (g_ctx.store.redo_stack.empty()) return;

        internal::StateDelta redo_delta = g_ctx.store.redo_stack.back();
        g_ctx.store.redo_stack.pop_back();

        internal::StateDelta undo_delta;
        for (auto const& [id, redo_data] : redo_delta.changes) {
            undo_delta.changes[id] = g_ctx.store.persistent_states[id];
            g_ctx.store.persistent_states[id] = redo_data;
        }
        g_ctx.store.undo_stack.push_back(undo_delta);
        Log("Redo Applied");
    }

    bool CanUndo() { return !g_ctx.store.undo_stack.empty(); }
    bool CanRedo() { return !g_ctx.store.redo_stack.empty(); }

    void PushCommand(std::unique_ptr<Command> cmd) { (void)cmd; } // Legacy - bypassed

} // namespace fanta
