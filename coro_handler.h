#ifndef _HTTP_CORO_HANDLER_H_
#define _HTTP_CORO_HANDLER_H_
#include <http_server/method.h>
#include <http_server/uri/parts.h>

#include <boost/utility/enable_if.hpp>
#include <boost/utility/result_of.hpp>
#include <boost/type_traits.hpp>
#include <boost/range.hpp>

#if __cplusplus < 201103L
# include <boost/shared_ptr.hpp>
#else
# include <memory> // unique_ptr
#endif

# include <boost/move/move.hpp>
#include <utility>

namespace http {

namespace detail {

class socket_deleter 
{
public:
  socket_deleter () {}

  template <typename Socket>
  void operator() (Socket* sock) const
  {
  }
};

template <typename Handler>
class callback_handler
{
public:
#if __cplusplus < 201103L
  callback_handler (Handler handler) : handler_ (boost::move (handler)) {}
#else
	callback_handler (Handler const& handler) : handler_ (handler) {}
	callback_handler (Handler&& handler) : handler_ (std::move	(handler)) {}
#endif

  template <typename Iterator, typename SocketPtr>
  bool operator() (method m, uri::parts<Iterator> parsed, SocketPtr sock) const
  {
  	typedef typename boost::remove_pointer<SocketPtr>::type Socket;
  	// convert socket into smart object
#if __cplusplus >= 201103L
		typedef std::unique_ptr<Socket, socket_deleter> sock_smart_ptr;
		sock_smart_ptr sptr (sock, socket_deleter ());
		return handler_ (m, parsed, std::move (sptr));
#else
		typedef boost::shared_ptr<Socket> sock_smart_ptr;
		sock_smart_ptr sptr (sock, socket_deleter ());
		return handler_ (m, parsed, sptr);
#endif
  }

private:
  Handler handler_;
};

} // namespace detail

template <typename Iterator, typename SmartSocket, typename Handler>
detail::callback_handler<Handler>
normalize_handler (Handler handler, typename boost::enable_if<
    boost::is_same<
        typename boost::result_of<Handler (
          http::method, uri::parts<Iterator>, SmartSocket
        )>::type
      , bool
    >
  >::type* = 0)
{
	return detail::callback_handler<Handler> (handler);
}

} // namespace http
#endif // _HTTP_CORO_HANDLER_H_
