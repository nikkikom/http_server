#ifndef _HTTP_SERVER_ON_REQUEST_COROUTINE_H_
#define _HTTP_SERVER_ON_REQUEST_COROUTINE_H_
#include <boost/utility/enable_if.hpp>
#include <boost/utility/result_of.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/decay.hpp>

#include <http_server/asio.h>
#include <http_server/trace.h>
#include <http_server/return_to_type.h>
#include <http_server/detail/convert_callback_to_coro.h>
#include <http_server/uri/parts.h>
#include <http_server/detail/enabler.h>

#include <boost/move/move.hpp>
#include <boost/bind.hpp>
#include <utility>

namespace http { 

using sys::error_code;

namespace detail {

template <typename Handler>
class convert_on_request_to_coro
{
public:
  typedef typename boost::decay<Handler>::type _Handler;

#if !defined (BOOST_RESULT_OF_USE_DECLTYPE)
  template <class> struct result {};
  template <class F, class Iterator, class SmartSock> 
  struct result<F (http::HttpMethod, uri::parts<Iterator>, SmartSock)>
  {
  	typedef typename boost::result_of<_Handler (
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

  template <typename Iterator, typename SmartSock>
  typename boost::result_of<_Handler (
    asio::yield_context, http::HttpMethod, uri::parts<Iterator>, SmartSock
  )>::type 
  operator() (http::HttpMethod method, uri::parts<Iterator> const& parsed,
    SmartSock sock)
  {
    typedef typename boost::result_of<_Handler (
      asio::yield_context, http::HttpMethod, uri::parts<Iterator>, SmartSock
    )>::type result_type;

  	using boost::cref;
  	return detail::convert_callback_to_coro (
  	  boost::bind<result_type> (handler_, _1, method, cref (parsed), sock)
  	) (sock->get_io_service ());
  }

private:
  _Handler handler_;
};

} // namespace detail

#if __cplusplus >= 201103L
template <typename R, typename Iterator, typename SmartSock, typename Handler>
R on_request (Handler&& handler, typename boost::enable_if_c<
    boost::is_same<typename boost::result_of<
      typename boost::decay<Handler>::type (
            asio::yield_context, http::HttpMethod, uri::parts<Iterator>, 
            SmartSock
    )>::type, bool>::value, detail::enabler>::type = detail::enabler ())
{
	HTTP_TRACE_ENTER ();
	typedef typename boost::decay<Handler>::type _Handler;
  return detail::convert_on_request_to_coro<_Handler> (
      std::forward<Handler> (handler));
}
#else
template <typename R, typename Iterator, typename SmartSock, typename Handler>
R on_request (Handler const& handler, typename boost::enable_if_c<
    boost::is_same<typename boost::result_of<Handler (
            asio::yield_context, http::HttpMethod, uri::parts<Iterator>, 
            SmartSock
    )>::type, bool>::value, detail::enabler>::type = detail::enabler ())
{
	HTTP_TRACE_ENTER ();
  return detail::convert_on_request_to_coro<Handler> (handler);
}
#endif
} // namespace http
#endif // _HTTP_SERVER_ON_REQUEST_COROUTINE_H_
