#ifndef _HTTP_SERVER_SERVER_H_
#define _HTTP_SERVER_SERVER_H_

#include <http_server/trace.h>
#include <http_server/compat.h>
#include <http_server/method.h>
#include <http_server/server/io_manager.h>
#include <http_server/error.h>
#include <http_server/error_handler.h>
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
  typedef typename error_handler<endpoint_type, sock_smart_ptr>::type 
     error_handler_type;
      
  // typedef char const* request_iterator;
  typedef boost::asio::buffers_iterator<
      boost::asio::streambuf::const_buffers_type> request_iterator;

  typedef http::headers<boost::iterator_range<request_iterator> > headers_type;

  // on request lower layer signature
	typedef compat::function<error_code (
      error_handler_type
    , http::HttpMethod
    , yplatform::url
    , headers_type
    , sock_smart_ptr
    // , detail::final_call_tag
	)> request_handler_type;

	typedef boost::container::stable_vector<request_handler_type> handler_vec;

public:
  server (typename manager_type::service_type& io) 
    : manager_ (io)
    , pool_ (io)
    , on_connect_handler_ (&server::on_connect_default)
    , on_error_handler_ (&server::on_error_default)
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
  
  template <typename Rep, typename Period>
  server&
  set_init_timeout (chrono::duration<Rep,Period> const& dur)
  {
    init_timeout_ = duration_cast<duration> (dur);
    return *this;
  }
  
  template <typename Rep, typename Period>
  server&
  set_idle_timeout (chrono::duration<Rep,Period> const& dur)
  {
    idle_timeout_ = duration_cast<duration> (dur);
    return *this;
  }
  
  template <typename Rep, typename Period>
  server&
  set_read_timeout (chrono::duration<Rep,Period> const& dur)
  {
    read_timeout_ = duration_cast<duration> (dur);
    return *this;
  }

  template <typename Rep, typename Period>
  server&
  set_write_timeout (chrono::duration<Rep,Period> const& dur)
  {
    write_timeout_ = duration_cast<duration> (dur);
    return *this;
  }
  

#if __cplusplus < 201103L
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
#else
  template <typename ConnectHandler>
  server& 
  on_connect (ConnectHandler&& handler)
  {
    HTTP_TRACE_ENTER_CLS();
    on_connect_handler_ = 
        http::on_connect<connect_handler_type, endpoint_type, sock_smart_ptr> (
            std::forward<ConnectHandler> (handler));
    return *this;
  }
#endif
	
#if __cplusplus < 201103L
  template <typename ErrorHandler>
  server& 
  on_error (ErrorHandler handler)
  {
    HTTP_TRACE_ENTER_CLS();
    on_error_handler_ = boost::move (handler);
    return *this;
  }
#else
  template <typename ErrorHandler>
  server& 
  on_error (ErrorHandler&& handler)
  {
    HTTP_TRACE_ENTER_CLS();
    on_error_handler_ = std::forward<ErrorHandler> (handler);
    return *this;
  }
#endif
	
#if __cplusplus < 201103L
  template <typename RequestHandler>
  server& 
  on_request (RequestHandler handler)
  {
    HTTP_TRACE_ENTER_CLS();

  	handlers_.push_back (
      // do recursive 'on_request' calls until resulting handler signature 
      // becomes 'error_code (error_handler, sock_smart_ptr)'. 
      // All black magic is hidden inside detail/repeat_until.h but, 
      // believe me, you do not want to look at it.
      
      detail::repeat_until<error_code (error_handler_type
          , http::HttpMethod, http::url
          , headers_type
          , sock_smart_ptr
          //, detail::final_call_tag
        )> 
      (
          detail::on_request_functor<
            error_handler_type, request_iterator, sock_smart_ptr
          > ()
        , boost::move (handler)
      )
  	);

  	return *this;
  }
