#pragma once
#include <list>
#include <vector>
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
	// typedefs
	using Entry = typename std::pair<Key, Value>;
	using BucketIterator = typename std::list<Entry>::iterator;
	// Bucket class
	class Bucket {
	private:
		std::list<Entry> m_data;
		mutable std::shared_mutex m_mutex;
		BucketIterator find(const Key& key);
	public:
		bool addOrUpdate(const Key& key, const Value& value);
		bool remove(const Key& key);
		Value get(const Key& key);
		bool get(const Key& key, Value& result);
		void snapShot(std::unordered_map<Key, Value, Hash>& map);
	};

	// Private members
	size_t m_numBuckets;
	std::vector<Bucket> m_buckets;
	Hash m_hasher;
public:
	TSHashMap(size_t numBuckets = 19) : m_numBuckets(numBuckets), m_buckets(numBuckets) {}
	TSHashMap(const TSHashMap&) = delete;
	TSHashMap& operator=(const TSHashMap&) = delete;
	// Modification operations [exclusive lock]
	bool addOrUpdate(const Key& key, const Value& value);
	bool remove(const Key& key);
	// Retrieve operations [shared lock]
	Value get(const Key& key);
	bool get(const Key& key, Value& result);
	std::unordered_map<Key, Value, Hash> snapShot();
private:
	inline size_t hash(const Key& key);
	Bucket& getBucket(const Key& key);
};

template<class Key, class Value, class Hash>
TSHashMap<Key, Value, Hash>::BucketIterator TSHashMap<Key, Value, Hash>::Bucket::find(const Key& key) {
	return std::find_if(m_data.begin(), m_data.end(),
		[&key](const Entry& entry) {
			return (entry.first == key);
		}
	);
}

template<class Key, class Value, class Hash>
bool TSHashMap<Key, Value, Hash>::Bucket::addOrUpdate(const Key& key, const Value& value) {
	std::unique_lock lock{m_mutex};
	auto found = find(key);
	if (found == m_data.end()) {
		m_data.emplace_back(key, value);
		return true;
	}
	else {
		found->second = value;
		return false;
	}
}

template<class Key, class Value, class Hash>
bool TSHashMap<Key, Value, Hash>::Bucket::remove(const Key& key) {
	std::unique_lock lock{m_mutex};
	auto found = find(key);
	if (found != m_data.end()) {
		m_data.erase(found);
		return true;
	}
	return false;
}

template<class Key, class Value, class Hash>
Value TSHashMap<Key, Value, Hash>::Bucket::get(const Key& key) {
	std::shared_lock lock{m_mutex};
	auto found = find(key);
	if (found != m_data.end()) {
		return found->second;
	}
	else {
		throw std::out_of_range("Entry does not exist");
	}
}

template<class Key, class Value, class Hash>
bool TSHashMap<Key, Value, Hash>::Bucket::get(const Key& key, Value& result) {
	std::shared_lock lock{m_mutex};
	auto found = find(key);
	if (found != m_data.end()) {
		result = found->second;
		return true;
	}
	return false;
}

template<class Key, class Value, class Hash>
void TSHashMap<Key, Value, Hash>::Bucket::snapShot(std::unordered_map<Key, Value, Hash>& map) {
	std::shared_lock lock{m_mutex};
	for (auto it = m_data.begin(); it != m_data.end(); ++it) {
		map.insert(*it);
	}
}



template<class Key, class Value, class Hash>
bool TSHashMap<Key, Value, Hash>::addOrUpdate(const Key& key, const Value& value) {
	return getBucket(key).addOrUpdate(key, value);
}

template<class Key, class Value, class Hash>
bool TSHashMap<Key, Value, Hash>::remove(const Key& key) {
	return getBucket(key).remove(key);
}

template<class Key, class Value, class Hash>
Value TSHashMap<Key, Value, Hash>::get(const Key& key) {
	return getBucket(key).get(key);
}

template<class Key, class Value, class Hash>
bool TSHashMap<Key, Value, Hash>::get(const Key& key, Value& result) {
	return getBucket(key).get(key, result);
}

template<class Key, class Value, class Hash>
std::unordered_map<Key, Value, Hash> TSHashMap<Key, Value, Hash>::snapShot() {
	std::unordered_map<Key, Value, Hash> result;
	for (auto& bucket : m_buckets) {
		bucket.snapShot(result);
	}
	return result;
}

template<class Key, class Value, class Hash>
inline size_t TSHashMap<Key, Value, Hash>::hash(const Key& key) {
	return (m_hasher(key) % m_numBuckets);
}

template<class Key, class Value, class Hash>
typename TSHashMap<Key, Value, Hash>::Bucket& TSHashMap<Key, Value, Hash>::getBucket(const Key& key) {
	return m_buckets[hash(key)];
}

