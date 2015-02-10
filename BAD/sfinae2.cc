#define BOOST_RESULT_OF_USE_TR1 1

#include <boost/utility/result_of.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>
#include <boost/typeof/typeof.hpp>

#include <http_server/detail/repeat_until.h>

using namespace boost;

struct A {};
struct B {};
struct C {};
struct D {};
struct E {};

template <class H> class foo_B_A {
	H handler_;
public:
  template <class> struct result {};
  template <class F> struct result<F(B)> { typedef bool type; };

  foo_B_A (H h) : handler_ (h) {}
  bool operator() (B) const { return handler_ (A ()); }
};

template <class H> class foo_C_B {
	H handler_;
public:
  template <class> struct result {};
  template <class F> struct result<F(C)> { typedef bool type; };
  foo_C_B (H h) : handler_ (h) {}
  bool operator() (C) const { return handler_ (B ()); }
};

template <typename H> 
foo_B_A<H>
foo (H h, typename boost::enable_if<
  boost::is_same<typename boost::result_of<H(A)>::type, bool> >::type* = 0)
{
	return foo_B_A<H> (h);
}

template <typename H> 
foo_C_B<H>
foo (H h, typename boost::enable_if<
  boost::is_same<typename boost::result_of<H(B)>::type, bool> >::type* = 0)
{
	return foo_C_B<H> (h);
}

struct bad_type {};
template <class, class=void> struct foo_return_traits { typedef bad_type type; };

template <class F> 
struct foo_return_traits<F, typename boost::enable_if<
  boost::is_same<typename boost::result_of<F(A)>::type, bool> >::type>
{
	typedef foo_B_A<F> type;
};

template <class F> 
struct foo_return_traits<F, typename boost::enable_if<
  boost::is_same<typename boost::result_of<F(B)>::type, bool> >::type>
{
	typedef foo_C_B<F> type;
};

struct foo_functor 
{
	template <class> struct result {};
	template <class F, class H> struct result<F(H)>
	{ 
		typedef typename foo_return_traits<H>::type type;
		static_assert(!boost::is_same<type, bad_type>::value, 
		  "Cannot find handler with such signature");
	};

	template <typename H>
	// auto
	typename foo_return_traits<H>::type
	operator() (H h) const // -> decltype (foo (h))
	{ return foo (h); }
};


struct InnerHandler 
{
	template <class> struct result {};
	template <class F> struct result<F(A)> { typedef bool type; };
	bool operator() (A) const { return false; }
};

int main ()
{
	using namespace http::detail;
	InnerHandler inner_handler = InnerHandler ();
	// auto f = foo (foo (foo (foo ( inner_handler ))));
	repeat_until<bool (E)> ( foo_functor (), inner_handler ) ( C () );
}
