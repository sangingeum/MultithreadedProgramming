#pragma once
#include <list>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <algorithm>
#include <stdexcept>
#include <unordered_map>

/* // How to specialize std::hash<T>
class Student {
public:
    size_t id;
    std::string name;
    bool operator==(const Student& other) const {
        return id == other.id;
    }
};

namespace std {
    template<>
    struct hash<Student> {
        size_t operator()(const Student& student) const {
            return hash<size_t>()(student.id);
        }
    };
}
*/

// Thread-safe hash map
template<class Key, class Value, class Hash = typename std::hash<Key>>
class TSHashMap
{
private:
    using Entry = typename std::pair<Key, Value>;
    using BucketIterator = typename std::list<Entry>::iterator;
	class Bucket {
    private:
        std::list<Entry> m_data;
        mutable std::shared_mutex m_mutex;
        BucketIterator find(const Key& key) {
            return std::find_if(m_data.begin(), m_data.end(), 
                [&key](const Entry& entry) { 
                    return (entry.first == key); 
                }
            );
        }
    public:
        void addOrUpdate(const Key& key, const Value& value) {
            std::unique_lock lock{m_mutex};
            auto found = find(key);
            if (found == m_data.end()) {
                m_data.emplace_back(key, value);
            }
            else {
                found->second = value;
            }
        }
        void remove(const Key& key) {
            std::unique_lock lock{m_mutex};
            auto found = find(key);
            if (found != m_data.end()) {
                m_data.erase(found);
            }
        }
        Value get(const Key& key) {
            std::shared_lock lock{m_mutex};
            auto found = find(key);
            if (found != m_data.end()) {
                return found->second;
            }
            else {
                throw std::out_of_range("Entry does not exist");
            }
        }
        void snapShot(std::unordered_map<Key, Value, Hash>& map) {
            std::shared_lock lock{m_mutex};
            for (auto it = m_data.begin(); it != m_data.end(); ++it) {
                map.insert(*it);
            }
        }
	};

    size_t m_numBuckets;
    std::vector<Bucket> m_buckets;
    Hash m_hasher;
public:
    TSHashMap(size_t numBuckets=19) : m_numBuckets(numBuckets), m_buckets(numBuckets){}
    TSHashMap(const TSHashMap&) = delete;
    TSHashMap& operator=(const TSHashMap&) = delete;
    void addOrUpdate(const Key& key, const Value& value) {
        getBucket(key).addOrUpdate(key, value);
    }
    void remove(const Key& key) {
        getBucket(key).remove(key);
    }
    Value get(const Key& key) {
        return getBucket(key).get(key);
    }
    std::unordered_map<Key, Value, Hash> snapShot() {
        std::unordered_map<Key, Value, Hash> result;
        for (auto& bucket : m_buckets) {
            bucket.snapShot(result);
        }
        return result;
    }

private:
    inline size_t hash(const Key& key) {
        return (m_hasher(key) % m_numBuckets);
    }
    Bucket& getBucket(const Key& key) {
        return m_buckets[hash(key)];
    }
};