#else
  template <typename RequestHandler>
  server& on_request (RequestHandler&& handler)
  {
    HTTP_TRACE_ENTER_CLS();

  	handlers_.push_back (
      // do recursive 'on_request' calls until resulting handler signature 
      // becomes 'error_code (error_handler, sock_smart_ptr)'. 
      // All black magic is hidden inside detail/repeat_until.h but, 
      // believe me, you do not want to look at it.
      
      detail::repeat_until<error_code (error_handler_type
          , http::HttpMethod, yplatform::url, headers_type
          , sock_smart_ptr
          //, detail::final_call_tag
        )> 
      (
          detail::on_request_functor<
            error_handler_type, request_iterator, sock_smart_ptr
          > ()
        , std::forward<RequestHandler> (handler)
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

#if __cplusplus < 201103L
  struct handle_parsed_request_accept_binder
  {
  	template <class> struct result {};

  	template <class F, class Error> struct result<F (Error,
  	  http::HttpMethod, http::url, headers_type, sock_smart_ptr)>
  	{
  		// Error should be bool (error_code, std::string)
  		BOOST_STATIC_ASSERT_MSG (
  		  (boost::is_convertible<Error, error_handler_type>::value),
  		      "Incompatible Error handler signature");

  		typedef error_code type;
    };

    server* that_;
    typename handler_vec::const_iterator next_iter_;

    handle_parsed_request_accept_binder (server* that,
        typename handler_vec::const_iterator next_iter)
      : that_ (that)
      , next_iter_ (next_iter)
    {
    }

    error_code 
    operator() (error_handler_type result_functor, 
        http::HttpMethod method, http::url parsed,
        headers_type headers, sock_smart_ptr sptr)
    {
    	return that_->handle_parsed_request_accept (result_functor, method,
    	    parsed, headers, sptr, next_iter_, 
    	    make_error_code (error::inappropriate_handler));
    }
  };
#endif

  error_code
  handle_parsed_request_accept (error_handler_type result_handler,
    http::HttpMethod method, http::url parsed,
    headers_type headers, sock_smart_ptr sptr,
    typename handler_vec::const_iterator next_iter, error_code ec)
  {
  	HTTP_TRACE_ENTER_CLS();

  	while (ec == make_error_code (error::inappropriate_handler) 
  		  && next_iter != handlers_.end ())
    {
    	request_handler_type& user_handler = *next_iter++;

    	ec = user_handler (
// #if __cplusplus < 201300L
			  boost::bind (&server::handle_parsed_request_accept, this,
			    result_handler, method, parsed, headers, sptr, next_iter, _1),
// #else // lambda
// #endif
        method, parsed, headers, sptr // , detail::final_call_tag ()
      );

      if (ec != make_error_code (error::inappropriate_handler))
        return ec;
    }

    if (ec == error_code ())
    {
    	HTTP_TRACE_NOTIFY ("found request handler");
    }

    if (next_iter == handlers_.end ())
    {
    	HTTP_TRACE_NOTIFY ("cannot find compatible request handler");
    	return make_error_code (error::no_suitable_request_handler);
    }

    return ec;
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
    	on_error_handler_ (connect_ec, std::string ());
    }

    // TODO: should implement more clever version, with handlers ordered list 
    // and tree contained parsed and not-yet-parsed entries.
    // ....

    // Parse http request now.
    error_code ret_ec = http::on_request<
        error_handler_type, request_iterator, sock_smart_ptr
    > ( // 1. find/create the proper handler
#if __cplusplus < 201103L
#if 0 // С boost::bind и boost::function не работает boost::result_of
		  // Было некогда разбираться, поэтому я сделал 
		  // handle_parsed_request_accept_binder вместо них.
		    
		    // convert to function because boost::result_of has the problems 
		    // with boost::bind return type.
		    boost::function<error_code (error_handler_type, http::HttpMethod,
		        http::url, headers_type, sock_smart_ptr, 
		        typename handler_vec::const_iterator, error_code)> 
		    (
          boost::bind<error_code> (
              &server::handle_parsed_request_accept, this
            , _1 // ResultF
            , _2 // HttpMethod
            , _3 // http::url
            , _4 // headers
            , _5 // SmartSock
            , handlers_.begin ()
            , make_error_code (error::inappropriate_handler)
          )
        )
#else
				handle_parsed_request_accept_binder (this, handlers_.begin ())
#endif
#else
				[this] (error_handler_type result_functor, 
				    http::HttpMethod method, http::url parsed, headers_type headers,
				    sock_smart_ptr sptr) -> error_code
				{
					return handle_parsed_request_accept (result_functor, method,
					    parsed, headers, sptr, handlers_.begin (), 
					    make_error_code (error::inappropriate_handler));
				}
#endif
    )
      // 2. call the handler
    (
      boost::bind<bool> (&server::on_error_default, _1, _2),
      sptr,
      detail::final_call_tag ()
    );

    if (! ret_ec)
    {
    	HTTP_TRACE_NOTIFY ("HTTP request line parse error");
    	return;
    }

    HTTP_TRACE_NOTIFY ("HTTP request line parse ok");
  }

protected:
  static error_code on_connect_default (error_code const&, endpoint_type const&,
      endpoint_type const&, sock_smart_ptr) // constexpr
  {
  	return error_code ();
  }

  static bool on_error_default (error_code const&, std::string const&
      // , endpoint_type const&, endpoint_type const&, sock_smart_ptr
  ) {
  	return false;
  }

private:
  typedef compat::function<error_code (error_code const&, endpoint_type const&, 
      endpoint_type const&, sock_smart_ptr)> connect_handler_type;

  manager_type          manager_;
  pool_type             pool_;
  handler_vec           handlers_;

  error_handler_type    on_error_handler_;
  connect_handler_type  on_connect_handler_;
  
  duration init_timeout_; // timeout after initial connect
  duration idle_timeout_; // timeout between requests
  duration read_timeout_; // current read timeout
  duration write_timeout_; // current write timeout
};


}} // namespace http::_server

#endif // _HTTP_SERVER_SERVER_H_
