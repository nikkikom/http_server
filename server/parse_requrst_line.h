#ifndef _HTTP_SERVER_PARSE_REQUEST_LINE_H_
#define _HTTP_SERVER_PARSE_REQUEST_LINE_H_

#include <http_server/trace.h>
#include <http_server/compat.h>
#include <http_server/method.h>

#if __cplusplus >= 201103L
# include <type_traits>
#endif

#include <utility> // std::move, forward, etc

namespace http { _inline_ns namespace _server {

namespace detail 
{

class uri_parser
{
protected:
  template <typename SmartSock>
  class op: public boost::enable_shared_from_this<op<SmartSock> >
  {
  public:
    op (SmartSock sock) : sock_ (boost::move (sock)) {}

    void start ()
    {
    	asio::async_read_until (*sock
  private:
    SmartSock sock_;
  };

public:
#if __cplusplus >= 201103L
  using _Handler = typename std::decay<Handler>::type;

  template <typename H, class = typename std::enable_if<
      std::is_same<typename std::decay<H>::type, _Handler>::value>::type>
	uri_parser (H&& handler)
	  : handler_ (std::forward<H> (handler))
	{
  }
#else
	typedef Handler _Handler;

	uri_parser (Handler const& handler)
	  : handler_ (handler)
	{
  }
#endif

  template <typename SmartSock>
	bool operator() (SmartSock sock) const
	{
		// 1. read line from socket


		// 2. parse line
    
		// 3. call user handler
  }

private:
  _Handler handler_;
};

}

#if __cplusplus >= 201103L
template <typename Handler>
detail::uri_parser<typename std::decay<Handler>::type>
uri_parser (Handler&& handler)
{
	typedef typename std::decay<Handler>::type _Handler;
	return detail::uri_parser<_Handler> (std::forward<_Handler> (handler));
}
#else
template <typename Handler>
detail::uri_parser<typename std::decay<Handler>::type>
uri_parser (Handler const& handler)
{
	return detail::uri_parser<Handler> (handler);
}
#endif

}} // namespace http::_server
#endif // _HTTP_SERVER_PARSE_REQUEST_LINE_H_
