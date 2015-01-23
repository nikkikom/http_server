#include <http_server/traits/handler_type.h>
#include <iostream>

#include <boost/type_traits/function_traits.hpp>

namespace traits = http::traits;

class A 
{
	struct enabler {};

public:
	template <typename H>
	void foo (H&& handler, typename std::enable_if<
	    traits::is_handler_stream<H>::value 
	  , enabler*>::type = 0)
	{
		foo_stream (std::forward<H> (handler));
  }

	template <typename H>
	void foo (H&& handler, typename std::enable_if<
	    traits::is_handler_asio_buffer<H>::value 
	  , enabler*>::type = 0)
	{
		foo_asio_buffer (std::forward<H> (handler));
  }

	template <typename H>
	void foo (H&& handler, typename std::enable_if<
	    traits::detail::is_handler_other_signature<H>::value
	  , enabler*>::type = 0)
	{
		foo_default (std::forward<H> (handler));
  }

protected:
  template <typename H>
  void foo_stream (H h)
  {
  	std::cout << __PRETTY_FUNCTION__ << "\n";
  	h (1);
  	std::cout << "--------------------------------------------\n";
  }

  template <typename H>
  void foo_asio_buffer (H h)
  {
  	std::cout << __PRETTY_FUNCTION__ << "\n";
  	h ("ccc");
  	std::cout << "--------------------------------------------\n";
  }

  template <typename H>
  void foo_default (H h)
  {
  	std::cout << __PRETTY_FUNCTION__ << "\n";
  	h ();
  	std::cout << "--------------------------------------------\n";
  }
};


int main ()
{
	A a;

#if 1
	a.foo (//traits::handler_stream (
	  [] (int i) { std::cout << "L1 (" << i << ")\n"; }
	);
#endif

#if 1
	a.foo (// traits::handler_asio_buffer (
	  [] (char const* s) { std::cout << "L2 (" << s << ")\n"; }
	);
#endif

#if 1
	a.foo (
	  [] () { std::cout << "L3 ()\n"; }
	);
#endif

}
