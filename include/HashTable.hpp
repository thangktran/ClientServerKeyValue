#pragma once

#include <optional>
#include <string_view>
#include <string>
#include <memory>
#include <list>
#include <shared_mutex>
#include <mutex>


namespace cskv {

typedef struct Entry {
    unsigned key;
    std::string value;
} Entry;


class HashTable {
public:
    HashTable(size_t size);
    ~HashTable();

    bool insert(unsigned key, std::string_view value);
    std::optional<std::string> get(unsigned key);
    void del(unsigned key);

private:
    unsigned simpleHash(unsigned key);

    std::unique_ptr<std::list<Entry>[]> _table;
    size_t _size;
    std::shared_mutex _sharedMutex;
};


} // namespace cskv