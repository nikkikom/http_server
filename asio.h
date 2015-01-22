#ifndef _HTTP_ASIO_H_
#define _HTTP_ASIO_H_
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/steady_timer.hpp>

namespace http {
  namespace asio = ::boost::asio;
  namespace sys = ::boost::system;
  using asio::ip::tcp;
  using sys::error_code;
}

//#define HTTP_ASIO_NS ::boost::asio
//#define HTTP_TCP_NS HTTP_ASIO_NS::ip::tcp

#endif // _HTTP_ASIO_H_
