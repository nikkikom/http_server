#ifndef _HTTP_SERVER_ON_REQUEST_COROUTINE_H_
#define _HTTP_SERVER_ON_REQUEST_COROUTINE_H_
#include <boost/utility/enable_if.hpp>
#include <boost/utility/result_of.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/decay.hpp>

#include <http_server/asio.h>
#include <http_server/trace.h>
#include <http_server/return_to_type.h>
#include <http_server/on_request.h>
#include <http_server/detail/convert_callback_to_coro.h>
#include <http_server/uri/parts.h>
#include <http_server/detail/enabler.h>

#include <boost/move/move.hpp>
#include <boost/bind.hpp>
#include <utility>

namespace http { 

using sys::error_code;

namespace detail {

template <class Endpoint, class Handler>
class convert_on_request_to_coro
{
public:
  typedef typename boost::decay<Handler>::type _Handler;

#if !defined (BOOST_RESULT_OF_USE_DECLTYPE)
  template <class> struct result {};
  template <class F, class Iterator, class SmartSock> 
  struct result<F (
      typename error_handler<Endpoint, SmartSock>::type, 
      http::HttpMethod, uri::parts<Iterator>, SmartSock
  )> {
  	typedef typename boost::result_of<_Handler (
  	  typename error_handler<Endpoint, SmartSock>::type,
  	  asio::yield_context, http::HttpMethod, uri::parts<Iterator>, SmartSock
  	)>::type type;
  };
#endif

#if __cplusplus >= 201103L
  template <typename H, class = typename boost::enable_if<
      boost::is_same<typename boost::decay<H>::type, _Handler>>::type>
  convert_on_request_to_coro (H&& h) : handler_ (std::forward<H> (h)) {}
#else
  convert_on_request_to_coro (_Handler const& h) : handler_ (h) {}
#endif

  template <class Iterator, class SmartSock>
  typename boost::result_of<_Handler (
    typename error_handler<Endpoint, SmartSock>::type,
    asio::yield_context, http::HttpMethod, uri::parts<Iterator>, SmartSock
  )>::type 
  operator() (typename error_handler<Endpoint, SmartSock>::type error_h, 
    http::HttpMethod method, uri::parts<Iterator> const& parsed,
    SmartSock sock)
  {
    typedef typename boost::result_of<_Handler (
      typename error_handler<Endpoint, SmartSock>::type,
      asio::yield_context, http::HttpMethod, uri::parts<Iterator>, SmartSock
    )>::type result_type;

  	using boost::cref;

  	// FIXME: forward error_h for C++
  	return detail::convert_callback_to_coro (
  	  boost::bind<result_type> (
  	      handler_, error_h, _1, method, cref (parsed), sock
  	  )
  	) (sock->get_io_service ());
  }

private:
  _Handler handler_;
};

} // namespace detail

#if __cplusplus >= 201103L
template <class Iterator, class Endpoint, class SmartSock, class Handler>
#if __cplusplus < 201300L
detail::convert_on_request_to_coro<Endpoint, Handler>
#else
auto
#endif
on_request (Handler&& handler, typename boost::enable_if_c<
    boost::is_same<typename boost::result_of<
      typename boost::decay<Handler>::type (
            typename error_handler<Endpoint, SmartSock>::type,
            asio::yield_context, http::HttpMethod, uri::parts<Iterator>, 
            SmartSock
    )>::type, bool>::value, detail::enabler>::type = detail::enabler ())
{
	HTTP_TRACE_ENTER ();
  return detail::convert_on_request_to_coro<Endpoint, Handler> (
      std::forward<Handler> (handler));
}
#else
template <class Iterator, class Endpoint, class SmartSock, class Handler>
detail::convert_on_request_to_coro<Endpoint, Handler>
on_request (Handler const& handler, typename boost::enable_if_c<
    boost::is_same<typename boost::result_of<Handler (
            typename error_handler<Endpoint,SmartSock>::type,
            asio::yield_context, http::HttpMethod, uri::parts<Iterator>, 
            SmartSock
    )>::type, bool>::value, detail::enabler>::type = detail::enabler ())
{
	HTTP_TRACE_ENTER ();
  return detail::convert_on_request_to_coro<Endpoint, Handler> (
      handler);
}
#endif

// #if __cplusplus < 201103L
namespace traits {

template <class Iterator, class Endpoint, class SmartSock, class Handler>
struct on_request<Iterator, Endpoint, SmartSock, Handler, 
  typename boost::enable_if<boost::is_same<typename boost::result_of<Handler (
      typename error_handler<Endpoint,SmartSock>::type,
      asio::yield_context, http::HttpMethod, uri::parts<Iterator>, SmartSock
  )>::type, bool> >::type>
{
	typedef detail::convert_on_request_to_coro<Endpoint, Handler> type;
};

} // namespace traits
// #endif // C++03

} // namespace http
#endif // _HTTP_SERVER_ON_REQUEST_COROUTINE_H_
