#ifndef _HTTP_SERVER_SERVER_H_
#define _HTTP_SERVER_SERVER_H_

#include <http_server/trace.h>
#include <http_server/compat.h>
#include <http_server/server/io_manager.h>
#include <http_server/request_predicate.h>

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

#include <utility> // std::move, forward, etc

namespace http { _inline_ns namespace _server {

template <class Manager = io_manager<>, class Pool = detail::no_pool>
class server: boost::noncopyable
{
	struct enabler {};

  typedef char const* request_iterator;

	typedef compat::function<bool(
	    method, 
	    boost::iterator_range<request_iterator>,
	    uri::parts<request_iterator>
	  )> predicate_type;
	typedef compat::function<void()> handler_type;

	typedef std::vector<std::pair<predicate_type, handler_type> > handler_vec;

public:
  typedef Manager manager_type;
  typedef Pool pool_type;

  typedef typename manager_type::socket_type socket_type;
  typedef typename manager_type::socket_ptr socket_ptr;

  typedef typename manager_type::endpoint_type endpoint_type;

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
			(error_code const& ec, endpoint_type const& remote_ep, socket_type& sock)
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

  template <typename ConnectHandler>
  server& on_connect (ConnectHandler handler, typename boost::enable_if<
      typename boost::is_same<typename boost::result_of<ConnectHandler (
            error_code, endpoint_type, endpoint_type, socket_type&
        )>::type, void>::type, enabler>::type = enabler ())
  {
    HTTP_TRACE_ENTER_CLS();
    on_connect_handler_ = on_connect_true_<ConnectHandler> (handler);
    return *this;
  }

#if __cplusplus >= 201103L
  template <typename ConnectHandler>
  server& 
  on_connect (ConnectHandler&& handler, typename boost::enable_if<
      typename boost::is_same<
          typename boost::result_of<ConnectHandler (
            error_code, endpoint_type, endpoint_type, socket_type&
          )>::type, error_code
      >::type, enabler>::type = enabler ())
  {
    HTTP_TRACE_ENTER_CLS();
    on_connect_handler_ = std::move (handler);
    return *this;
  }
#else
  template <typename ConnectHandler>
  server& 
  on_connect (ConnectHandler const& handler, typename boost::enable_if<
      typename boost::is_same<
          typename boost::result_of<
            ConnectHandler (
              error_code, endpoint_type, endpoint_type, socket_type&
            )>::type, error_code
      >::type, enabler>::type = enabler ())
  {
    HTTP_TRACE_ENTER_CLS();
    on_connect_handler_ = boost::move (handler);
    return *this;
  }
#endif

  template <typename RequestPredicate, typename RequestHandler>
  server& on_request (RequestPredicate pred, RequestHandler handler)
  {
    HTTP_TRACE_ENTER_CLS();

  	handlers_.push_back (handler_vec::value_type (
  	 normalize_predicate<request_iterator> (pred), 
  	 handler // normalize_handler (handler)
  	));

#if 0
    // FIXME: debug
    HTTP_TRACE_NOTIFY("pred(/a/b/c/d/e)");
  	std::cout << pred (method::Get, boost::as_literal ("/a/b/c/d/e")) << "\n";
#endif
  	return *this;
  }

protected:
  void handle_accept (error_code const& ec, 
      endpoint_type const& local_ep, endpoint_type const& remote_ep,
      socket_type& sock)
  {
    HTTP_TRACE_ENTER_CLS();
    error_code connect_ec = on_connect_handler_ (ec, local_ep, remote_ep, sock);

    if (connect_ec)
    {
    	// print error, close connection
    }
    else
    {
    	// continue 
    }


  }

protected:
  template <typename H> struct on_connect_true_ {
  	H handler;
  	on_connect_true_ (H const& h) : handler (h) {}
#if __cplusplus >= 201103L
		on_connect_true_ (H&& h) : handler (std::move (h)) {}
#endif
  	error_code operator() (error_code const& ec, endpoint_type const& l, 
  	    endpoint_type const& r, socket_type& s) const
  	{
  		handler (ec, l, r, s);
  		return error_code ();
    }

  	error_code operator() (error_code const& ec, endpoint_type const& l, 
  	    endpoint_type const& r, socket_type& s) 
  	{
  		handler (ec, l, r, s);
  		return error_code ();
    }
  };

  static error_code on_connect_default (error_code const&, 
      endpoint_type const&, endpoint_type const&, socket_type&)
  {
  	return error_code ();
  }

private:
  manager_type manager_;
  pool_type pool_;
  handler_vec handlers_;

  compat::function<error_code (error_code const&, endpoint_type const&, 
      endpoint_type const&, socket_type&)> on_connect_handler_;
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
