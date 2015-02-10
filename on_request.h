#ifndef _HTTP_SERVER_ON_REQUEST_H_
#define _HTTP_SERVER_ON_REQUEST_H_


#if __cplusplus < 201103L
# include <boost/static_assert.hpp>
#endif

namespace http {

namespace detail { struct final_call_tag {}; }

namespace traits {

struct bad_type;

// #if __cplusplus < 201103L
template <class R, class Iterator, class Sock, class Handler, 
  class Enabler = void> 
struct on_request 
{
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

template <typename R, class Iterator, class Sock>
struct on_request_functor
{
#if __cplusplus < 201103L
	template <class> struct result {};
	template <class F, class H> struct result<F(H)>
	{
		typedef typename traits::on_request<R,Iterator,Sock,H>::type type;

    BOOST_STATIC_ASSERT_MSG ((!boost::is_same<type, traits::bad_type>::value),
      "Cannot find 'on_request' handler with such signature");

  };
#endif

  template <class H>
  typename traits::on_request<R, Iterator, Sock, H>::type
  operator() (H h) const
  {
  	return on_request<R, Iterator, Sock> (h);
  }
};

} // namespace detail
} // namespace http

#endif // _HTTP_SERVER_ON_REQUEST_H_
