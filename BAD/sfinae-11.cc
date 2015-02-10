
#include <iostream>
// #include <type_traits>
#include <string>

#include <boost/utility/result_of.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>
#include <boost/typeof/typeof.hpp>

// using namespace std;

struct A {};
struct B {};
struct C {};
struct D {};
struct E {};


template <typename H> 
auto foo (H h, typename boost::enable_if<
  boost::is_same<typename boost::result_of<H(A)>::type, bool> >::type* = 0)
{
	return [h] (B x) { return h (A {}); };
}

template <typename H> 
auto foo (H h, typename boost::enable_if<
  boost::is_same<typename boost::result_of<H(B)>::type, bool> >::type* = 0)
{
	return [h] (C x) { return h (B {}); };
}

template <typename H> 
auto foo (H h, typename boost::enable_if<
  boost::is_same<typename boost::result_of<H(C)>::type, bool> >::type* = 0)
{
	return [h] (D x) { return h (C {}); };
}

template <typename H> 
auto foo (H h, typename boost::enable_if<
  boost::is_same<typename boost::result_of<H(D)>::type, bool> >::type* = 0)
{
	return [h] (E x) { return h (D {}); };
}


struct foo_functor 
{
	template <typename T>
	// BOOST_TYPEOF_TPL (foo (T()))
	auto
	operator() (T t) const { return foo (t); }
};

template <class Action, class Stop, class Arg, class=void>
struct repeat_until_helper
{
	typedef typename boost::result_of<Action (Arg)>::type action_result;

  template <class> struct result {};
  template <class F> struct result<F(Action,Arg)> 
  { 
  	typedef repeat_until_helper <Action, Stop, action_result> type;
  };
    
	// repeat_until_helper <Action, Stop, action_result>
	typename boost::result_of<
	  repeat_until_helper <Action, Stop, action_result> (
	      Action, action_result)
	>::type operator() (Action action, Arg arg) const
	{
		return repeat_until_helper <Action, Stop, action_result> {} (
		  action, action (arg)
		);
  }
};

template <typename Action, class Stop, class Arg>
struct repeat_until_helper<Action, Stop, Arg, 
  typename boost::enable_if<
    boost::is_same<typename boost::result_of<Arg (Stop)>::type, bool> 
  >::type>
{
  template <class> struct result {};
  template <class F> struct result<F(Action,Arg)> 
  { 
  	typedef Arg type;
  };
 
	Arg 
	operator() (Action action, Arg arg) const
	{
		return arg;
  }
};

template <class Stop, class Action, class Arg>
auto
repeat_until (Action action, Arg arg)
{
	return repeat_until_helper<Action, Stop, Arg> {} (action, arg);
}


int main ()
{
	auto inner_handler = [] (A) -> bool { return false; };

	// auto f = foo (foo (foo (foo ( inner_handler ))));
	auto f = repeat_until<E> ( foo_functor {}, inner_handler );

	std::cout << f ( E{} )  << "\n";
}
