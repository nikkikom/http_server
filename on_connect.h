#ifndef _HTTP_SERVER_ON_CONNECT_H_
#define _HTTP_SERVER_ON_CONNECT_H_
#include <boost/utility/enable_if.hpp>
#include <boost/utility/result_of.hpp>
#include <boost/type_traits/is_same.hpp>

#include <http_server/asio.h>
#include <http_server/trace.h>
#include <http_server/return_to_type.h>
#include <http_server/convert_callback_to_coro.h>
#include <http_server/detail/enabler.h>

#include <utility>

namespace http { 

using sys::error_code;

#if __cplusplus >= 201100L
template <typename R, typename Endpoint, typename SmartSock, typename Handler>
R on_connect (Handler&& handler, typename boost::enable_if_c<
       boost::is_void<typename boost::result_of<Handler (
            asio::yield_context, error_code, Endpoint, Endpoint, SmartSock
       )>::type>::value
    || boost::is_same<typename boost::result_of<Handler (
            asio::yield_context, error_code, Endpoint, Endpoint, SmartSock
       )>::type, bool>::value
    || boost::is_same<typename boost::result_of<Handler (
            asio::yield_context, error_code, Endpoint, Endpoint, SmartSock
       )>::type, error_code>::value
    , detail::enabler>::type = detail::enabler ())
{
	HTTP_TRACE_ENTER ();

	return convert_on_connect_to_coro (
    return_to_type (std::forward<Handler> (handler), 
	    make_error_code (sys::errc::io_error)
    )
  );
}

template <typename R, typename Endpoint, typename SmartSock, typename Handler>
R on_connect (Handler&& handler, typename boost::enable_if_c<
       boost::is_void<typename boost::result_of<Handler (
            error_code, Endpoint, Endpoint, SmartSock
       )>::type>::value
    || boost::is_same<typename boost::result_of<Handler (
            error_code, Endpoint, Endpoint, SmartSock
       )>::type, bool>::value
    || boost::is_same<typename boost::result_of<Handler (
            error_code, Endpoint, Endpoint, SmartSock
       )>::type, error_code>::value
    , detail::enabler>::type = detail::enabler ())
{
	HTTP_TRACE_ENTER ();

	return return_to_type (std::forward<Handler> (handler), 
	  make_error_code (sys::errc::io_error)
  );
}
#else
template <typename R, typename Endpoint, typename SmartSock, typename Handler>
R on_connect (Handler const& handler, typename boost::enable_if_c<
       boost::is_void<typename boost::result_of<Handler (
            asio::yield_context, error_code, Endpoint, Endpoint, SmartSock
       )>::type>::value
    || boost::is_same<typename boost::result_of<Handler (
            asio::yield_context, error_code, Endpoint, Endpoint, SmartSock
       )>::type, bool>::value
    || boost::is_same<typename boost::result_of<Handler (
            asio::yield_context, error_code, Endpoint, Endpoint, SmartSock
       )>::type, error_code>::value
    , detail::enabler>::type = detail::enabler ())
{
	HTTP_TRACE_ENTER ();

	return convert_on_connect_to_coro (
    return_to_type (handler, make_error_code (sys::errc::io_error))
  );
}

template <typename R, typename Endpoint, typename SmartSock, typename Handler>
R on_connect (Handler const& handler, typename boost::enable_if_c<
       boost::is_void<typename boost::result_of<Handler (
            error_code, Endpoint, Endpoint, SmartSock
       )>::type>::value
    || boost::is_same<typename boost::result_of<Handler (
            error_code, Endpoint, Endpoint, SmartSock
       )>::type, bool>::value
    || boost::is_same<typename boost::result_of<Handler (
            error_code, Endpoint, Endpoint, SmartSock
       )>::type, error_code>::value
    , detail::enabler>::type = detail::enabler ())
{
	HTTP_TRACE_ENTER ();

	return return_to_type (handler, make_error_code (sys::errc::io_error));
}
#endif

} // namespace http
#endif // _HTTP_ON_CONNECT_H_
