#ifndef _HTTP_SERVER_ON_REQUEST_H_
#define _HTTP_SERVER_ON_REQUEST_H_

#if !defined (BOOST_RESULT_OF_USE_DECLTYPE)
# include <boost/type_traits/is_same.hpp>
# include <boost/static_assert.hpp>
#endif

#include <yplatform/url.hpp>
#include <yplatform/http/headers.hpp>

namespace http {

using ::yplatform::url;
using ::yplatform::http::headers;

namespace detail { struct final_call_tag {}; }

namespace traits {

struct bad_type {};

// #if __cplusplus < 201103L
template <class Error, class Iterator, class Sock, 
  class Handler, class Enabler = void> 
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

template <class Error, class Iterator, class Sock>
struct on_request_functor
{
#if !defined (BOOST_RESULT_OF_USE_DECLTYPE)
	template <class> struct result {};
	template <class F, class H> struct result<F(H)>
	{
		typedef typename traits::on_request<Error,Iterator,Sock,H>::type type;

    BOOST_STATIC_ASSERT_MSG ((!boost::is_same<type, traits::bad_type>::value),
      "Cannot find 'on_request' handler with such signature");

  };
#endif

#if __cplusplus >= 201103L
  template <class H>
  auto operator() (H&& h) const 
#if __cplusplus < 201300L
    -> decltype (on_request<Error, Iterator, Sock> (std::forward<H> (h)))
#endif
  {
  	return on_request<Error, Iterator, Sock> (std::forward<H> (h));
  }
#else
  template <class H>
  typename traits::on_request<Error, Iterator, Sock, H>::type
  operator() (H const& h) const
  {
  	return on_request<Error, Iterator, Sock> (h);
  }
#endif
};

} // namespace detail
} // namespace http

#endif // _HTTP_SERVER_ON_REQUEST_H_
