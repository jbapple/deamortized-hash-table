#include <utility>

#include "linear_hash_set.hh"

// TODO: boost::optional
// TODO: use bool here as bool of "present" in hash table
// TODO: use std::aligned_storage
template<typename Key, typename Value>
struct Both {
  Key key;
  char value[8 * ((sizeof(Value) + 7)/8)]; //aligned storage
  bool key_only;
  explicit Both(const Key & key) : key(key), value(), key_only(true) {}
  Both(const Key& key, const Value & v) 
    : key(key), value(), key_only(false) {
    new (&value) Value(v);
  }
  ~Both() {
    if (not key_only) {
      reinterpret_cast<Value*>(&value)->~Value();
    }
    key.~Key();
  }
  Both& operator=(const Both& that) {
    key_only = that.key_only;
    if (not that.key_only) {
      value = that.value;
    }
    key = that.key;
    return *this;
  }
  Both(const Both& that) : key(that.key), value(), key_only(that.key_only) {
    if (not key_only) {
      new (&value) Value(*reinterpret_cast<const Value *>(&that.value));
    }
  }   
  Both(Both&& that) : key(std::move(that.key)), key_only(that.key_only) {
    if (not key_only) {
      new (&value) Value(std::move(*reinterpret_cast<Value *>(&that.value)));
    }
  }
  Both& operator=(Both&& that) {
    key_only = that.key_only;
    if (not that.key_only) {
      value = std::move(that.value);
    }
    key = std::move(that.key);
    return *this;
  }
  Value & get_value() {
    return *reinterpret_cast<Value*>(&value);
  }
};



template<typename Hash>
struct HashKey {
  Hash hasher;
  template<typename Key, typename Value>
  size_t operator()(const Both<Key,Value>& kv) const {
    return hasher(kv.key);
  }
};

template<typename Equal>
struct EqualKey {
  Equal equaler;
  template<typename Key, typename Value>
  bool operator()(const Both<Key,Value>& lhs,
                  const Both<Key,Value>& rhs) const {
    return equaler(lhs.key, rhs.key);
  }
};

template<typename Key,
         typename Value,
         typename Hash = std::hash<Key>, 
         typename Equal = std::equal_to<Key> >
class LinearHashMap
{
private:   
  typedef Both<Key, Value> KeyValue;
  LinearHashSet<KeyValue, HashKey<Hash>, EqualKey<Equal> > root;
public:
  explicit LinearHashMap(const size_t capacity) : root(capacity) {}
  LinearHashMap() : root() {}
  //~LinearHashMap() {}
  bool insert(const Key& k, const Value & v) {
    return root.insert(KeyValue(k,v));
  }   
  void erase(const Key& k) {    
    root.erase(KeyValue(k));
  }
  KeyValue* find(const Key& k) {
    return root.find(KeyValue(k));
  }
  void swap(LinearHashMap& that) {
    root.swap(that.root);
  }
  
  LinearHashMap& operator=(const LinearHashMap&) = delete;
  LinearHashMap(const LinearHashMap&) = delete;
  LinearHashMap(LinearHashMap&&) = delete;
  LinearHashMap& operator=(LinearHashMap&&) = delete;
};
