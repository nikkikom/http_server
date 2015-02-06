#ifndef _HTTP_DETAIL_CONVERT_CORO_HELPERS_H_
#define _HTTP_DETAIL_CONVERT_CORO_HELPERS_H_
#include <http_server/asio.h>
#include <http_server/detail/convert_callback_to_coro.h>

#if __cplusplus < 201103L
# include <boost/shared_ptr.hpp>
# include <boost/move/move.hpp>
#endif

#include <utility>

namespace http {
namespace detail {
#if __cplusplus >= 201103L
template <typename CoroHandler>
convert_callback_to_coro<CoroHandler>
convert_callback_to_coro (CoroHandler&& ch)
{
	return convert_callback_to_coro<CoroHandler> (
	  std::forward<CoroHandler> (ch));
}
#else
template <typename CoroHandler>
convert_callback_to_coro<CoroHandler>
convert_callback_to_coro (CoroHandler const& ch)
{
	return convert_callback_to_coro<CoroHandler> (ch);
}
#endif

} // namespace detail
} // namespace http
#endif // _HTTP_DETAIL_CONVERT_CORO_HELPERS_H_
