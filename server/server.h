#ifndef _HTTP_SERVER_SERVER_H_
#define _HTTP_SERVER_SERVER_H_

#include <http_server/trace.h>
#include <http_server/compat.h>
#include <http_server/method.h>
#include <http_server/server/io_manager.h>
#include <http_server/callback_handler.h>
#include <http_server/return_to_type.h>
#include <http_server/convert_callback_to_coro.h>
#include <http_server/tag.h>

#include <http_server/detail/no_pool.h>

#include <boost/noncopyable.hpp>

#if !DISABLE_BOOST_REGEX
# include <boost/regex.hpp>
#endif

#if __cplusplus >= 201103L && !DISABLE_STD_REGEX
# include <regex>
#endif

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
	    HttpMethod, 
	    uri::parts<request_iterator>,
      
      sock_smart_ptr
	  )> handler_type;

	typedef boost::container::stable_vector<handler_type> handler_vec;

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

  template <typename Handler>
  server& register_path (std::string const& path, Handler handler)
  {
    HTTP_TRACE_ENTER_CLS();
    return *this;
  }

#if !DISABLE_BOOST_REGEX
  template <typename CharT, typename Traits, typename Handler>
  server& register_path (::boost::basic_regex<CharT,Traits> const& rgx,
      Handler handler)
  {
    HTTP_TRACE_ENTER_CLS();
    return *this;
  }
#endif

#if __cplusplus >= 201103L && !DISABLE_STD_REGEX
  template <typename CharT, typename Traits, typename Handler>
  server& register_path (::std::basic_regex<CharT,Traits> const& rgx,
      Handler handler)
  {
    HTTP_TRACE_ENTER_CLS();
    return *this;
  }
#endif

  template <typename Predicate, typename Handler>
  server& register_path (Predicate pred, Handler handler)
  {
    HTTP_TRACE_ENTER_CLS();
    return *this;
  }

  // coro version
  template <typename ConnectHandler>
  server& on_connect (ConnectHandler handler, typename boost::enable_if_c<
       boost::is_void<typename boost::result_of<ConnectHandler (
            asio::yield_context, error_code, endpoint_type, endpoint_type, 
            sock_smart_ptr
       )>::type>::value 
    || boost::is_same<typename boost::result_of<ConnectHandler (
            asio::yield_context, error_code, endpoint_type, endpoint_type, 
            sock_smart_ptr
       )>::type, bool>::value 
    || boost::is_same<typename boost::result_of<ConnectHandler (
            asio::yield_context, error_code, endpoint_type, endpoint_type, 
            sock_smart_ptr
       )>::type, error_code>::value 
    , enabler>::type = enabler ())
  {
    HTTP_TRACE_ENTER_CLS();

    on_connect_handler_ = convert_on_connect_to_coro (
          return_to_type (
            boost::move (handler), 
            make_error_code (sys::errc::io_error)
          )
    );

    return *this;
  }

  
  /////////////////////////////////////////////////////////////////////////////
  // callback version
  template <typename ConnectHandler>
  server& on_connect (ConnectHandler handler, typename boost::enable_if_c<
       boost::is_void<typename boost::result_of<ConnectHandler (
            error_code, endpoint_type, endpoint_type, sock_smart_ptr
       )>::type>::value 
    || boost::is_same<typename boost::result_of<ConnectHandler (
            error_code, endpoint_type, endpoint_type, sock_smart_ptr
       )>::type, bool>::value 
    || boost::is_same<typename boost::result_of<ConnectHandler (
            error_code, endpoint_type, endpoint_type, sock_smart_ptr
       )>::type, error_code>::value 
    , enabler>::type = enabler ())
  {
    HTTP_TRACE_ENTER_CLS();
    on_connect_handler_ = return_to_type (
        boost::move (handler),
        make_error_code (sys::errc::io_error)
    );
    return *this;
  }

  template <typename RequestHandler>
  server& 
  on_request (RequestHandler handler,
    typename boost::enable_if<
      boost::is_same<typename boost::result_of<RequestHandler ()>::type, bool>,
      enabler>::type = enabler ())
  {
    HTTP_TRACE_ENTER_CLS();

#if 0
  	handlers_.push_back (
  	 normalize_handler<request_iterator> (handler)
  	);
#endif

#if 0
    // FIXME: debug
    HTTP_TRACE_NOTIFY("pred(/a/b/c/d/e)");
  	std::cout << pred (method::Get, boost::as_literal ("/a/b/c/d/e")) << "\n";
#endif
  	return *this;
  }

#if __cplusplus < 201103L
  template <typename RequestHandler>
  server& 
  on_request (RequestHandler handler,
    typename boost::enable_if<
      boost::is_same<typename boost::result_of<RequestHandler (
        http::HttpMethod, uri::parts<request_iterator>, sock_smart_ptr
      )>::type, bool>,
      enabler>::type = enabler ())
  {
    HTTP_TRACE_ENTER_CLS();

  	handlers_.push_back (
  	 normalize_handler<request_iterator, sock_smart_ptr, RequestHandler> (
  	    boost::move (handler)
  	  )
  	);
  	return *this;
  }
#else
  template <typename RequestHandler>
  server& 
  on_request (RequestHandler&& handler,
    typename boost::enable_if<
      boost::is_same<typename boost::result_of<RequestHandler (
        http::HttpMethod, uri::parts<request_iterator>, sock_smart_ptr
      )>::type, bool>,
      enabler>::type = enabler ())
  {
    HTTP_TRACE_ENTER_CLS();

  	handlers_.push_back (
  	 normalize_handler<request_iterator, sock_smart_ptr> (
  	    std::forward<RequestHandler> (handler)
  	  )
  	);
  	return *this;
  }
#endif

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
    	BOOST_FOREACH (handler_type& handler, handlers_)
    	{
    		if (true == 
    			  (ok = handler (method::Get, uri::parts<request_iterator> (), sptr)))
    			break;
      }

      HTTP_TRACE_NOTIFY ("found request handler=" << ok);

      if (! ok) connect_ec = make_error_code (sys::errc::function_not_supported);
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

  compat::function<error_code (error_code const&, endpoint_type const&, 
      endpoint_type const&, sock_smart_ptr)> on_connect_handler_;
};


}} // namespace http::_server

#if 0
http::server srv (io);

srv
  .listen_on ()
  .listen_on ()
  .register_path (cb)
  .register_path (cb)
;
#endif


#endif // _HTTP_SERVER_SERVER_H_
