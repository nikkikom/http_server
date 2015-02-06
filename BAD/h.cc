#include <iostream>
#include <utility>
#include <boost/move/move.hpp>
#include <boost/move/utility_core.hpp>

#if __cplusplus >= 201100L
#include <type_traits>
using std::decay;
using std::forward;
using std::enable_if;
using std::is_same;
#endif
using std::cout;
using boost::move;

struct handler 
{
	handler () { cout << "handler::handler ()\n"; }
	handler (handler const&) { cout << "handler::CCTOR\n"; }
	handler& operator= (handler const&) { cout << "handler::ASSIGN\n"; return *this; }
#if __cplusplus >= 201100L
	handler (handler&&) { cout << "handler::MCTOR\n"; }
	handler& operator= (handler&&) { cout << "handler::MASSIGN\n"; return *this; }
#endif

	void operator() () { cout << "handler::CALL\n"; }
	void operator() () const { cout << "handler::CALL (const)\n"; }
};

class handler2
{
private:
	BOOST_COPYABLE_AND_MOVABLE(handler2)

public:
	handler2 () { cout << "handler::handler ()\n"; }
	handler2 (handler2 const&) { cout << "handler::CCTOR\n"; }
	handler2& operator= (BOOST_COPY_ASSIGN_REF(handler2)) { cout << "handler::ASSIGN\n"; return *this; }
	handler2 (BOOST_RV_REF(handler2)) { cout << "handler::MCTOR\n"; }
	handler2& operator= (BOOST_RV_REF(handler2)) { cout << "handler::MASSIGN\n"; return *this; }

	void operator() () { cout << "handler::CALL\n"; }
	void operator() () const { cout << "handler::CALL (const)\n"; }
};

template <class H>
struct wrapper
{

#if __cplusplus >= 201100L
  typedef typename decay<H>::type HH;

	template <class X, 
	class = typename enable_if<is_same<typename decay<X>::type,HH>::value>::type> 
	wrapper (X&& h) : h_ (forward<X> (h)) {}
#else
	typedef H HH;

	wrapper (HH const& h) : h_ (h) {}
#endif

	void operator() () { h_ (); }
	HH h_;
};

#if __cplusplus >= 201100L
template <typename H>
wrapper<typename decay<H>::type>
wrap (H&& h)
{
	return wrapper<typename decay<H>::type> (forward<H> (h));
}
#else
template <typename H>
wrapper<H>
wrap (H const& h)
{
	return wrapper<H> (h);
}
#endif

template <class H>
struct wrapper2
{

#if __cplusplus >= 201100L
  typedef typename decay<H>::type HH;

	template <class X, 
	class = typename enable_if<is_same<typename decay<X>::type,HH>::value>::type> 
	wrapper2 (X&& h) : h_ (wrap (forward<X> (h))) {}
#else
	typedef H HH;

	wrapper2 (HH const& h) : h_ (wrap (h)) {}
#endif

	void operator() () { h_ (); }
	wrapper<HH> h_;
};

#if __cplusplus >= 201100L
template <typename H>
wrapper2<typename decay<H>::type>
wrap2 (H&& h)
{
	return wrapper2<typename decay<H>::type> (forward<H> (h));
}
#else
template <typename H>
wrapper2<H>
wrap2 (H const& h)
{
	return wrapper2<H> (h);
}
#endif

#if __cplusplus >= 201100L
template <typename H>
wrapper<typename decay<H>::type>
foo (H&& h)
{
	return wrap (forward<H> (h));
}
#else
template <typename H>
wrapper<H>
foo (H const& h)
{
	return wrap<H> (h);
}
#endif

#if __cplusplus >= 201100L
template <typename H>
wrapper2<typename decay<H>::type>
foo2 (H&& h)
{
	return wrap2 (forward<H> (h));
}
#else
template <typename H>
wrapper2<H>
foo2 (H const& h)
{
	return wrap2<H> (h);
}
#endif

int main ()
{
	{
    typedef handler2 H;
    { wrapper<H> w = wrap (H ()); w (); } cout << "------------\n";
    { wrapper<H> w = foo (H ()); w (); } cout << "------------\n";
    { H h; wrapper<H> w = foo (h); w (); } cout << "------------\n";
  }
  cout << "==============================\n";
	{
    typedef handler H;
    { wrapper2<H> w = wrap2 (H ()); w (); } cout << "------------\n";
    { wrapper2<H> w = foo2 (H ()); w (); } cout << "------------\n";
    { H h; wrapper2<H> w = foo2 (h); w (); } cout << "------------\n";
  }
}
