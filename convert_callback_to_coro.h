#ifndef _HTTP_CONVERT_CALLBACK_TO_CORO_H_
#define _HTTP_CONVERT_CALLBACK_TO_CORO_H_
#include <http_server/asio.h>


#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/decay.hpp>
#include <boost/utility/enable_if.hpp>

#include <boost/bind.hpp>
#include <boost/move/move.hpp>
#include <boost/shared_ptr.hpp>

#include <utility>

namespace http {
namespace detail {

template <typename CoroHandler>
class convert_callback_to_coro_helper
{
public:
  typedef typename boost::decay<CoroHandler>::type _CoroHandler;

  typedef typename 
  	  boost::result_of<_CoroHandler (asio::yield_context)>::type result_type;

#if __cplusplus >= 201103L
	template <class CH, class = typename boost::enable_if<
	    boost::is_same<typename boost::decay<CH>::type, _CoroHandler>>::type>
	convert_callback_to_coro_helper (CH&& handler)
	  : handler_ (std::forward<CH>	(handler)) {}
#else
  convert_callback_to_coro_helper (_CoroHandler const& handler) : handler_ (handler) {}
#endif

  result_type operator() (asio::io_service& io)
  {
  	result_type ret;

  	boost::shared_ptr<asio::steady_timer> timer (new asio::steady_timer (
  	  io, asio::steady_timer::duration::max ()));
#if 0
  	  boost::make_shared<asio::steady_timer> (io, 
  	    asio::steady_timer::duration::max ());
#endif
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
    _CoroHandler handler_;
    convert_callback_to_coro_helper::result_type& ret_;

  public:
    typedef void result_type;

    convertor (boost::shared_ptr<asio::steady_timer> timer,
        _CoroHandler const& handler, convert_callback_to_coro_helper::result_type& ret) 
      : timer_ (timer)
      , handler_ (handler)
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
  _CoroHandler handler_;
};

} // namespace detail

#if __cplusplus >= 201103L
template <typename CoroHandler>
detail::convert_callback_to_coro_helper<typename boost::decay<CoroHandler>::type>
convert_callback_to_coro (CoroHandler&& ch)
{
	return detail::convert_callback_to_coro_helper<
	  typename boost::decay<CoroHandler>::type> (
	    std::forward<CoroHandler> (ch));
}
#else
template <typename CoroHandler>
detail::convert_callback_to_coro_helper<CoroHandler>
convert_callback_to_coro (CoroHandler const& ch)
{
	return detail::convert_callback_to_coro_helper<CoroHandler> (ch);
}
#endif

namespace detail {
template <typename Handler>
class convert_on_connect_to_coro_helper
{
public:
  typedef typename boost::decay<Handler>::type _Handler;

#if __cplusplus >= 201103L
  template <typename H, class = typename boost::enable_if<
    boost::is_same<typename boost::decay<H>::type, _Handler>>::type>
  convert_on_connect_to_coro_helper (H&& handler)
    : handler_ (std::forward<H> (handler))
  {
  }
#else
  convert_on_connect_to_coro_helper (_Handler const& handler)
    : handler_ (handler)
  {
  }
#endif

  template <typename Endpoint, typename SmartSock>
	typename boost::result_of<
	  _Handler (asio::yield_context, error_code, Endpoint, Endpoint, SmartSock)
	>::type operator() (error_code const& ec, Endpoint const& local,
	                        Endpoint const remote, SmartSock sock)
	{
		using boost::cref;

	  return convert_callback_to_coro (
	    boost::bind (handler_, _1, cref (ec), cref (local), cref (remote), sock)
	  ) (sock->get_io_service ());
  } 

private:
  _Handler handler_;
};
} // namespace detail

#if __cplusplus >= 201103L
template <typename CoroHandler>
detail::convert_on_connect_to_coro_helper<
  typename boost::decay<CoroHandler>::type>
convert_on_connect_to_coro (CoroHandler&& ch)
{
	return detail::convert_on_connect_to_coro_helper<
	    typename boost::decay<CoroHandler>::type> (
	    std::forward<CoroHandler> (ch));
}
#else
template <typename CoroHandler>
detail::convert_on_connect_to_coro_helper<CoroHandler>
convert_on_connect_to_coro (CoroHandler const& ch)
{
	return detail::convert_on_connect_to_coro_helper<CoroHandler> (ch);
}
#endif

} // namespace http
#endif // _HTTP_CONVERT_CALLBACK_TO_CORO_H_
