#include <boost/mpl/bool.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/utility/result_of.hpp>

#include <iostream>

using ::boost::result_of;
using ::boost::enable_if;
using ::boost::mpl::true_;
using ::boost::mpl::false_;

using ::std::cout;

template <typename T> struct is_bool : false_ {};
template <> struct is_bool<bool> : true_ {};

template <typename T, typename ResultOf = void> struct is_sig_int: false_ {};

template <typename T>
struct is_sig_int<T, 
  typename enable_if<
    is_bool<
      typename result_of<T (int)>::type
    >
  >::type
>: true_ {};

template <typename T, typename ResultOf = void> struct is_sig_int2: false_ {};

template <typename T>
struct is_sig_int2<T, 
  typename enable_if<is_bool<typename result_of<T (int,int)>::type> >::type>
: true_ {};

template <typename T>
typename enable_if<
    is_bool<typename result_of<T (int)>::type>
  , bool>::type
foo (T t)
{
	cout << "foo (T(int))\n";
	return t (1);
}

template <typename T>
typename enable_if<
    is_bool<typename result_of<T (int,int)>::type>
  , bool>::type
foo (T t)
{
	cout << "foo (T(int,int))\n";
	return t (2,3);
}

struct A { 
#if 1
	typedef bool result_type;
#else
	template <class> struct result;
  template <class F> struct result<F(int)> { typedef bool type; };
#endif
	bool operator() (int) const { return true; }
};

int main ()
{
	foo (A ());
}
