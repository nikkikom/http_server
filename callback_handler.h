#ifndef _HTTP_CALLBACK_HANDLER_H_
#define _HTTP_CALLBACK_HANDLER_H_
#include <http_server/method.h>
#include <http_server/uri/parts.h>

#include <boost/utility/enable_if.hpp>
#include <boost/utility/result_of.hpp>
#include <boost/type_traits.hpp>
#include <boost/range.hpp>

#if __cplusplus < 201103L
# include <boost/shared_ptr.hpp>
#else
# include <memory> // unique_ptr
#endif

# include <boost/move/move.hpp>
#include <utility>

namespace http {

namespace detail {

template <typename Handler>
class callback_handler
{
public:
#if __cplusplus < 201103L
  callback_handler (Handler handler) : handler_ (boost::move (handler)) {}
#else
	callback_handler (Handler const& handler) : handler_ (handler) {}
	callback_handler (Handler&& handler) : handler_ (std::move	(handler)) {}
#endif

  template <typename Iterator, typename SmartSocket>
  bool operator() (HttpMethod m, uri::parts<Iterator> parsed,
                   SmartSocket sock) const
  {
		return handler_ (m, parsed, sock);
  }

private:
  Handler handler_;
};

} // namespace detail

template <typename Iterator, typename SmartSocket, typename Handler>
detail::callback_handler<Handler>
normalize_handler (Handler handler, typename boost::enable_if<
    boost::is_same<
        typename boost::result_of<Handler (
          http::HttpMethod, uri::parts<Iterator>, SmartSocket
        )>::type
      , bool
    >
  >::type* = 0)
{
	return detail::callback_handler<Handler> (handler);
}

} // namespace http
#endif // _HTTP_CALLBACK_HANDLER_H_
