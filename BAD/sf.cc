#include <type_traits>
#include <utility>

using namespace std;

template <class Action, typename Stop, typename Arg, typename = void>
struct repeat_until_helper
{
	typedef result_of_t<Action (Arg)> action_result;
  auto operator() (Action action, Arg arg) const
  {
  	return repeat_until_helper<Action, Stop, action_result> {}
  	  (action, action (arg))
  	;
  }
};

template <class Action, typename Stop, typename Arg>
struct repeat_until_helper<Action, Stop, Arg,
  enable_if_t<
    is_same<result_of_t<Arg (Stop)>, bool>::value
  >
>
{
	Arg operator() (Action, Arg arg) const { return arg; }
};

template <typename Stop, class Action, typename Arg>
auto
repeat_until (Action action, Arg arg)
{
	return repeat_until_helper<Action, Stop, Arg> {} (action, arg);
}

struct A {}; struct B {}; struct C {};

template <typename H> auto foo (H h, 
  enable_if_t<is_same<result_of_t<H(A)>, bool>::value>* = nullptr)
{ return [h] (B) { return h (A {}); }; }

template <typename H> auto foo (H h, 
  enable_if_t<is_same<result_of_t<H(B)>, bool>::value>* = nullptr)
{ return [h] (C) { return h (B {}); }; }

struct foo_functor
{
	template <typename T>
	auto operator() (T t) const { return foo (t); }
};

struct InnerHandler
{
	template <typename T>
	bool operator() (T) { return true; }
};

int main ()
{
	auto inner_handler = [] (auto) { return true; };
#if 0 
	// OK
	auto f = repeat_until<C> (foo_functor {}, inner_handler);
#else
	// Error - but why???
	auto f = repeat_until_helper<foo_functor, C, decltype (inner_handler)> {}
	  (foo_functor (), inner_handler);
#endif
	f (C {});
}
