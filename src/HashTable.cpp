#include "include/HashTable.hpp"

#include <algorithm>


namespace cskv {


HashTable::HashTable(size_t size)
    : 
    _table (nullptr),
    _size(size) 
{
    _table = std::make_unique<std::list<Entry>[]>(_size);
}


HashTable::~HashTable() {

}


bool HashTable::insert(unsigned key, std::string_view value) {

    std::unique_lock lck(_sharedMutex);

    size_t hash = simpleHash(key);
    auto& bucket = _table[hash];

    // search if key already exists.
    auto it = std::find_if(bucket.begin(), bucket.end(), [key](const Entry& e) {
        return e.key == key;
    });

    if (it == bucket.end()) {
        Entry e;
        e.key = key;
        e.value = value;
        bucket.emplace_back(e);
        return true;
    }

    // else replace the value.
    it->value = value;
    return true;

}


std::optional<std::string> HashTable::get(unsigned key) {

    std::shared_lock lck(_sharedMutex);

    size_t hash = simpleHash(key);
    auto& bucket = _table[hash];

    auto it = std::find_if(bucket.begin(), bucket.end(), [key](const Entry& e) {
        return e.key == key;
    });

    if (it == bucket.end()) {
        return std::nullopt;
    }

    return it->value;

}


void HashTable::del(unsigned key) {

    std::unique_lock lck(_sharedMutex);

    size_t hash = simpleHash(key);
    auto& bucket = _table[hash];

    bucket.remove_if([key](const Entry& e) {
        return e.key == key;
    });
}


unsigned HashTable::simpleHash(unsigned key) {
    return key % _size;
}


} // namespace cskv