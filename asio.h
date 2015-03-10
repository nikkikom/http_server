#ifndef _HTTP_ASIO_H_
#define _HTTP_ASIO_H_
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/steady_timer.hpp>

#if defined(BOOST_ASIO_HAS_STD_CHRONO)
# include <chrono>
#elif defined(BOOST_ASIO_HAS_BOOST_CHRONO)
# include <boost/chrono/system_clocks.hpp>
#endif

namespace http {
  namespace asio = ::boost::asio;
  namespace sys = ::boost::system;
  using asio::ip::tcp;
  using sys::error_code;
  
  typedef asio::steady_timer::clock_type clock_type;
  typedef asio::steady_timer::duration duration;
  typedef asio::steady_timer::time_point time_point;

#if defined(BOOST_ASIO_HAS_STD_CHRONO)
  namespace chrono = std::chrono;
#elif defined(BOOST_ASIO_HAS_BOOST_CHRONO)
  namespace chrono = boost::chrono;
#endif
  
}

//#define HTTP_ASIO_NS ::boost::asio
//#define HTTP_TCP_NS HTTP_ASIO_NS::ip::tcp

#endif // _HTTP_ASIO_H_
