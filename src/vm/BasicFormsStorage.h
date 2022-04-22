#pragma once

#include "../common.h"
#include "BasicForm.h"
#include "Error.h"

namespace vm {

template <typename T>
class IdKeyedStorage {
   public:
    void add(int64_t id, T item) { _map[id] = item; }

    T find(int64_t id) {
        auto it = _map.find(id);
        if (it == _map.end()) {
            throwNotFoundError();
        }
        return it->second;
    }

    void remove(int64_t id) { _map.erase(id); }

   protected:
    virtual void throwNotFoundError() = 0;

   private:
    std::unordered_map<int64_t, T> _map;
};

class IdKeyedFormStorage : public IdKeyedStorage<BasicForm*> {
   protected:
    inline void throwNotFoundError() override {
        throw Error(ErrorCode::kFormNotFound, "The specified form does not exist.");
    }
};

class IdKeyedControlStorage : public IdKeyedStorage<TView*> {
   protected:
    inline void throwNotFoundError() override {
        throw Error(ErrorCode::kControlNotFound, "The specified control does not exist.");
    }
};

class BasicFormsStorage {
   public:
    inline int64_t nextId() { return _nextId++; }
    IdKeyedFormStorage forms{};
    IdKeyedControlStorage controls{};

   private:
    int64_t _nextId = 1;
};

extern BasicFormsStorage basicFormsStorage;

}  // namespace vm
