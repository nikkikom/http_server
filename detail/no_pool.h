#ifndef _HTTP_DETAIL_NO_POOL_H_
#define _HTTP_DETAIL_NO_POOL_H_
namespace http { namespace detail {

struct no_pool 
{
	no_pool () {}
	template <typename A> no_pool (A&) {};
};

}} // namespace http::detail

#endif // _HTTP_DETAIL_NO_POOL_H_
