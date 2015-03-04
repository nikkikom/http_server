#ifndef _HTTP_SERVER_ON_REQUEST_URI_PARSED_H_
#define _HTTP_SERVER_ON_REQUEST_URI_PARSED_H_
#include <boost/utility/enable_if.hpp>
#include <boost/utility/result_of.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/decay.hpp>

#include <http_server/trace.h>
#include <http_server/on_request.h>
#include <http_server/error_handler.h>
#include <http_server/uri/parts.h>
#include <http_server/detail/enabler.h>

namespace http { 

namespace detail {

template <class Iterator, class Handler>
class on_request_uri_parsed
{
public:
  typedef typename boost::decay<Handler>::type _Handler;

#if !defined (BOOST_RESULT_OF_USE_DECLTYPE)
  template <class> struct result {};

  template <class F, class Error, class SmartSock>
  struct result<F (
      Error, SmartSock, detail::final_call_tag
  )> {
  	typedef typename boost::result_of<_Handler (
  	  Error, http::HttpMethod, uri::parts<Iterator>, SmartSock
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

  // FIXME: move error handler on c++11
	template <class Error, class SmartSock>
	typename boost::result_of<_Handler (
	  Error, http::HttpMethod, uri::parts<Iterator>, SmartSock
	)>::type
	operator() (Error error_h, SmartSock sock, detail::final_call_tag)
	{
		typedef typename 
		  boost::result_of<on_request_uri_parsed (
		      Error, SmartSock, detail::final_call_tag)>::type result_type;

		// TODO: parse method and uri::parts here ...

		return handler_ (error_h, method::Get, uri::parts<Iterator> (), sock);
  }

private:
  _Handler handler_;
};

}

#if __cplusplus >= 201103L
template <class Error, class Iterator, class SmartSock, class Handler>
detail::on_request_uri_parsed<Iterator, Handler>
on_request (Handler&& handler, typename boost::enable_if_c<
    boost::is_same<typename boost::result_of<
      typename boost::decay<Handler>::type (
            Error, http::HttpMethod, uri::parts<Iterator>, SmartSock
    )>::type, error_code>::value, detail::enabler>::type = detail::enabler ())
{
	HTTP_TRACE_ENTER ();
  return detail::on_request_uri_parsed<Iterator, Handler> (
      std::forward<Handler> (handler));
}
#else
template <class Error, class Iterator, class SmartSock, class Handler>
detail::on_request_uri_parsed<Iterator, Handler> 
on_request (Handler const& handler, typename boost::enable_if_c<
    boost::is_same<typename boost::result_of<Handler (
            Error, http::HttpMethod, uri::parts<Iterator>, SmartSock
    )>::type, error_code>::value, detail::enabler>::type = detail::enabler ())
{
	HTTP_TRACE_ENTER ();
  return detail::on_request_uri_parsed<Iterator, Handler> (handler);
}
#endif

// #if __cplusplus < 201103L
namespace traits {

template <class Error, class Iterator, class SmartSock, class Handler>
struct on_request<Error, Iterator, SmartSock, Handler, 
  typename boost::enable_if<boost::is_same<typename boost::result_of<Handler (
    Error, http::HttpMethod, uri::parts<Iterator>, SmartSock
  )>::type, error_code> >::type>
{
	typedef detail::on_request_uri_parsed<Iterator, Handler> type;
};

} // namespace traits
// #endif // C++03


} // namespace http
#endif // _HTTP_SERVER_ON_REQUEST_URI_PARSED_H_
