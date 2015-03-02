#ifndef _HTTP_SERVER_ERROR_HANDLER_H_
#define _HTTP_SERVER_ERROR_HANDLER_H_
#include <http_server/compat.h>
#include <http_server/asio.h>

namespace http {

template <typename Endpoint, typename SockPtr>
struct error_handler
{
  typedef compat::function<void (error_code const&, Endpoint const&,
      Endpoint const&, SockPtr)> type;
};

} // namespace http
#endif // _HTTP_SERVER_ERROR_HANDLER_H_
