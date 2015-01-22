#ifndef _HTTP_PREDICATES_H_
#define _HTTP_PREDICATES_H_
#include <http/traits/is_iterator_range.h>
#include <http/traits/is_url_placeholder.h>
#include <http/uri/parts.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/utility/enable_if.hpp>

#include <utility>

namespace http {
namespace predicates {

#if __cplusplus >= 201103L
template <typename T>
using as_literal_type = decltype (boost::as_literal (std::declval<T> ()));
#else
template <typename T>
struct as_literal_type
{
	typedef boost::iterator_range<boost::range_iterator<T> > type;
};

template <std::size_t N>
struct as_literal_type<char[N]>
{
	typedef boost::iterator_range<char const*> type;
};

template <>
struct as_literal_type<char*>
{
	typedef boost::iterator_range<char*> type;
};

template <>
struct as_literal_type<char const*>
{
	typedef boost::iterator_range<char const*> type;
};
#endif

template <typename ExtractorT, typename TestT>
class istarts_with_helper
{
public:
	istarts_with_helper (ExtractorT const& extract, TestT const& tst,
	    std::locale const& loc)
	  : extract_ (extract)
	  , test_ (tst)
	  , loc_ (loc)
	{
  }

  template <typename Iterator>
  bool operator() (method m, boost::iterator_range<Iterator> const& path, 
      uri::parts<Iterator> const& parsed) const
  {
  	return boost::istarts_with (extract_ (m, path, parsed), test_, loc_);
  }

private:
  ExtractorT extract_;
  TestT test_;
  std::locale loc_;
};

template <typename ExtractorT, typename TestT>
typename boost::enable_if_c<
         traits::is_url_placeholder<ExtractorT>::value 
      && traits::is_iterator_range<TestT>::value
  , istarts_with_helper<ExtractorT,typename as_literal_type<TestT>::type>
>::type
istarts_with (ExtractorT const& extract, TestT const& tst, 
  std::locale const& loc = std::locale ())
{
	return 
	  istarts_with_helper<
	      ExtractorT
	    , typename as_literal_type<TestT>::type
	  > (extract, boost::as_literal (tst), loc);
}

#if 0
template <typename ExtractorT, typename TestT, >
class istarts_with_helper
{
public:
	istarts_with_helper (ExtractorT const& extract, TestT const& tst,
	    std::locale const& loc)
	  : extract_ (extract)
	  , test_ (tst)
	  , loc_ (loc)
	{
  }

  template <typename RangeT>
  bool operator() (RangeT const& url) const
  {
  	return boost::istarts_with(extract_ (boost::as_literal (url)), test_, loc_);
  }

private:
  ExtractorT extract_;
  TestT test_;
  std::locale loc_;
};
#endif

}} // namespace http::predicates
#endif // _HTTP_PREDICATES_H_
