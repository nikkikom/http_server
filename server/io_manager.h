#ifndef _HTTP_SERVER_IO_MANAGER_H_
#define _HTTP_SERVER_IO_MANAGER_H_
#include <http/asio.h>
#include <http/compat.h>
#include <http/trace.h>
#include <http/debug.h>

#include <http/detail/mutex.h>

#include <boost/thread.hpp>
#include <boost/utility/addressof.hpp>
#include <boost/move/utility_core.hpp>
#include <boost/throw_exception.hpp>

#include <boost/ptr_container/ptr_vector.hpp>

#include <boost/move/core.hpp>

#if __cplusplus < 201103L
# include <boost/bind.hpp>
#endif

#include <memory>

namespace http { _inline_ns namespace _server {

template <typename Socket>
struct io_asio_traits
{
  typedef typename asio::io_service service_type;
	typedef typename Socket::endpoint_type endpoint_type;
	typedef typename Socket::protocol_type protocol_type;
	typedef typename protocol_type::acceptor acceptor_type;
};

template <
    typename Socket = tcp::socket
  , typename Traits = io_asio_traits<Socket> 
>
class io_manager 
{
private:
  BOOST_COPYABLE_AND_MOVABLE (io_manager)
  io_manager& operator= (BOOST_COPY_ASSIGN_REF(io_manager) im);

public:
  typedef Traits              traits_type;

  typedef Socket              socket_type;
  typedef socket_type      *  socket_ptr;
  typedef socket_type const*  socket_const_ptr;

  typedef typename traits_type::service_type service_type;
  typedef typename traits_type::endpoint_type endpoint_type;
  typedef typename traits_type::acceptor_type acceptor_type;

  io_manager (service_type& io) 
    : io_ (io)
  {
  	HTTP_TRACE_ENTER_CLS();
  }

  // do not copy acceptors
  io_manager (io_manager const& im)
    : io_ (im.io_)
  {
  	HTTP_TRACE_ENTER_CLS();
  }

  io_manager (BOOST_RV_REF(io_manager) im)
    : io_ (im.io_)
    , acceptors_ (boost::move (im.acceptors_))
  {
  	HTTP_TRACE_ENTER_CLS();
  }

  void run ()
  {
  	HTTP_TRACE_ENTER_CLS();
  	io_.run ();
  }

  socket_ptr create ()
  {
  	HTTP_TRACE_ENTER_CLS();
    return new socket_type (io_);
  }

  static void destroy (socket_ptr sock)
  {
  	HTTP_TRACE_ENTER();
    assert (sock);
    delete sock;
  }

  template <typename Handler>
  void connect (socket_ptr sock, Handler handler)
  {
  	HTTP_TRACE_ENTER_CLS();
#if 0
    asio::spawn (
        sock->get_io_service (), make_connector (*sock, ep_, handler));
#endif
  }

  static sys::error_code close (socket_ptr sock)
  {
  	HTTP_TRACE_ENTER();
  	sys::error_code ec;
  	return sock->close (ec);
  }

  static bool is_open (socket_const_ptr* sock) 
  { 
  	HTTP_TRACE_ENTER();
  	return sock->is_open (); 
  }

  static bool ping (socket_type& sock);

#if 0
  endpoint_type endpoint () const 
  { 
  	HTTP_TRACE_ENTER_CLS();
  	return ep_; 
  }
#endif

  template <typename Handler>
	void 
	listen_on (endpoint_type const& ep, Handler handler)
	{
  	HTTP_TRACE_ENTER_CLS();

		detail::lock_guard<detail::mutex> lock (mux_);

  	AUTO_PTR<acceptor_type> acc (new acceptor_type (io_, ep));
		acceptors_.push_back (acc.get ());
		try { accept (*acc, ep, handler); }
  	catch (...) { 
  		acceptors_.pop_back (); 
  		throw; 
  	}

  	acc.release ();
	}

protected:
  template <typename Handler>
  void accept (acceptor_type& acc, endpoint_type const& ep, Handler handler)
  {
  	HTTP_TRACE_ENTER_CLS();

    socket_ptr sock_ptr;
    AUTO_PTR<socket_type> sock (sock_ptr = create ());

    endpoint_type* eptr;
    AUTO_PTR<endpoint_type> remote_ep (eptr = new endpoint_type);

    acc.async_accept (*sock, *eptr,
#if __cplusplus < 201103L
      boost::bind (&io_manager::template handle_accept<Handler>, this,
        asio::placeholders::error, boost::ref (acc), ep, eptr,
        sock_ptr, handler
      )
#else
			[this,&acc,sock_ptr,ep,eptr,handler] (error_code const& ec) {
        handle_accept<Handler> (ec, acc, ep, eptr, sock_ptr, handler);
      }
#endif
    ); 

    remote_ep.release ();
    sock.release ();
  }

protected:
  template <typename Handler>
  void handle_accept (error_code const& ec
	, acceptor_type& acc,
      endpoint_type const& local_ep, endpoint_type const* remote_ep,
      socket_ptr sock, Handler handler)
  {
  	assert (remote_ep);

#if __cplusplus >= 201103L
		auto _deleter = [this] (socket_ptr s) { destroy (s); };
		std::unique_ptr<socket_type, decltype (_deleter)> s_guard (sock, _deleter);

    std::unique_ptr<endpoint_type const> ep_guard (remote_ep);
#else
    try {
#endif
      HTTP_TRACE_ENTER_CLS();

      accept (acc, local_ep, handler);
    
      // FIXME: should be remote_ep instead
      handler (ec, *remote_ep, *sock);

#if __cplusplus < 201103L
    } 
    catch (...) 
    {
    	delete remote_ep;
    	destroy (sock); 
    	throw;
    }

    delete remote_ep;
    destroy (sock); 
#endif
  }

private:
  detail::mutex mux_;
  service_type& io_;
  // endpoint_type ep_;
  boost::ptr_vector<acceptor_type> acceptors_;
};

}} // namespace http::_server

#endif // _HTTP_SERVER_IO_MANAGER_H_
