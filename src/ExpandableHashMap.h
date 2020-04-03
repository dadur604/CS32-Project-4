// ExpandableHashMap.h

// Skeleton for the ExpandableHashMap class template.  You must implement the first six
// member functions.

#include <list>
#include <string>
#include <iostream>

template<typename KeyType, typename ValueType>
class ExpandableHashMap
{
public:
    ExpandableHashMap(double maximumLoadFactor = 0.5);
    ~ExpandableHashMap();
    void reset();
    int size() const;
    void associate(const KeyType& key, const ValueType& value);

      // for a map that can't be modified, return a pointer to const ValueType
    const ValueType* find(const KeyType& key) const;

      // for a modifiable map, return a pointer to modifiable ValueType
    ValueType* find(const KeyType& key)
    {
        return const_cast<ValueType*>(const_cast<const ExpandableHashMap*>(this)->find(key));
    }

      // C++11 syntax for preventing copying and assignment
    ExpandableHashMap(const ExpandableHashMap&) = delete;
    ExpandableHashMap& operator=(const ExpandableHashMap&) = delete;

private:
    int m_numItems;
    int m_numBuckets;

    double m_maxLoad;

    struct KV {
        KeyType key;
        ValueType value;
    };

    std::list<KV>* m_map;

    unsigned int getBucket(const KeyType& key) const;
    void reallocate();
};

template <typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::ExpandableHashMap(double maximumLoadFactor)
{
    m_numItems = 0;
    m_numBuckets = 4;

    m_maxLoad = maximumLoadFactor;

    m_map = new std::list<KV>[m_numBuckets];
}

template <typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::~ExpandableHashMap()
{
    delete[] m_map;
}

template <typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::reset()
{
    delete[] m_map;
    m_numBuckets = 8;
    m_numItems = 0;
    m_map = new std::list<KV>[m_numBuckets];
}

template <typename KeyType, typename ValueType>
int ExpandableHashMap<KeyType, ValueType>::size() const
{
    return m_numItems; 
}

template <typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::associate(const KeyType& key, const ValueType& value)
{

    ValueType* existingVal = find(key);
    if (existingVal != nullptr) {
        *existingVal = value;
        return;
    }

    KV kv;
    kv.key = key;
    kv.value = value;

    unsigned int bucketNum = getBucket(key);
    m_map[bucketNum].push_back(kv);

    m_numItems++;

    if (m_numItems / static_cast<double>(m_numBuckets) > m_maxLoad) {
        reallocate();
    }
}

template <typename KeyType, typename ValueType>
const ValueType* ExpandableHashMap<KeyType, ValueType>::find(const KeyType& key) const
{
    unsigned int bucketNum = getBucket(key);
    for (auto it = m_map[bucketNum].begin(); it != m_map[bucketNum].end(); it++) {
        if ((*it).key == key)
            return &it->value;
    }

    return nullptr;
}

template <typename KeyType, typename ValueType>
unsigned int ExpandableHashMap<KeyType, ValueType>::getBucket(const KeyType& key) const {
    unsigned int hasher(const KeyType & key);
    unsigned int hash = hasher(key);
    return hash % m_numBuckets;
}

template <typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::reallocate() {
    std::cerr << "Reallocating!" << std::endl;
    int oldNumBuckets = m_numBuckets;
    m_numBuckets *= 2;
    std::list<KV>* newMap = new std::list<KV>[m_numBuckets * 2];

    for (int i = 0; i < oldNumBuckets; i++) {
        while(m_map[i].size() > 0) {
            auto it = m_map[i].begin();
            unsigned int newBucketNumber = getBucket(it->key);
            newMap[newBucketNumber].splice(newMap[newBucketNumber].begin(), m_map[i], it);
        }
    }

    delete[] m_map;

    m_map = newMap;
}