// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <memory>
#include <unordered_map>
#include <functional>

namespace moonray_gui_v2 {

template <typename K, typename V, typename HashK = std::hash<K>, typename HashV = std::hash<V>>
class BijectiveMap {
private:
    std::unordered_map<K, V, HashK> keyMap;
    std::unordered_map<V, K, HashV> valMap;
public:
    // Inserts a key-value pair. Removes existing key or value if they exist
    bool insert(const K& key, const V& val) {
        removeByKey(key);
        removeByValue(val);
        keyMap.insert({key, val});
        valMap.insert({val, key});
        return true;
    }

    // Removes a key-value pair by key
    bool removeByKey(const K& key) {
        auto it = keyMap.find(key);
        if (it == keyMap.end()) {
            return false; // Key not found
        }
        V val = it->second;
        keyMap.erase(it);
        valMap.erase(val);
        return true;
    }

    // Removes a key-value pair by value
    bool removeByValue(const V& val) {
        auto it = valMap.find(val);
        if (it == valMap.end()) {
            return false; // Value not found
        }
        K key = it->second;
        valMap.erase(it);
        keyMap.erase(key);
        return true;
    }

    // Checks if the map has a key
    bool hasKey(const K& key) const {
        return keyMap.count(key) > 0;
    }

    // Checks if the map has a value
    bool hasValue(const V& val) const {
        return valMap.count(val) > 0;
    }

    // Get value by key
    V getValue(const K& key) const {
        return keyMap.at(key);
    }

    // Get key by value
    K getKey(const V& val) const {
        return valMap.at(val);
    }

    // Crawls the map and applies a function to each key-value pair
    void forEach(const std::function<void(const K&, const V&)>& func) const {
        for (const auto& pair : keyMap) {
            func(pair.first, pair.second);
        }
    }
};

} // end namespace moonray_gui_v2
