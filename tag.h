#ifndef _HTTP_TAG_H_
#define _HTTP_TAG_H_

namespace http {
namespace tag {

// usefull tag types

// directs to use SSL in listen_on
struct tls_t {};

// use coroutine user handlers
struct use_coro_t {};

// use callback-style user handlers
struct use_callbacks_t {};

namespace {
	tls_t const tls = tls_t ();
	tls_t const ssl = tls_t ();

	use_coro_t const use_coro = use_coro_t ();
	use_callbacks_t const use_callbacks = use_callbacks_t ();
}


}} // namespace http::tag
#endif // _HTTP_TAG_H_
