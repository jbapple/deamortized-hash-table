#include "tlsf_allocator.hpp"
#include "deamortized_hash_map.hh"

#include "util.hh"

#include <unordered_map>
#include <unordered_set>
#include <set>
#include <tuple>
#include <algorithm>

using namespace std;

struct notint {
  int x;
  notint(int x) : x(x) {}
  bool operator<(const notint& y) const { return x < y.x; }
  bool operator==(const notint& y) const { return x == y.x; }
};

struct notint_hash {
  size_t operator()(const notint& x) const {
    const static hash<int> y = hash<int>();
    return y(x.x);
  }
};

template<typename S, typename T>
void test(const size_t max_size, const size_t samples) {
  size_t sample[samples];
  for (size_t i = 0; i < samples; ++i) {
    __sync_synchronize();
    const size_t begin = get_time();
    __sync_synchronize();
    T init;
    __sync_synchronize();
    const size_t end = get_time();
    __sync_synchronize();
    sample[i] = end-begin;
  }
  std::sort(sample, sample+samples);
  cout << 0 << '\t' << sample[samples/2] << endl;

  size_t last_size = 0;
  unordered_set<S, notint_hash> data(max_size);
  while (data.size() < max_size) {
    data.insert(rand());
  }

  for (double size = 1.0; size < max_size; size *= 1.5) {
    const size_t int_size = size;
    if (int_size <= last_size) continue;
    last_size = int_size;

    for (size_t i = 0; i < samples; ++i) {
      sample[i] = 0;
    }

    for (size_t j = 0; j < samples; ++j) {    T subject;
      size_t i = 0;
      for (const S x : data) {
        if (i > int_size) break;
        ++i;
        //if (subject.here->node_count > int_size) break;
        __sync_synchronize();
        const size_t begin = get_time();
        __sync_synchronize();
        //subject.insert(x);
        subject.insert(x, true);
        __sync_synchronize();
        const size_t end = get_time();
        __sync_synchronize();
        sample[j] = std::max(end-begin, sample[j]);
      }
    }
    std::sort(sample, sample+samples);
    cout << int_size << '\t' << sample[samples/2] << endl;
  }
}

struct none {};

int main(int argc, char ** argv) {
  srand(0);
  /*
  unsigned size = 100000;//00;
  unsigned samples = 1 << 3;
  if (4 == argc) {
  */
  size_t size = read<unsigned>(argv[1]);
  size_t samples = read<unsigned>(argv[2]);
    /*
}
  const unsigned which = read<unsigned>(argv[1]);
  */
  //test<unordered_set<int, hash<int>, equal_to<int>, TlsfAllocator<int> > >(size, samples);
  //test<notint, set<notint> >(size, samples);
  test<notint, base_hash_map<notint,bool, notint_hash> >(size, samples);
  //test<notint, deamortized_map<notint, bool, none, TlsfAllocator<notint>, std::less<notint> > >(size, samples);

  /*
  typedef hash_map<sample_type> table;
  typedef std::set<sample_type> tree;
  // typedef btree::btree_set<sample_type> btree;

  typedef quiet_map<sample_type, lazy_map<sample_type> > try1;
  //typedef quiet_map<sample_type, lazier_map<sample_type, BasicBitArray> > try2;
  typedef quiet_map<sample_type, lazier_map<sample_type, BasicBitArray, BasicArray<slot<sample_type > > > > try2;
  //typedef quiet_map<sample_type, lazier_map<sample_type, AhoBitArray> > try3;
  typedef quiet_map<sample_type, lazier_map<sample_type, TieredBitArray, BasicArray<slot<sample_type > > > > try4;
  typedef quiet_map<sample_type, lazier_map<sample_type, BasicBitArray, TieredArray<slot<sample_type > > > > try5; // good
  typedef quiet_map<sample_type, lazier_map<sample_type, TieredBitArray, TieredArray<slot<sample_type > > > > try6;
  typedef quiet_map<sample_type, lazier_map<sample_type, TieredBoolArray, TieredArray<slot<sample_type > > > > try7;
  typedef quiet_map<sample_type, lazier_map<sample_type, AhoBitArray, TieredArray<slot<sample_type > > > > try8;
  typedef quiet_map<sample_type, lazier_map<sample_type, TieredPackedBitArray, TieredArray<slot<sample_type > > > > try9;
  typedef quiet_map<sample_type, lazier_map<sample_type, ImplicitBitArray, TieredArray<slot<sample_type > > > > try10; // good
  typedef quiet_map<sample_type, lazier_map<sample_type, ImplicitBitArray, MmapArray<slot<sample_type > > > > try11; // bad
  typedef quiet_map<sample_type, lazier_map<sample_type, ImplicitBitArray, TieredMmapArray<slot<sample_type > > > > try12; // not better than 10
  // to try: tiered mmap bitarray at larger sizes

  switch(which) {
  case 0:
    test<table>(size, samples);
    break;
  case 1:
    test<tree>(size, samples);
    break;
  case 2:
    test<try1>(size, samples);
    break;
  case 3:
    test<try2>(size, samples);
    break;
  case 4:
    test<try4>(size, samples);
    break;
  case 5:
    test<try5>(size, samples);
    break;
  case 6:
    test<try10>(size, samples);
    break;
  case 7:
    test<unordered_set<sample_type, hash_some> >(size, samples);
    break; 
  case 8:
    test<try9>(size, samples);
    break; 
  // case 9:
  //   test<btree>(size, samples);
  //   break;
  }
  */
  /*
  //  print_test(
             test<
               //std::set<sample_type>, //, std::set<sample_type> >
               quiet_map<sample_type, lazier_map<sample_type, BasicBitArray> >
               //hash_map<sample_type>
               ,
               quiet_map<sample_type, lazier_map<sample_type, AhoBitArray> >
               //std::set<sample_type>
               //hash_map<sample_type>

                 >
                 
    (size, samples)
               //)
               ;
  */
/*
 >(size, samples));
  } else if ("aho" == container_type_string) {
    print_test(test<quiet_map<sample_type, lazier_map<sample_type, AhoBitArray> > >(size, samples));

  
  const std::string container_type_string = argv[1];
  
  if ("tree" == container_type_string) {
    print_test(test<std::set<sample_type> >(size, samples));
  } else if ("std::unordered_set" == container_type_string) {
    //print_test(test<std::unordered_set<sample_type> >(size, samples));
  } else if ("hash" == container_type_string) {
    print_test(test<hash_map<sample_type> >(size, samples));
  } else if ("lazy" == container_type_string) {
    print_test(test<quiet_map<sample_type, lazy_map<sample_type> > >(size, samples));
  } else if ("separate" == container_type_string) {
    print_test(test<quiet_map<sample_type, lazier_map<sample_type, BasicBitArray> > >(size, samples));
  } else if ("aho" == container_type_string) {
    print_test(test<quiet_map<sample_type, lazier_map<sample_type, AhoBitArray> > >(size, samples));
  } else {
    std::cerr << "No test";
  }
  */
}
