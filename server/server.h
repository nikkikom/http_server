#ifndef _HTTP_SERVER_SERVER_H_
#define _HTTP_SERVER_SERVER_H_

#include <http_server/trace.h>
#include <http_server/compat.h>
#include <http_server/method.h>
#include <http_server/server/io_manager.h>
#include <http_server/on_connect.h>
#include <http_server/on_request.h>
#include <http_server/tag.h>

#include <http_server/detail/no_pool.h>
#include <http_server/detail/repeat_until.h>

#include <boost/noncopyable.hpp>

#if __cplusplus < 201103L
# include <boost/bind.hpp>
# include <boost/bind/protect.hpp>
# include <boost/move/move.hpp>
#endif

#include <boost/foreach.hpp>
#include <boost/container/stable_vector.hpp>

#include <utility> // std::move, forward, etc

namespace http { _inline_ns namespace _server {

template <class Manager = io_manager<>, class Pool = detail::no_pool>
class server: boost::noncopyable
{
public:
  typedef Manager manager_type;
  typedef Pool pool_type;

  typedef typename manager_type::socket_type socket_type;
  typedef typename manager_type::socket_ptr socket_ptr;
	typedef boost::shared_ptr<socket_type> sock_smart_ptr;

  typedef typename manager_type::endpoint_type endpoint_type;

private:
	struct enabler {};
  typedef char const* request_iterator;
	typedef compat::function<bool(
      sock_smart_ptr, detail::final_call_tag
	  )> request_handler_type;

	typedef boost::container::stable_vector<request_handler_type> handler_vec;

public:
  server (typename manager_type::service_type& io) 
    : manager_ (io)
    , pool_ (io)
    , on_connect_handler_ (&server::on_connect_default)
  {
  	HTTP_TRACE_ENTER_CLS();
  }

  ~server () 
  {
  	HTTP_TRACE_ENTER_CLS();
  }

  void run ()
  {
  	HTTP_TRACE_ENTER_CLS();
  	manager_.run ();
  }

  server& set_keep_alive (...)
  {
  	HTTP_TRACE_ENTER_CLS();

  	return *this;
  }

  server&
  listen_on (endpoint_type const& ep)
  {
  	HTTP_TRACE_ENTER_CLS();
    
    manager_.listen_on (ep, 
#if __cplusplus < 201103L
      boost::protect (
        boost::bind (&server::handle_accept, this, _1, ep, _2, _3)
      )
#else
			[this, ep] 
			(error_code const& ec, endpoint_type const& remote_ep, socket_ptr sock)
			{
				handle_accept (ec, ep, remote_ep, sock);
      }
#endif
    );

    return *this;
  }

  // Start SSL/TLS acceptor
  server&
  listen_on (endpoint_type const& ep, tag::tls_t)
  {
  	HTTP_TRACE_ENTER_CLS();
    
    manager_.listen_on (ep, 
#if __cplusplus < 201103L
      boost::protect (
        boost::bind (&server::handle_accept, this, _1, ep, _2, _3)
      )
#else
			[this, ep] 
			(error_code const& ec, endpoint_type const& remote_ep, socket_ptr sock)
			{
				handle_accept (ec, ep, remote_ep, sock);
      }
#endif
    );

    return *this;
  }

	// TODO: split into c++03 and c++11 args forwarding
  template <typename ConnectHandler>
  server& 
  on_connect (ConnectHandler handler)
  {
    HTTP_TRACE_ENTER_CLS();
    on_connect_handler_ = 
        http::on_connect<connect_handler_type, endpoint_type, sock_smart_ptr> (
            boost::move (handler));
    return *this;
  }
	
	// TODO: split into c++03 and c++11 args forwarding
  template <typename RequestHandler>
  server& 
  on_request (RequestHandler handler)
  {
    HTTP_TRACE_ENTER_CLS();

  	handlers_.push_back (
      // do recursive 'on_request' calls until resulting handler signature 
      // becomes 'bool (sock_smart_ptr)'. All black magic is hidden inside
      // detail/repeat_until.h but, believe me, you do not want to look at it.
      
      detail::repeat_until< bool (sock_smart_ptr, detail::final_call_tag) > (
        detail::on_request_functor<
          request_handler_type, request_iterator, sock_smart_ptr> (), 
        boost::move (handler)
      )
  	);

  	return *this;
  }

protected:
  class socket_dispose
  {
  	manager_type& manager_;
  	pool_type& pool_;

  public:
    socket_dispose (manager_type& manager, pool_type& pool)
      : manager_ (manager)
      , pool_ (pool)
    {}

    template <typename SockPtr>
    void operator() (SockPtr* sock_ptr) const
    {
    	// TODO: check sock state and return to pool?
    	manager_.destroy (sock_ptr);
    }
  };

  void handle_accept (error_code const& ec, 
      endpoint_type const& local_ep, endpoint_type const& remote_ep,
      socket_ptr sock)
  {
    HTTP_TRACE_ENTER_CLS();

    // FIXME: check ec!

    // Converting socket into smart object.
    sock_smart_ptr sptr (sock, socket_dispose (manager_, pool_));

    error_code connect_ec = 
        on_connect_handler_ (ec, local_ep, remote_ep, sptr);

    if (! connect_ec)
    {
    	// continue 
    	bool ok = false;
    	BOOST_FOREACH (request_handler_type& handler, handlers_)
    	{
    		// ok = handler (method::Get, uri::parts<request_iterator> (), sptr);
    		ok = handler (sptr, detail::final_call_tag ());
    		if (ok) break;
      }

      HTTP_TRACE_NOTIFY ("found request handler=" << ok);

      if (! ok) 
      	connect_ec = make_error_code (sys::errc::function_not_supported);
    }

    if (connect_ec)
    {
    	// print error, close connection
    }
  }

protected:
  static error_code on_connect_default (error_code const&, endpoint_type const&,
      endpoint_type const&, sock_smart_ptr)
  {
  	return error_code ();
  }

private:
  manager_type manager_;
  pool_type pool_;
  handler_vec handlers_;

  typedef compat::function<error_code (error_code const&, endpoint_type const&, 
      endpoint_type const&, sock_smart_ptr)> connect_handler_type;
      
  connect_handler_type on_connect_handler_;
};


}} // namespace http::_server

#endif // _HTTP_SERVER_SERVER_H_
