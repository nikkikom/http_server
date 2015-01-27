#ifndef _HTTP_CONVERT_CORO_HELPERS_H_
#define _HTTP_CONVERT_CORO_HELPERS_H_
#include <http_server/asio.h>
#include <http_server/convert_callback_to_coro.h>

#if __cplusplus < 201103L
# include <boost/shared_ptr.hpp>
# include <boost/move/move.hpp>
#endif

#include <utility>



#if __cplusplus >= 201103L
template <typename CoroHandler>
detail::convert_callback_to_coro<CoroHandler>
convert_callback_to_coro (CoroHandler&& ch)
{
	return detail::convert_callback_to_coro<CoroHandler> (
	  std::forward<CoroHandler> (ch));
}
#else
template <typename CoroHandler>
detail::convert_callback_to_coro<CoroHandler>
convert_callback_to_coro (CoroHandler ch)
{
	return detail::convert_callback_to_coro<CoroHandler> (boost::move (ch));
}
#endif

} // namespace http
#endif // _HTTP_CONVERT_CORO_HELPERS_H_
