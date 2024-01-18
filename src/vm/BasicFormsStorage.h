#pragma once

#include "../common.h"
#include "shared/Error.h"
#include "vm/BasicForm.h"

namespace vm {

template <typename T>
class IdKeyedStorage {
   public:
    void add(int64_t id, gsl::owner<T> item) { _map[id] = item; }

    T find(int64_t id) {
        auto it = _map.find(id);
        if (it == _map.end()) {
            throwNotFoundError();
        }
        return it->second;
    }

    void remove(int64_t id) {
        if (isClearing) {
            return;
        }

        delete _map[id];
        _map.erase(id);
    }

    bool isClearing = false;

    void clear() {
        assert(!isClearing);

        // They try to remove themselves from the map in their destructors, so set a
        // flag that disables that behavior here. We will erase from the map.
        isClearing = true;
        for (auto& [id, item] : _map) {
            delete item;
        }
        isClearing = false;

        _map.clear();
    }

   protected:
    virtual void throwNotFoundError() = 0;

   private:
    std::unordered_map<int64_t, T> _map;
};

class IdKeyedFormStorage : public IdKeyedStorage<BasicForm*> {
   protected:
    inline void throwNotFoundError() override {
        throw shared::Error(shared::ErrorCode::kFormNotFound, "The specified form does not exist.");
    }
};

class IdKeyedControlStorage : public IdKeyedStorage<TView*> {
   protected:
    inline void throwNotFoundError() override {
        throw shared::Error(shared::ErrorCode::kControlNotFound, "The specified control does not exist.");
    }
};

class BasicFormsStorage {
   public:
    inline int64_t nextId() { return _nextId++; }
    IdKeyedFormStorage forms{};
    IdKeyedControlStorage controls{};
    void clear();

   private:
    int64_t _nextId = 1;
};

extern BasicFormsStorage basicFormsStorage;

}  // namespace vm
