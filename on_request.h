#ifndef _HTTP_SERVER_ON_REQUEST_H_
#define _HTTP_SERVER_ON_REQUEST_H_

#include <boost/static_assert.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/utility/result_of.hpp>

namespace http {

namespace detail { struct final_call_tag {}; }

namespace traits {

struct bad_type {};

// #if __cplusplus < 201103L
template <class ResultF, class Iterator, class Sock, class Handler, 
  class Enabler = void> 
struct on_request 
{
#if 1
	BOOST_STATIC_ASSERT_MSG ((boost::is_same<
	    typename boost::result_of<ResultF(bool)>::type, void>::value),
	  "Result Functor should have 'void (bool)' signature"
	);
#endif

	typedef bad_type type;
};
// #endif
} // namespace traits
} // namespace http


// #include <http_server/on_request/callback.h>
#include <http_server/on_request/uri_parsed.h>
#include <http_server/on_request/coroutine.h>

namespace http {
namespace detail {

template <class ResultF, class Iterator, class Sock>
struct on_request_functor
{
	BOOST_STATIC_ASSERT_MSG ((boost::is_same<
	    typename boost::result_of<ResultF(bool)>::type, void>::value),
	  "Result Functor should have 'void (bool)' signature"
	);

#if !defined (BOOST_RESULT_OF_USE_DECLTYPE)
	template <class> struct result {};
	template <class F, class H> struct result<F(H)>
	{
		typedef typename traits::on_request<ResultF,Iterator,Sock,H>::type type;

    BOOST_STATIC_ASSERT_MSG ((!boost::is_same<type, traits::bad_type>::value),
      "Cannot find 'on_request' handler with such signature");

  };
#endif

#if __cplusplus >= 201103L
  template <class H>
  auto operator() (H&& h) const 
#if __cplusplus < 201300L
    -> decltype (on_request<ResultF, Iterator, Sock> (std::forward<H> (h)))
#endif
  {
  	return on_request<ResultF, Iterator, Sock> (std::forward<H> (h));
  }
#else
  template <class H>
  typename traits::on_request<ResultF, Iterator, Sock, H>::type
  operator() (H const& h) const
  {
  	return on_request<ResultF, Iterator, Sock> (h);
  }
#endif
};

} // namespace detail
} // namespace http

#endif // _HTTP_SERVER_ON_REQUEST_H_
