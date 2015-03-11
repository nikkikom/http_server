#ifndef _HTTP_SERVER_ERROR_HANDLER_H_
#define _HTTP_SERVER_ERROR_HANDLER_H_
#include <http_server/compat.h>
#include <http_server/asio.h>

#include <string>

namespace http {

template <typename Endpoint, typename SockPtr>
struct error_handler
{

  typedef compat::function<bool (
      error_code const&   // error
    , std::string const&  // error details message
                          // (should not duplicate err_code)
#if 0
    , Endpoint const&     // local endpoint
    , Endpoint const&     // remote endpoint
    , SockPtr             // socket
#endif
  )> type;
};

} // namespace http
#endif // _HTTP_SERVER_ERROR_HANDLER_H_
