#ifndef _HTTP_COMPAT_H_
#define _HTTP_COMPAT_H_

#include <memory>

#if __cplusplus >= 201103L
# include <functional>

# define AUTO_PTR ::std::unique_ptr
# define _inline_ns inline
# define _noexcept noexcept
# define _constexpr constexpr
#else
# include <boost/function.hpp>

# define AUTO_PTR ::std::auto_ptr
# define _inline_ns
# define _noexcept
# define _constexpr
#endif

namespace http { namespace compat {

#if __cplusplus >= 201103L
using std::function;
#else
using boost::function;
#endif

}} // namespace http::compat

#endif // _HTTP_COMPAT_H_
