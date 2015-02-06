#ifndef _HTTP_SERVER_ON_REQUEST_CALLBACK_H_
#define _HTTP_SERVER_ON_REQUEST_CALLBACK_H_
#include <boost/utility/enable_if.hpp>
#include <boost/utility/result_of.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/decay.hpp>

#include <http_server/trace.h>
#include <http_server/uri/parts.h>
#include <http_server/detail/enabler.h>

namespace http { 

#if __cplusplus >= 201103L
template <typename R, typename Iterator, typename SmartSock, typename Handler>
R on_request (Handler&& handler, typename boost::enable_if_c<
    boost::is_same<typename boost::result_of<
      typename boost::decay<Handler>::type (
            http::HttpMethod, uri::parts<Iterator>, SmartSock
    )>::type, bool>::value, detail::enabler>::type = detail::enabler ())
{
	HTTP_TRACE_ENTER ();
  return handler;
}
#else
template <typename R, typename Iterator, typename SmartSock, typename Handler>
R on_request (Handler const& handler, typename boost::enable_if_c<
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
