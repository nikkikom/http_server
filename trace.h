#ifndef _HTTP_TRACE_H_
#define _HTTP_TRACE_H_

#if !HTTP_TRACE

#define HTTP_TRACE_ENTER()

#define HTTP_TRACE_ENTER_EX(ex)

#define HTTP_TRACE_ENTER_CLS()

#define HTTP_TRACE_ENTER_CLS_EX(ex)

#define HTTP_TRACE_RETURN(VALUE)

#define HTTP_TRACE_NOTIFY(MSG)

#else // HTTP_ENABLE_TRACE

#include <string>
#include <sstream>

#ifndef HTTP_TRACE_STRM
# include <iostream>
# define HTTP_TRACE_STRM ::std::clog
#endif

#define HTTP_TRACE_ENTER() ::http::detail::call_tracer     \
    __call_tracer(__FILE__,__LINE__,__FUNCTION__,             \
    __PRETTY_FUNCTION__,(void*)0)

#define HTTP_TRACE_ENTER_EX(ex) ::http::detail::call_tracer     \
    __call_tracer(__FILE__,__LINE__,__FUNCTION__,             \
    __PRETTY_FUNCTION__,(void*)0,ex)

#define HTTP_TRACE_ENTER_CLS() ::http::detail::call_tracer \
    __call_tracer(__FILE__,__LINE__,__FUNCTION__,             \
    __PRETTY_FUNCTION__,this)

#define HTTP_TRACE_ENTER_CLS_EX(ex) ::http::detail::call_tracer \
    __call_tracer(__FILE__,__LINE__,__FUNCTION__,             \
    __PRETTY_FUNCTION__,this,ex)

#define HTTP_TRACE_RETURN(VALUE) __call_tracer.leave(__LINE__,VALUE)

#define HTTP_TRACE_NOTIFY(MSG) __call_tracer.notify(__LINE__,MSG)

#define HTTP_TRACE_RETURN(VALUE) __call_tracer.leave(__LINE__,VALUE)

#define HTTP_TRACE_NOTIFY(MSG) __call_tracer.notify(__LINE__,MSG)

namespace http {

namespace detail {

class call_tracer
{
	enum direction { dir_inside, dir_enter, dir_leave };

public:
  call_tracer (std::string const& file, int line, std::string const& func,
      std::string const& pretty, void* that,
      std::string const& extra = std::string ())
    // : file_ (file)
    : line_ (line)
    , func_ (func)
    , pretty_ (pretty)
    , that_ (that)
    , extra_ (extra)
  {
  	std::size_t found = file.rfind ('/');

  	if (found != file.npos)
    	file_ = file.substr (found+1);
    else
    	file_ = file;

  	print (dir_enter, "ENTER");
  }

  ~call_tracer ()
  {
  	print (dir_leave, "LEAVE");
  }

  template <typename T>
  void leave (int line, T const& value)
  {
  	std::ostringstream os;
  	os << "RETURN VALUE: " << value;
  	print (dir_leave, os.str (), line);
  }

  void notify (int line, std::string const& msg)
  {
  	print (dir_inside, std::string ("NOTIFY: ") + msg, line);
  }

protected:
  void print (direction d, std::string const& msg, int line = -1)
  {
  	switch (d) {
  		case dir_enter: HTTP_TRACE_STRM << "> "; break;
  		case dir_leave: HTTP_TRACE_STRM << "< "; break;
  		case dir_inside: HTTP_TRACE_STRM << "= "; break;
    }

    HTTP_TRACE_STRM << func_;

    if (that_ != (void*) 0) HTTP_TRACE_STRM << " %" << that_;

    HTTP_TRACE_STRM << " @" << file_ << ':';

    if (line >= 0) HTTP_TRACE_STRM << line << '/' << line_;
    else HTTP_TRACE_STRM << line_;

    HTTP_TRACE_STRM << ": " << msg;
    HTTP_TRACE_STRM << '\n';
  }

private:
  std::string file_;
  int         line_;
  std::string func_;
  std::string pretty_;
  void*       that_;
  std::string extra_;
};

}} // namespace http::detail
#endif // HTTP_ENABLE_TRACE

#endif // _HTTP_TRACE_H_
