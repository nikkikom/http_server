#ifndef _HTTP_CONVERT_CALLBACK_TO_CORO_H_
#define _HTTP_CONVERT_CALLBACK_TO_CORO_H_
#include <http_server/asio.h>

#if __cplusplus < 201103L
# include <boost/shared_ptr.hpp>
# include <boost/move/move.hpp>
#endif

#include <boost/bind.hpp>
# include <boost/move/move.hpp>

#include <utility>

namespace http {
namespace detail {

template <typename CoroHandler>
class convert_callback_to_coro_helper
{
public:
  typedef typename 
  	  boost::result_of<CoroHandler (asio::yield_context)>::type result_type;

#if __cplusplus < 201103L
  convert_callback_to_coro_helper (CoroHandler handler)
    : handler_ (boost::move (handler)) {}
#else
	convert_callback_to_coro_helper (CoroHandler handler)  // FIXME
	  : handler_ (std::move	(handler)) {}
#endif

  result_type operator() (asio::io_service& io)
  {
  	result_type ret;

  	boost::shared_ptr<asio::steady_timer> timer = 
  	  boost::make_shared<asio::steady_timer> (io, 
  	    asio::steady_timer::duration::max ());

  	asio::spawn (io, convertor (timer, handler_, ret));

  	error_code ec;
  	timer->wait (ec);

  	return ret;
  }

protected:
  class convertor 
  {
  private:
    boost::shared_ptr<asio::steady_timer> timer_;
    CoroHandler handler_;
    result_type& ret_;

  public:
    convertor (boost::shared_ptr<asio::steady_timer> timer,
        CoroHandler handler, result_type& ret) 
      : timer_ (timer)
      , handler_ (boost::move (handler))
      , ret_ (ret)
    {
    }

    void operator() (asio::yield_context const& yield) 
    {
      ret_ = handler_ (yield);
      timer_->cancel ();
    }
  };

private:
  CoroHandler handler_;
};

} // namespace detail

#if __cplusplus >= 201103L
template <typename CoroHandler>
detail::convert_callback_to_coro_helper<CoroHandler>
convert_callback_to_coro (CoroHandler&& ch)
{
	return detail::convert_callback_to_coro_helper<CoroHandler> (
	  std::forward<CoroHandler> (ch));
}
#else
template <typename CoroHandler>
detail::convert_callback_to_coro_helper<CoroHandler>
convert_callback_to_coro (CoroHandler ch)
{
	return detail::convert_callback_to_coro_helper<CoroHandler> (
	    boost::move (ch));
}
#endif

namespace detail {
template <typename Handler>
class convert_on_connect_to_coro_helper
{
public:
  convert_on_connect_to_coro_helper (Handler handler)
    : handler_ (boost::move (handler))
  {
  }

  template <typename Endpoint, typename SmartSock>
	typename boost::result_of<
	  Handler (asio::yield_context, error_code, Endpoint, Endpoint, SmartSock)
	>::type operator() (error_code const& ec, Endpoint const& local,
	                        Endpoint const remote, SmartSock sock)
	{
		using boost::cref;

	  return convert_callback_to_coro (
	    boost::bind (handler_, _1, cref (ec), cref (local), cref (remote), sock)
	  ) (sock->get_io_service ());
  } 

private:
  Handler handler_;
};
}

#if __cplusplus >= 201103L
template <typename CoroHandler>
detail::convert_on_connect_to_coro_helper<CoroHandler>
convert_on_connect_to_coro (CoroHandler&& ch)
{
	return detail::convert_on_connect_to_coro_helper<CoroHandler> (
	    std::forward<CoroHandler> (ch));
}
#else
template <typename CoroHandler>
detail::convert_on_connect_to_coro_helper<CoroHandler>
convert_on_connect_to_coro (CoroHandler ch)
{
	return detail::convert_on_connect_to_coro_helper<CoroHandler> (
	    boost::move (ch));
}
#endif



} // namespace http
#endif // _HTTP_CONVERT_CALLBACK_TO_CORO_H_
