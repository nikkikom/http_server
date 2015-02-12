#ifndef _HTTP_SERVER_ON_REQUEST_CALLBACK_H_
#define _HTTP_SERVER_ON_REQUEST_CALLBACK_H_
#include <boost/utility/enable_if.hpp>
#include <boost/utility/result_of.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/decay.hpp>
#include <boost/logic/tribool.hpp>

#include <http_server/trace.h>
#include <http_server/on_request.h>
#include <http_server/uri/parts.h>
#include <http_server/detail/enabler.h>

namespace http { 

namespace detail {
template <typename Handler>
class on_request_callback_helper
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

};

#if __cplusplus >= 201103L
template <typename Iterator, typename SmartSock, typename Handler>
auto 
on_request (Handler&& handler, typename boost::enable_if_c<
    boost::is_same<typename boost::result_of<
      typename boost::decay<Handler>::type (
            http::HttpMethod, uri::parts<Iterator>, SmartSock
    )>::type, bool>::value, detail::enabler>::type = detail::enabler ())
#if __cplusplus < 201300L
      -> decltype (handler)
#endif
{
	HTTP_TRACE_ENTER ();
  return handler;
}
#else
template <typename Iterator, typename SmartSock, typename Handler>
Handler
on_request (Handler const& handler, typename boost::enable_if_c<
    boost::is_same<typename boost::result_of<Handler (
            http::HttpMethod, uri::parts<Iterator>, SmartSock
    )>::type, bool>::value, detail::enabler>::type = detail::enabler ())
{
	HTTP_TRACE_ENTER ();
  return handler;
}
#endif
} // namespace http
#endif // _HTTP_SERVER_ON_REQUEST_CALLBACK_H_
