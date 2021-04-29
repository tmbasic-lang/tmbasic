#pragma once

#include "../common.h"

namespace tmbasic {

const size_t kUndoStackLimit = 100;

template <typename TDocument>
class UndoStack {
   public:
    bool canUndo() { return !_undos.empty(); }

    bool canRedo() { return !_redos.empty(); }

    void push(TDocument document) {
        _redos = {};
        _undos.push_back(std::move(document));
        if (_undos.size() > kUndoStackLimit) {
            _undos.erase(_undos.begin());
        }
    }

    void abandon() { _undos.pop_back(); }

    TDocument undo(TDocument current) {
        _redos.push(std::move(current));
        auto doc = std::move(_undos.at(_undos.size() - 1));
        _undos.pop_back();
        return doc;
    }

    TDocument redo(TDocument current) {
        _undos.push_back(std::move(current));
        auto doc = std::move(_redos.top());
        _redos.pop();
        return doc;
    }

   private:
    std::vector<TDocument> _undos{};
    std::stack<TDocument> _redos{};
};

}  // namespace tmbasic
