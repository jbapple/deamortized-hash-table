#include "util.hh"

#include <unordered_map>
#include <unordered_set>
#include <set>
#include <tuple>

using namespace std;

#include "linear-probing.cc"
#include "lazy-linear.cc"

struct some {
  const static int some_size = 2;
  int x[some_size];
  static some random() {
    some ans;
    for (int i = 0; i < some::some_size; ++i) {
      ans.x[i] = rand();
    }
    return ans;
  }
  bool operator==(const some& that) const {
    for (int i = 0; i < some::some_size; ++i) {
      if (x[i] != that.x[i]) return false;
    }
    return true;
  }
  bool operator<(const some& that) const {
    for (int i = 0; i < some::some_size; ++i) {
      if (x[i] < that.x[i]) return true;
      if (x[i] > that.x[i]) return false;
    }
    return false;
  }

};

size_t hashf(const some & x) {
  size_t ans = 0;
  for (int i = 0; i < some::some_size; ++i) {
    ans ^= x.x[i];
  }
  return ans;
}

size_t hash(const some & x) {
  return hashf(x);
}

typedef some sample_type;

struct dummy {
  template<typename T>
  void insert(const T&) {}
};

size_t flatsub(const size_t& x, const size_t& y) {
  if (y > x) return 0;
  return x-y;
}

template<typename T, typename U>
void test(const unsigned size, const unsigned samples) {
  unsigned i = 0;
  high_priority zz;
  std::vector<T> p(samples);
  std::vector<U> q(samples);
  std::vector<dummy> r(samples);
  //std::vector<size_t> l1(samples,0), l2(samples,0);
  //std::vector<size_t> s1(samples,0), s2(samples,0);
  size_t l1(0), l2(0), l3(0), s1(0), s2(0), s3(0), t1(0), t2(0), t3(0);
  for (unsigned j = 0; j < size; ++j) { 
    const auto x = some::random();
    __sync_synchronize();
    const size_t begin = get_time();
    __sync_synchronize();
    for (unsigned k = 0; k < samples; ++k) { 
      p[k].insert(x);
    }
    __sync_synchronize();
    const size_t mid = get_time();
    __sync_synchronize();
    for (unsigned k = 0; k < samples; ++k) { 
      q[k].insert(x);
    }
    __sync_synchronize();
    const size_t end = get_time();
    __sync_synchronize();
    for (unsigned k = 0; k < samples; ++k) { 
      r[k].insert(x);
    }
    __sync_synchronize();
    const size_t last = get_time();
    __sync_synchronize();

    s3 = last - end;
    l3 = std::max(l3, s3);
    t3 = t3 + s3;

    s1 = flatsub(mid - begin, s3);
    s2 = flatsub(end - mid, s3);
    
    l1 = std::max(l1, s1);
    l2 = std::max(l2, s2);

    t1 = flatsub(t1 + mid - begin, s3);
    t2 = flatsub(t2 + end - mid, s3);
 
    cout << static_cast<double>(j)/1000.0 << '\t'
         << static_cast<double>(l1)/(1000.0 * static_cast<double>(samples)) << '\t'
         << static_cast<double>(l2)/(1000.0 * static_cast<double>(samples)) << '\t'
         << static_cast<double>(s1)/(1000.0 * static_cast<double>(samples)) << '\t'
         << static_cast<double>(s2)/(1000.0 * static_cast<double>(samples)) << '\t'
         << static_cast<double>(t1)/(1000.0 * static_cast<double>(samples * (j+1))) << '\t'
         << static_cast<double>(t2)/(1000.0 * static_cast<double>(samples * (j+1))) << endl;
  }
}

template<typename T, typename U>
void test_lookup(const unsigned size, const unsigned samples) {
  void * dummy = 0;
  unsigned i = 0;
  high_priority zz;
  T p;
  U q;
  decltype(p.find(some::random())) pf;
  decltype(q.find(some::random())) qf;
  std::vector<some> x(samples, some::random());
  for (size_t i = 0; i < samples; ++i) {
    x[i] = some::random();
  }
  std::vector<size_t> l1(samples,0), l2(samples,0);
  std::vector<size_t> s1(samples,0), s2(samples,0);
  for (unsigned j = 0; j < size; ++j) { 
    const auto y = some::random();
    p.insert(y), q.insert(y);
    for (unsigned k = 0; k < samples; ++k) { 
      size_t begin, mid, end;
      if ((j & 1)) {
        begin = get_time();
        pf = p.find(x[k]);
        mid = get_time(); 
        qf = q.find(x[k]);
        end = get_time();
        s1[k] = mid-begin;
        s2[k] = end-mid;
        l1[k] = std::max(l1[k], mid-begin);
        l2[k] = std::max(l2[k], end-mid);
      } else {
        begin = get_time();
        qf = q.find(x[k]);
        mid = get_time(); 
        pf = p.find(x[k]);
        end = get_time();
        s2[k] = mid-begin;
        s1[k] = end-mid;
        l2[k] = std::max(l2[k], mid-begin);
        l1[k] = std::max(l1[k], end-mid);
      }
    }
    cout << static_cast<double>(j)/1000.0 << '\t'
         << avg(l1)/1000.0 << '\t'
         << avg(l2)/1000.0 << '\t' 
         << avg(s1)/static_cast<double>(1000) << '\t'
         << avg(s2)/static_cast<double>(1000) << endl;
  }
}

/*
void print_test(std::vector<std::tuple<unsigned, double, double> > x) {
  std::unordered_map<unsigned, std::pair<std::vector<double>, std::vector<double> > > collect;    
  for (const auto& y : x) {
    collect[get<0>(y)].first.push_back(get<1>(y));
    collect[get<0>(y)].second.push_back(get<2>(y));
  }
  for (const auto& y : collect) {
    std::cout << y.first << '\t' 
              << avg(y.second.first) << '\t'
              << avg(y.second.second) << std::endl;
  }
}
*/

int main(int argc, char ** argv) {
  srand(0);
  unsigned size = 1000000;//00;
  unsigned samples = 1 << 5;
  if (4 == argc) {
    size = read<unsigned>(argv[2]);
    samples = read<unsigned>(argv[3]);
  }
  const unsigned which = read<unsigned>(argv[1]);

  typedef hash_map<sample_type> table;
  typedef std::set<sample_type> tree;

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

  switch(which) {
  case 0:
    test<table, tree>(size, samples);
    break;
  case 1:
    test<tree, dummy>(size, samples);
    break;
  case 2:
    test<table, try2>(size, samples);
    break;
  case 3:
    test<try2, tree>(size, samples);
    break;
  case 4:
    test<table, dummy>(size, samples);
    break;
  case 5:
    test<try2, dummy>(size, samples);
    break;
  case 6:
    test<try5, dummy>(size, samples);
    break;
  case 7:
    test<try10, dummy>(size, samples);
    break;
  }

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
