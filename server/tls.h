#ifndef _HTTP_SERVER_TLS_H_
#define _HTTP_SERVER_TLS_H_

#include <http_server/trace.h>
#include <http_server/compat.h>
#include <http_server/asio.h>

#include <boost/noncopyable.hpp>

#if __cplusplus < 201103L
# include <boost/move/move.hpp>
#endif

#include <utility> // std::move, forward, etc

namespace http { _inline_ns namespace _server {
namespace detail {

template <typename Handler>
class tls_starter
{
public:
  tls_starter (Handler handler) 
    : handler_ (boost::move (handler))
  {
  }

  void operator() (asio::yield_context const& yield, sys::error_code const& ec,
      endpoint_type const& remote, socket_type& sock) const
  {
    typedef asio::ssl::stream<socket_type> ssl_socket;
    asio::ssl::context ssl_ctx (ssl::context::sslv23);
    ssl_ctx.set_default_verify_paths ();

    ssl_socket ssl (sock, ssl_ctx);

    // Coroutines handshake
    sys::error_code ecc;
    ssl.async_handshake (asio::ssl::stream_base::server, yield[ecc]);

    handler_ (

  }

private:
  Hanlder handler_;
};

template <typename Handler>
tls_starter<Handler> 
tls_start (Handler handler)
{
	return tls_starter<Handler> (boost::move (handler));
}

} // namespace detail
}} // namespace http::_server


#endif // _HTTP_SERVER_TLS_H_
