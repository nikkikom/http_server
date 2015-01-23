#include <boost/utility/result_of.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/bool.hpp>
using namespace boost; 
using mpl::true_;
using mpl::false_;

template <class, class ResultOf = bool> struct sig_int : false_ {};

template <class T>
struct sig_int<T, typename result_of<T(int)>::type> : true_ {};

template< class F >
typename enable_if<sig_int<F>, void >::type
f (F const& x)
{
  x(1);
}

template <class, class ResultOf = bool> struct sig_int2 : false_ {};
template <class T> 
struct sig_int2<T, class result_of<T(int,int)>::type> : true_ {};

template< class F >
typename enable_if<sig_int2<F>, void >::type
f (F const& x)
{
  x(1,2);
}

struct A {
  template <class> struct result {};
  template <class F> struct result<F (int)> { typedef bool type; };

  bool operator()(int) const { return true; }
};

int main()
{
  A a;
  f (a);
}
