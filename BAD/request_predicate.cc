#include <http_server/request_predicate.h>
#include <iostream>

template <typename I>
struct A {
  template <typename P>
	void foo (P p, typename boost::enable_if<
	    http::is_request_predicate<P,I> >::type* = 0)
	{
    
  }
};

struct check
{
#if !defined(BOOST_RESULT_OF_USE_DECLTYPE)
	typedef bool result_type;
#endif

	template <typename I>
	bool operator() (http::method m, http::uri::parts<I> p) const
	{
		return true;
  }
};

int main ()
{
	A<char*> a;
	a.foo (check ());
}
