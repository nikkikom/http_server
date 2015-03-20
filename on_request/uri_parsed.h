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
#include <http_server/detail/is_yield_context.h>

#include <yplatform/url.hpp>
#include <yplatform/http/headers.hpp>
#include <yplatform/http/parse_headers.hpp>

namespace http { 

namespace detail {

inline static HttpMethod 
translate_method(const std::string& name)
{
  if (boost::iequals(name, boost::as_literal("options")))
    return method::Options;
  if (boost::iequals(name, boost::as_literal("get")))
    return method::Get;
  if (boost::iequals(name, boost::as_literal("head")))
    return method::Head;
  if (boost::iequals(name, boost::as_literal("post")))
    return method::Post;
  if (boost::iequals(name, boost::as_literal("put")))
    return method::Put;
  if (boost::iequals(name, boost::as_literal("patch")))
    return method::Patch;
  if (boost::iequals(name, boost::as_literal("delete")))
    return method::Delete;
  if (boost::iequals(name, boost::as_literal("trace")))
    return method::Trace;
  if (boost::iequals(name, boost::as_literal("connect")))
    return method::Connect;
  return method::Unknown;
}

template <typename SmartSocket, typename Handler, typename ErrorHandler>
class read_request_op
{
public:
  read_request_op(SmartSocket socket, Handler& handler,
      ErrorHandler& error_handler) :
    socket_(socket),
#if __cplusplus >= 201103L
    handler_(std::move(handler)),
    error_handler_(std::move(error_handler)),
#else
    handler_(handler),
    error_handler_(error_handler),
#endif
    start_(0),
    buffers_(new boost::asio::streambuf),
    method_(method::Unknown)
  {}

#if __cplusplus >= 201103L
  read_request_op(const read_request_op& other) :
    socket_(other.socket_),
    handler_(other.handler_),
    error_handler_(other.error_handler_),
    start_(other.start_),
    buffers_(other.buffers_),
    method_(other.method_),
    url_(other.url_)
  {}

  read_request_op(read_request_op&& other) :
    socket_(other.socket_),
    handler_(std::move(other.handler_)),
    error_handler_(std::move(other.error_handler_)),
    start_(other.start_),
    buffers_(other.buffers_),
    method_(other.method_),
    url_(std::move(other.url_))
  {}
#endif

  void operator ()()
  {
    HTTP_TRACE_ENTER_CLS();

    boost::asio::async_read_until(*socket_, *buffers_, "\r\n", *this);
  }

  void operator ()(boost::system::error_code ec, std::size_t sz = 0)
  {
    HTTP_TRACE_ENTER_CLS();

    if (ec)
    {
      // TODO: Handle an error.
      return;
    }

    switch (++start_)
    {
      case 1:
        handle_request(ec);
        return;
      case 2:
        handle_headers(ec, sz);
        return;
    }
  }

private:
  void handle_request(boost::system::error_code)
  {
    HTTP_TRACE_ENTER_CLS();

    std::istream is(&*buffers_);

    // Extract request method.
    std::string method;
    is >> method;

    // Extract request URL.
    std::string url;
    is >> url;

    // Extract HTTP protocol version.
    std::string version;
    std::getline(is >> std::ws, version);

    if (version.compare(0, 5, "HTTP/") != 0)
    {
      // TODO: Handle http::error::malformed_request error.
      return;
    }

    if (http::method::Unknown == (method_ = translate_method(method)))
    {
      // TODO: Handle http::error::unknown_method error.
      return;
    }

    url_ = url;

    // Read HTTP headers.
    boost::asio::async_read_until(*socket_, *buffers_, "\r\n\r\n", *this);
  }

  void handle_headers(boost::system::error_code, std::size_t sz)
  {
    HTTP_TRACE_ENTER_CLS();

    // Parse HTTP request headers.
    typedef boost::asio::buffers_iterator<
      boost::asio::streambuf::const_buffers_type> iterator;
    boost::asio::streambuf::const_buffers_type bufs = buffers_->data();
    yplatform::http::headers<boost::iterator_range<iterator> > headers;
    yplatform::http::parse_headers(
      boost::make_iterator_range(boost::asio::buffers_begin(bufs),
        boost::asio::buffers_begin(bufs) + sz),
      headers);
    buffers_->consume(sz);

    handler_(error_handler_, method_, url_, headers, socket_);
  }

  SmartSocket socket_;
  Handler handler_;
  ErrorHandler error_handler_;
  int start_;
  boost::shared_ptr<boost::asio::streambuf> buffers_;
  HttpMethod method_;
  yplatform::url url_;
};

template <typename Iterator, typename Handler>
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
  	  Error, http::HttpMethod, http::url,
  	  http::headers<boost::iterator_range<Iterator> >, SmartSock
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
	  Error, http::HttpMethod, http::url,
    http::headers<boost::iterator_range<Iterator> >, SmartSock
	)>::type
	operator() (Error error_h, SmartSock sock, detail::final_call_tag)
	{
    HTTP_TRACE_ENTER_CLS();

		typedef typename 
		  boost::result_of<on_request_uri_parsed (
		      Error, SmartSock, detail::final_call_tag)>::type result_type;

    read_request_op<SmartSock, _Handler, Error>(sock, handler_, error_h)();

		/*return handler_ (error_h, method::Get, http::url(),
        http::headers<boost::iterator_range<Iterator> >, sock);*/
    return boost::system::error_code();
    // return true;
  }

private:
  _Handler handler_;
};

}

#if __cplusplus >= 201103L
template <class Error, class Iterator, class SmartSock, class Handler>
detail::on_request_uri_parsed<Iterator, Handler>
on_request (Handler&& handler, typename boost::enable_if_c<
    ! detail::is_yield_context<Error>::value &&
    boost::is_same<typename boost::result_of<
      typename boost::decay<Handler>::type (
            Error, http::HttpMethod, http::url,
            http::headers<boost::iterator_range<Iterator> >, SmartSock
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
            Error, http::HttpMethod, http::url,
            http::headers<boost::iterator_range<Iterator> >, SmartSock
    )>::type, error_code>::value
 && boost::is_same<typename boost::result_of<Error (
      error_code, std::string
    )>::type, bool>::value
  , detail::enabler>::type = detail::enabler ())
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
    Error, http::HttpMethod, http::url,
    http::headers<boost::iterator_range<Iterator> >, SmartSock
  )>::type, error_code> >::type>
{
	typedef detail::on_request_uri_parsed<Iterator, Handler> type;
};

} // namespace traits
// #endif // C++03


} // namespace http
#endif // _HTTP_SERVER_ON_REQUEST_URI_PARSED_H_
