#ifndef _HTTP_SERVER_ON_REQUEST_H_
#define _HTTP_SERVER_ON_REQUEST_H_
#include <boost/utility/enable_if.hpp>
#include <boost/utility/result_of.hpp>
#include <boost/type_traits/is_same.hpp>

#include <http_server/asio.h>
#include <http_server/trace.h>
#include <http_server/return_to_type.h>
#include <http_server/convert_callback_to_coro.h>
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
  template <class> struct result {};
  template <class F, class Iterator, class SmartSock> 
  struct result<F (http::HttpMethod, uri::parts<Iterator>, SmartSock)>
  {
  	typedef typename boost::result_of< Handler (
  	  asio::yield_context, http::HttpMethod, uri::parts<Iterator>, SmartSock
  	)>::type type;
  };

  convert_on_request_to_coro (Handler h) : handler_ (boost::move (h)) {}

  template <typename Iterator, typename SmartSock>
  typename boost::result_of< Handler (
    asio::yield_context, http::HttpMethod, uri::parts<Iterator>, SmartSock
  )>::type 
  operator() (http::HttpMethod method, uri::parts<Iterator> const& parsed,
    SmartSock sock)
  {


    typedef typename boost::result_of< Handler (
      asio::yield_context, http::HttpMethod, uri::parts<Iterator>, SmartSock
    )>::type result_type;

  	using boost::cref;
  	return convert_callback_to_coro (
  	  boost::bind<result_type> (handler_, _1, method, cref (parsed), sock)
  	) (sock->get_io_service ());
  }

private:
  Handler handler_;
};

} // namespace detail

template <typename R, typename Iterator, typename SmartSock, typename Handler>
R on_request (Handler handler, typename boost::enable_if_c<
    boost::is_same<typename boost::result_of<Handler (
            asio::yield_context, http::HttpMethod, uri::parts<Iterator>, 
            SmartSock
    )>::type, bool>::value, detail::enabler>::type = detail::enabler ())
{
	HTTP_TRACE_ENTER ();
  return detail::convert_on_request_to_coro<Handler> (boost::move (handler));
}

template <typename R, typename Iterator, typename SmartSock, typename Handler>
R on_request (Handler handler, typename boost::enable_if_c<
    boost::is_same<typename boost::result_of<Handler (
            http::HttpMethod, uri::parts<Iterator>, SmartSock
    )>::type, bool>::value, detail::enabler>::type = detail::enabler ())
{
	HTTP_TRACE_ENTER ();
  return handler;
}
} // namespace http
#endif // _HTTP_ON_REQUEST_H_
