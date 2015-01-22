#ifndef _HTTP_DETAIL_MUTEX_H_
#define _HTTP_DETAIL_MUTEX_H_

#if __cplusplus < 201103L
# include <boost/thread.hpp>
#else
# include <mutex>
#endif

namespace http { namespace detail {

#if __cplusplus < 201103L
using boost::mutex;
using boost::lock_guard;
using boost::unique_lock;
#else
using std::mutex;
using std::lock_guard;
using std::unique_lock;
#endif

}}
#endif // _HTTP_DETAIL_MUTEX_H_
