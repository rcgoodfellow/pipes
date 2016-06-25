#ifndef LIBDNA_PIPES_HXX
#define LIBDNA_PIPES_HXX

#include <vector>
#include <functional>
#include <numeric>
#include <algorithm>
#include <unordered_map>
#include <type_traits>
#include <experimental/optional>

namespace pipes
{
  template< typename C, typename F>
  auto
  operator | (C && c, F f)
  {
    return f(c);
  }

  //collect --------------------------------------------------------------------
  template < 
    typename A,
    typename F,
    template <typename, typename...> class C, 
    template <typename, typename...> class CC = C,
    typename ...AT
  >
  auto
  do_collect (const C<A,AT...> & c, F f) 
  {
    using T = typename std::result_of<F(A&)>::type::value_type;
    CC<T> result;
    result.reserve(c.size());
    for(const auto & x : c)
    {
      auto xx = f(x);
      if(xx) result.insert(result.end(), *xx);
    }
    return result;
  }

  template<typename F>
  auto collect(F f)
  {
    return [f](auto & c){ return do_collect(c, f); };
  }


  //map ------------------------------------------------------------------------
  template < 
    typename A,
    typename F,
    template <typename, typename...> class C, 
    template <typename, typename...> class CC = C,
    typename ...AT
  >
  auto
  do_map (const C<A,AT...> & c, F f) 
  {
    using T = typename std::result_of<F(A&)>::type;
    CC<T> result;
    result.reserve(c.size());
    std::transform(c.begin(), c.end(), std::inserter(result, result.end()), f);
    return result;
  }
  
  template < 
    template <typename, typename...> class CC,
    typename A,
    typename F, 
    template <typename, typename...> class C,
    typename ...AT
  >
  auto
  do_map (const C<A,AT...> & c, F f) 
  {
    return do_map<A, F, C, CC, AT...>(c, f);
  }

  //TODO work this into a more general fold
  //for std::maps
  template < 
    template <typename, typename...> class CC,
    typename K, typename A,
    typename F, 
    typename ...AT
  >
  auto
  do_map (const std::unordered_map<K,A,AT...> & c, F f) 
  {
    using T = typename std::result_of<F(std::pair<K,A>&)>::type;
    CC<T> result;
    result.reserve(c.size());
    std::transform(c.begin(), c.end(), std::inserter(result, result.end()), f);
    return result;
  }

  template < typename F >
  auto
  map(F f)
  {
    return [f](auto & c){ return do_map(c, f); };
  }

  template < template <typename, typename...> class C, typename F >
  auto
  map(F f)
  {
    return [f](auto & c){ return do_map<C>(c, f); };
  }

  //flatmap -------------------------------------------------------------------
  template < 
    typename A,
    typename F,
    template <typename, typename...> class C, 
    template <typename, typename...> class CC = C,
    typename ...AT
  >
  auto
  do_flatmap (C<A,AT...> & c, F f) 
  {
    using CT = typename std::result_of<F(A&)>::type;
    CC<CT> result;
    result.reserve(c.size());
    std::transform(c.begin(), c.end(), std::inserter(result, result.end()), f);

    using T = typename CT::value_type;
    CC<T> flat_result;

    size_t sz{0};
    for(auto xs : result) sz += xs.size();
    flat_result.reserve(sz);

    for(auto xs : result) 
      flat_result.insert(flat_result.end(), xs.begin(), xs.end());

    return flat_result;
  }

  template < 
    template <typename, typename...> class CC,
    typename A,
    typename F, 
    template <typename, typename...> class C,
    typename ...AT
  >
  auto
  do_flatmap (C<A,AT...> & c, F f) 
  {
    return do_flatmap<A, F, C, CC, AT...>(c, f);
  }
  
  template < typename F >
  auto
  flatmap(F f)
  {
    return [f](auto & c){ return do_flatmap(c, f); };
  }

  template < template <typename, typename...> class C, typename F >
  auto
  flatmap(F f)
  {
    return [f](auto & c){ return do_flatmap<C>(c, f); };
  }
  

  //reduce --------------------------------------------------------------------
  template < 
    typename A,
    template <typename, typename...> class C, 
    typename F, typename ...AT
  >
  auto
  do_reduce(C<A,AT...> & c, F f)
  {
    switch(c.size())
    {
      case 0: return A{};
      case 1: return *c.begin();
      default: return std::accumulate(c.begin()+1, c.end(), *c.begin(), f);
    }
  }

  template < 
    typename A,
    template <typename, typename...> class C, 
    typename F, typename ...AT
  >
  auto
  do_reduce_first(C<A,AT...> & c, F f)
  {
    switch(c.size())
    {
      case 0: throw std::runtime_error{
                "reduce_first does not work for empty collections"};
      case 1: return *c.begin();
      default: return std::accumulate(c.begin()+1, c.end(), *c.begin(), f);
    }
  }

  template < typename F >
  auto
  reduce(F f)
  {
    return [f](auto && c) { return do_reduce(c, f); };
  }

  template < typename F >
  auto
  reduce_first(F f)
  {
    return [f](auto && c){ return do_reduce_first(c, f); };
  }

  auto plus = [](auto x, auto y){ return x + y; };

  //sort -----------------------------------------------------------------------
  template < 
    typename A,
    template <typename, typename...> class C, 
    typename F, typename ...AT
  >
  auto
  do_sort(const C<A,AT...> & c, F f)
  {
    auto cc = c;
    std::sort(cc.begin(), cc.end(), f);
    return cc;
  }
  
  template < 
    typename A,
    template <typename, typename...> class C, 
    typename ...AT
  >
  auto
  do_sort(const C<A,AT...> & c)
  {
    auto cc = c;
    std::sort(cc.begin(), cc.end());
    return cc;
  }
  
  template < typename F >
  auto
  sort(F f)
  {
    return [f](const auto & c){ return do_sort(c, f); };
  }
  
  auto
  inline
  sort()
  {
    return [](const auto & c){ return do_sort(c); };
  }

  //filter ---------------------------------------------------------------------
  template < 
    typename A,
    template <typename, typename...> class C, 
    typename F, typename ...AT
  >
  auto
  do_filter (C<A,AT...> & c, F f) 
  {
    auto ff = [f](auto x){ return !f(x); };
    C<A,AT...> result;
    std::copy_if(
        c.begin(), 
        c.end(), 
        std::inserter(result, result.end()), 
        ff );
    return result;
  }

  template < typename F >
  auto
  filter(F f)
  {
    return [f](auto & c){ return do_filter(c, f); };
  }

  template<typename T>
  std::vector<T> range(T begin, T end)
  {
    std::vector<T> v(end - begin, T{});
    size_t i{0}; 
    for(T x=begin; x<end; ++x, ++i) v[i] = x;
    return v;
  }

  //foreach --------------------------------------------------------------------
  
  template <typename F>
  auto for_each(F f)
  {
    return 
      [f](auto & C) -> auto &
      { 
        for(auto x : C) f(x);
        return C;
      };
  }

}

#endif
