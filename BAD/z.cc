#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/utility/result_of.hpp>
#include <boost/static_assert.hpp>

using namespace boost;
struct A {};
struct B {};

template <class F, class=void> struct decl
{
	typedef typename result_of<F(A)>::type type;
	BOOST_STATIC_ASSERT_MSG(is_same<type, bool>::value, 
	    "Return type must be bool");
};

template <class F>
struct decl<F, typename enable_if<
    is_same<typename result_of<F(A)>::type, bool> 
  >::type
>
{
	typedef int type;
};


struct X
{
	void operator() (A c) const { return ; }
};

int main ()
{
	typename decl<X>::type xx;
	// using Y = bool(char);
	// typename decl<A>::type x;
}
