#ifndef _HTTP_SERVER_ON_REQUEST_URI_PARSED_H_
#define _HTTP_SERVER_ON_REQUEST_URI_PARSED_H_
#include <boost/utility/enable_if.hpp>
#include <boost/utility/result_of.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/decay.hpp>

#include <http_server/trace.h>
#include <http_server/uri/parts.h>
#include <http_server/detail/enabler.h>

namespace http { 

namespace detail {

template <typename Handler>
class on_request_uri_parsed
{
public:
  typedef typename boost::decay<Handler>::type _Handler;

#if !defined (BOOST_RESULT_OF_USE_DECLTYPE)
  template <class, class> struct result {};

  template <class F, class Iterator, class SmartSock>
  struct result<Iterator, F (SmartSock)>
  {
  	typedef typename boost::result_of<_Handler (
  	  http::HttpMethod, uri::parts<Iterator>, SmartSock
  	)>::type type;
  };
#endif

#if __cplusplus >= 201103L
  template <typename H, class = typename boost::enable_if<
      boost::is_same<typename boost::decay<H>::type, _Handler>>::type>
  on_request_uri_parsed (H&& h) : handler_ (std::forward<H> (h)) {}
#else
	on_request_uri_parsed (_Handler const& h) : handler_ (h) {}
#endif

	template <typename Iterator, typename SmartSock>
	typename boost::result_of<_Handler (
	  http::HttpMethod, uri::parts<Iterator>, SmartSock
	)>::type
	operator() (SmartSock sock)
	{
		typedef typename 
		  boost::result_of<on_request_uri_parsed (SmartSock)>::type result_type;

		// TODO: parse method and uri::parts ...
		return handler_ (method::Get, uri::parts<Iterator> (), sock);
  }

private:
  _Handler handler_;
};

}

#if __cplusplus >= 201103L
template <typename R, typename Iterator, typename SmartSock, typename Handler>
detail::on_request_uri_parsed<Handler>
on_request (Handler&& handler, typename boost::enable_if_c<
    boost::is_same<typename boost::result_of<
      typename boost::decay<Handler>::type (
            http::HttpMethod, uri::parts<Iterator>, SmartSock
    )>::type, bool>::value, detail::enabler>::type = detail::enabler ())
{
	HTTP_TRACE_ENTER ();
  return detail::on_request_uri_parsed<Handler> (
      std::forward<Handler> (handler));
}
#else
template <typename R, typename Iterator, typename SmartSock, typename Handler>
detail::on_request_uri_parsed<Handler> 
on_request (Handler const& handler, typename boost::enable_if_c<
    boost::is_same<typename boost::result_of<Handler (
            http::HttpMethod, uri::parts<Iterator>, SmartSock
    )>::type, bool>::value, detail::enabler>::type = detail::enabler ())
{
	HTTP_TRACE_ENTER ();
  return detail::on_request_uri_parsed<Handler> (handler);
}
#endif
} // namespace http
#endif // _HTTP_SERVER_ON_REQUEST_URI_PARSED_H_
