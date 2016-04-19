#include <vector>
#include <list>
#include <iostream>
#include "pipes.hxx"

using namespace pipes;
using namespace std;

void basic()
{
  string s = 
  4
  | [](int x){ return vector<string>(x, "muffin"); }
  | reduce([](string x, string y) { return x + "\n" + y; });

  cout << s << endl;
}

void gross_basic()
{
  string s = 
  reduce( [](string x, string y) { return x + "\n" + y; } )(
    vector<string>(4, "muffin") 
  );

  cout << s << endl;
}

void garden()
{
  struct Plant { string name; size_t quantity; };
  using Landscape = vector<Plant>;

  Landscape yard
  {
    {"tulip", 150},
    {"weed", 62},
    {"tomato", 47},
    {"basil", 55},
    {"rose", 15}
  };

  auto garden =
    yard
    | filter( [](Plant p){ return p.name == "weed"; } ) //pick the weeds
    | map( [](Plant p){ p.quantity *= 1.5; return p; } ) //grow
    | map( [](Plant p){ 
        return p.name + "(" + to_string(p.quantity) + ")"; } ) //plant -> str
    | reduce( [](string x, string y){ return x + "\n" + y; }) //[str] -> str
    ; 
  
  cout << garden << endl;
}

int main()
{
  basic();
  cout << endl;
  gross_basic();
  cout << endl;
  garden();
}
