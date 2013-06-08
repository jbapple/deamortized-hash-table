#include "util.hh"

#include <unordered_map>
#include <unordered_set>
#include <set>
#include <tuple>

using namespace std;

//#include "linear-probing.cc"
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

template<typename T, typename U>
std::vector<std::tuple<unsigned, double, double> > test(const unsigned size, const unsigned samples) {
  unsigned i = 0;
  std::vector<std::tuple<unsigned, double, double> > ans((size * samples)); 
  high_priority zz;
  for (unsigned k = 0; k < samples; ++k) { 
    T p;
    U q;
    double l1 = 0.0, l2 = 0.0;
    for (unsigned j = 0; j < size; ++j) { 
      const auto x = some::random();
      double begin, mid, end;
      if (not (j & 1)) {
        begin = get_time();
        p.insert(x);
        mid = get_time(); 
        q.insert(x);
        end = get_time();
        l1 = std::max(l1, mid-begin);
        l2 = std::max(l2, end-mid);
      } else {
        begin = get_time();
        q.insert(x);
        mid = get_time(); 
        p.insert(x);
        end = get_time();
        l2 = std::max(l2, mid-begin);
        l1 = std::max(l1, end-mid);
      }
      ans[i++] = make_tuple(j,l1,l2);
    } 
  }
  return ans;
}

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

int main(int argc, char ** argv) {
  srand(0);
  unsigned size = 10000;//00;
  unsigned samples = 1000;
  if (4 == argc) {
    size = read<unsigned>(argv[2]);
    samples = read<unsigned>(argv[3]);
  }

  print_test(test<
               //std::set<sample_type>, //, std::set<sample_type> >
               quiet_map<sample_type, lazier_map<sample_type, BasicBitArray> >
               ,
               quiet_map<sample_type, lazier_map<sample_type, AhoBitArray> >

                 >
                 
    (size, samples));
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
