#define BOOST_RESULT_OF_USE_TR1 1
#include <boost/utility/result_of.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>
#include <iostream>

using namespace boost;

template <class H>
struct foo_1
{
  H h; foo_1 (H h) : h(h) {}
	bool operator() (int i) { return h('0' + i); } 
};

template <typename H>
foo_1<H> 
foo (H h, 
  typename enable_if<
      is_same<
          typename result_of<H(char)>::type
        , bool
      >
    >::type* = 0) 
{ return foo_1<H> (h); }

template <class, class = void> struct foo_decl { typedef void type; };

template <class H>
struct foo_decl<H, 
  typename enable_if<is_same<typename result_of<H(char)>::type, bool> >
::type>
{
	typedef foo_1<H> type;
};

struct foo_functor
{
	template <class> struct result {};
	template <class F, class H> struct result<F(H)>
	{
		typedef typename foo_decl<H>::type type;
  };

  template <class H>
  // typename foo_decl<H>::type
  auto
  operator() (H h) const { return foo (h); }
};

struct MyH 
{
	typedef bool result_type;
	bool operator() (char b) const { return b == '4'; }
};

int main ()
{
//	typename boost::result_of<foo_functor(MyH)>::type r = 
  typename foo_decl<MyH>::type r =
	  foo_functor() (MyH ());

	std::cout << "i=" << r (4) << "\n";
}
