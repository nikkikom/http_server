#ifndef _HTTP_TRAITS_IS_URL_PLACEHOLDER_H_
#define _HTTP_TRAITS_IS_URL_PLACEHOLDER_H_
#include <http/placeholders.h>

#include <boost/mpl/bool.hpp>

namespace http { namespace traits {

namespace mpl = ::boost::mpl;

template <typename T>
struct is_url_placeholder
  : mpl::false_
{};

template <>
struct is_url_placeholder<placeholders::url::tag_user_info>: mpl::true_ {};

template <>
struct is_url_placeholder<placeholders::url::tag_port>: mpl::true_ {};

template <>
struct is_url_placeholder<placeholders::url::tag_path>: mpl::true_ {};

template <>
struct is_url_placeholder<placeholders::url::tag_scheme>: mpl::true_ {};

template <>
struct is_url_placeholder<placeholders::url::tag_query>: mpl::true_ {};

template <>
struct is_url_placeholder<placeholders::url::tag_fragment>: mpl::true_ {};

template <>
struct is_url_placeholder<placeholders::url::tag_query_key>: mpl::true_ {};

template <>
struct is_url_placeholder<placeholders::url::tag_query_value>: mpl::true_ {};

}} // http::traits
#endif // _HTTP_TRAITS_IS_URL_PLACEHOLDER_H_
