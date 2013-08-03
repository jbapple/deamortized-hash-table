#include "linear_hash_set.hh"

struct foo {
  const int x;
  const double y;
  foo(int x, double y) : x(x), y(y) {}
  //foo& operator=(const foo&) = delete;
  //foo(const foo& that) : x(that.x), y(that.y) {}
  //foo(foo &&) = delete;
  //foo& operator=(foo&&) = delete;
  bool operator==(const foo& that) const {
    return x == that.x and y == that.y;
  }
};

struct foo_hash {
  std::hash<int> int_hash;
  std::hash<double> double_hash;
  size_t operator()(const foo& x) const {
    return int_hash(x.x) ^ double_hash(x.y);
  }
};

int main() {
  LinearHashSet<foo, foo_hash> x;
  LinearHashSet<foo, foo_hash> y(8);
  x.insert(foo(1, 2.0));
  x.erase(foo(3, 4.0));
  y.member(foo(5, 6.0));
}
