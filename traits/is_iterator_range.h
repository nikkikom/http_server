#ifndef _HTTP_TRAITS_IS_ITERATOR_RANGE_H_
#define _HTTP_TRAITS_IS_ITERATOR_RANGE_H_
#include <boost/mpl/bool.hpp>

namespace boost {
  template <typename Iterator> class iterator_range;
}

namespace http { namespace traits {
namespace mpl = ::boost::mpl;

template <typename T>
struct is_iterator_range
  : mpl::false_
{};

template <typename T>
struct is_iterator_range< ::boost::iterator_range<T> >
  : mpl::true_
{};

template <size_t N>
struct is_iterator_range<char[N]>
  : mpl::true_
{};

template <>
struct is_iterator_range<char*>
  : mpl::true_
{};

template <>
struct is_iterator_range<char const*>
  : mpl::true_
{};

}} // http::traits
#endif // _HTTP_TRAITS_IS_ITERATOR_RANGE_H_
