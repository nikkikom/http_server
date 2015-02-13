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

#include <boost/logic/tribool.hpp>
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
	typedef compat::function<boost::tribool (
      boost::function<void(bool)>, 
      http::HttpMethod, uri::parts<request_iterator>, sock_smart_ptr
      // , detail::final_call_tag
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
      // becomes 'tribool (handler(bool), sock_smart_ptr)'. 
      // All black magic is hidden inside detail/repeat_until.h and, 
      // believe me, you do not want to look at it.
      
      detail::repeat_until< boost::tribool (
          boost::function<void(bool)>, http::HttpMethod, 
          uri::parts<request_iterator>, sock_smart_ptr
        ) > 
      (
          detail::on_request_functor<
              boost::function<void(bool)>, request_iterator, sock_smart_ptr
          > (), boost::move (handler)
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

  struct handle_user_request_accept_binder
  {
  	template <class> struct result {};
  	template <class F> 
  	struct result<F(boost::function<void(bool)>, http::HttpMethod, 
  	  uri::parts<request_iterator>, sock_smart_ptr)>
  	{
  		typedef boost::tribool type;
    };

  	server* that_;
  	typename handler_vec::const_iterator next_iter_;

    handle_user_request_accept_binder (server* that, 
      typename handler_vec::const_iterator next_iter)
      : that_ (that), next_iter_ (next_iter) {}

    boost::tribool
    operator () (boost::function<void(bool)> result_functor, 
					  http::HttpMethod method, uri::parts<request_iterator> parsed,
					  sock_smart_ptr sptr)
		{
      return that_->handle_user_request_accept (result_functor, method, 
        parsed, sptr, next_iter_, false);
    }
	};

  boost::tribool 
  handle_user_request_accept (boost::function<void(bool)> result_functor, 
    http::HttpMethod method, uri::parts<request_iterator> parsed,
    sock_smart_ptr sptr,
    typename handler_vec::const_iterator next_iter, bool ok)
  {
    HTTP_TRACE_ENTER_CLS();

    while (! ok && next_iter != handlers_.end ())
    {
    	// repeat
    	boost::tribool ret;

    	request_handler_type& user_handler = *next_iter++;

      ok = ret = user_handler (
#if __cplusplus < 201103L
          boost::bind (&server::handle_user_request_accept, this,
            result_functor, method, parsed, sptr, next_iter, _1),
#else
					// TODO: move result_functor in C++14
					[this, next_iter, result_functor, method, parsed, sptr] (bool ok)
					{ 
						handle_user_request_accept (result_functor, method, parsed,
						                            sptr, next_iter, ok);
					},
#endif
					method, parsed, sptr
          // sptr, detail::final_call_tag ()
      );

      if (boost::indeterminate (ret)) return ret;
    }

    if (ok)
    {
      HTTP_TRACE_NOTIFY ("found request handler");
      return true;
    }

    if (next_iter == handlers_.end ())
    {
      HTTP_TRACE_NOTIFY ("not found request handler");
      return false;
    }

    throw (std::runtime_error ("should not happen"));
  }

  boost::tribool 
  handle_user (
    boost::function<void(bool)> result_functor, http::HttpMethod,
    uri::parts<request_iterator> const&
  )
  {
  	return true;
  }
 
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

    if (connect_ec)
    {
    	// print error, close connection
    	return;
    }

    // TODO: implement more clever version, with handlers ordered list and
    // tree contained parsed and not-yet-parsed entries.

    // Parse http request
    boost::tribool parse_ok = http::on_request<
      boost::function<void(bool)>, request_iterator, sock_smart_ptr
    > (
#if __cplusplus < 201103L
          // For some reasons, compiler fails to deduce arguments properly 
          // for the bind wrapper objects. So I have to create my own lambda-like
          // wrapper (C++03 should die).
#if 0
          boost::bind<boost::tribool> (
              &server::handle_user_request_accept, this
            , _1 // ResultF
            , _2 // HttpMethod
            , _3 // uri::parts
            , _4 // SmartSock
            , handlers_.begin ()
            , false
          )
#else
					handle_user_request_accept_binder (this, handlers_.begin ())
#endif
#else
					[this] (boost::function<void(bool)> result_functor, 
					  http::HttpMethod method, uri::parts<request_iterator> parsed,
					  sock_smart_ptr sptr) -> boost::tribool
					{
						return handle_user_request_accept (result_functor, method, 
						  parsed, sptr, handlers_.begin (), false);
          }
#endif
   )
  	( 
  	  boost::bind (&server::handle_http_parse, this, _1),
      sptr, 
      detail::final_call_tag ()
    );

  	if (! parse_ok) 
    {
      HTTP_TRACE_NOTIFY ("HTTP request line parse error");
      return;
    }

  	if (parse_ok) 
    {
      HTTP_TRACE_NOTIFY ("HTTP request line parse ok");
      return;
    }

    // parse_ok == boost::indeterminate, do nothing
  }

  void handle_http_parse (bool ok)
  {
    HTTP_TRACE_ENTER_CLS();
    HTTP_TRACE_NOTIFY ("HTTP request line parse " << ok << " (delayed)");
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
