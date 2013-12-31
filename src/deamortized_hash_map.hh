#include <memory>

template<typename Key, typename Val>
struct base_hash_map {
  base_hash_map();
  struct Unit;
  Unit* find(const Key &);
  void weak_erase(Unit* location);
  Unit* insert(const Key&, const Val &);
};

