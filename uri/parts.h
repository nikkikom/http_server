#ifndef _HTTP_URI_PARTS_H_
#define _HTTP_URI_PARTS_H_

#include <boost/range/iterator_range.hpp>
#include <boost/optional.hpp>

namespace http {
namespace uri {

template <class FwdIter>
struct hierarchical_part {
  boost::optional<boost::iterator_range<FwdIter> > user_info;
  boost::optional<boost::iterator_range<FwdIter> > host;
  boost::optional<boost::iterator_range<FwdIter> > port;
  boost::optional<boost::iterator_range<FwdIter> > path;
};

template <class FwdIter>
struct parts {
  boost::optional<boost::iterator_range<FwdIter> > scheme;
  hierarchical_part<FwdIter> hier_part;
  boost::optional<boost::iterator_range<FwdIter> > query;
  boost::optional<boost::iterator_range<FwdIter> > fragment;
};

} // namespace uri
} // namespace http
#endif // _HTTP_URI_PARTS_H_

