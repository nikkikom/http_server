#include <iostream>
#include <type_traits>
#include <string>
using namespace std;

struct A {};
struct B {};
struct C {};
struct D {};
struct E {};

template <typename H>
auto foo (H h, enable_if_t<
  is_same<typename result_of<H(A)>::type, bool>::value>* = 0)
{
	return [h] (B x) { return h (A {}); };
}

template <typename H>
auto foo (H h, enable_if_t<
  is_same<typename result_of<H(B)>::type, bool>::value>* = 0)
{
	return [h] (C x) { return h (B {}); };
}

template <typename H>
auto foo (H h, enable_if_t<
  is_same<typename result_of<H(C)>::type, bool>::value>* = 0)
{
	return [h] (D x) { return h (C {}); };
}

template <typename H>
auto foo (H h, enable_if_t<
  is_same<typename result_of<H(D)>::type, bool>::value>* = 0)
{
	return [h] (E x) { return h (D {}); };
}

    struct foo_functor {
      template<class Arg>
      auto operator()(Arg arg)const{
        return foo(arg);
      }
    };

    template<class Action, class Stop, class Arg, class=void>
    struct repeat_until_helper {
      using action_result = result_of_t< Action(Arg) >;
      auto operator()(Action action, Arg arg)const {
        return repeat_until_helper<Action, Stop, action_result>{}(
          action, action(arg)
        );
      }
    };

	    template<class Action, class Stop, class Arg>
    struct repeat_until_helper< Action, Stop, Arg,
      enable_if_t<is_same<result_of_t<Arg (Stop)>, bool>::value>
    > {
      auto operator()(Action action, Arg arg)const {
        return arg;
      }
    };


	template<class Stop, class Action, class Arg>
	auto repeat_until( Action action, Arg arg) {
		return repeat_until_helper< Action, Stop, Arg >{}(action, arg );
	}

int main ()
{
	auto inner_handler = [] (A) -> bool { return false; };

	auto f = repeat_until< D >( foo_functor{}, inner_handler );

	std::cout << f (D {}) << "\n";
}
