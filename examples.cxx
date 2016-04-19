#include <vector>
#include <list>
#include <iostream>
#include "pipes.hxx"

using namespace pipes;
using namespace std;

int main()
{
  struct ingredient { string name; size_t quantity; };
  using whopper = vector<ingredient>;
  whopper standard = {
    {"patty", 1},
    {"tomato", 3},
    {"pickles", 5},
    {"lettuce", 2},
    {"mayo", 4},
  };

  auto myWhopper = 
    standard
    | filter([](auto i) { return i.name == "pickles"; })
    | map([](auto i) { i.quantity *= 2; return i; })
    | sort([](auto x, auto y) { return x.quantity < y.quantity; })
    | map([](auto i) { return i.name + "(" + to_string(i.quantity) + ")"; })
    | reduce([](auto x, auto y) { return x + "\n" + y; });
  

  cout << myWhopper << endl;
}
