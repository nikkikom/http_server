#include <boost/range/as_literal.hpp>
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "server.h"
#include "placeholders.h"
#include "predicates.h"
#include "trace.h"

#include <boost/asio.hpp>

namespace sys = ::boost::system;
using sys::error_code;

namespace asio = ::boost::asio;
using asio::ip::tcp;

void on_connect (asio::yield_context const& yield, sys::error_code const& ec,
  tcp::endpoint const& local_ep, tcp::endpoint const& remote_ep,
  tcp::socket& sock) 
{
	std::cout << "connect from " << remote_ep << " to " << local_ep << "\n";
}

#if 0
struct on_connect2 {
	void operator() (asio::yield_context const& yield, sys::error_code const& ec,
	  tcp::endpoint const& local_ep, tcp::endpoint const& remote_ep,
	    tcp::socket& sock) const {}
};
#endif

// predicate example:
struct my_path
{
#if !defined (BOOST_RESULT_OF_USE_DECLTYPE)
	template <class> struct result {};
	template <class F, class Iterator> 
	struct result<F (http::method,boost::iterator_range<Iterator>)> 
	{ typedef bool type; };
#endif

	template <typename Iterator>
	bool operator() (http::method m, boost::iterator_range<Iterator> path)
	{
		return boost::istarts_with (path, boost::as_literal ("/a/b/c"));
  }
};

class my_handler
{
	template <typename SmartSock>
	class rollit : public boost::enable_shared_from_this<rollit<SmartSock> >
	{
  public:
    rollit (SmartSock sock) : sock_ (boost::move (sock)) {};
  
    void start ()
    {
      asio::async_read (*sock_, sbuf_,
        boost::bind (&rollit::handle_read, 
            this->shared_from_this (), _1, _2)
      );
    }

    void handle_read (sys::error_code ec, std::size_t bytes)
    {
    	sbuf_.commit (bytes);
    	std::istream is (&sbuf_);
    	std::string s;
    	is >> s;
    	std::cout << "READ: " << s << "\n";
    }

  private:
    SmartSock sock_;
    asio::streambuf sbuf_;
  };

public:
#if !defined (BOOST_RESULT_OF_USE_DECLTYPE)
	template <class> struct result {};
	template <class F, class Iterator, class SmartSock> 
	struct result<F (http::method,http::uri::parts<Iterator>,SmartSock)>
	{ typedef bool type; };
#endif

	template <typename Iterator, typename SmartSock>
	bool operator() (http::method m, http::uri::parts<Iterator> const& parsed, 
	    SmartSock sock) const
	{
#if 0
		if (! parsed.hier_part.path 
			  || ! boost::istarts_with (*parsed.hier_part.path, 
			    boost::as_literal ("/callback/")))
			return false;
#endif
    boost::make_shared<rollit<SmartSock> > (boost::move (sock))->start ();

		return true;
  }

};

struct request_handler
{
	// simple do-nothing handler for compile test purposes.
	void operator() () const {}
};

int main ()
{
	namespace predicates = ::http::predicates;
	namespace url = ::http::placeholders::url;
	namespace tag = ::http::tag;

	HTTP_TRACE_ENTER();

	asio::io_service io;
	::http::server<> server (io);

  HTTP_TRACE_NOTIFY("Starting server");
  server
    // ставим полезные проперти
    .set_keep_alive ()

    // слушаем порт tcp/1234
    .listen_on (tcp::endpoint (tcp::v4 (), 1234))

    // и 1235/TLS - TLS пока недоделан
    .listen_on (tcp::endpoint (tcp::v4 (), 1235), ::http::tag::tls)

    // при коннекте клиента к нам вызывается handler "on_connect", который может
    // логгировать, или вернуть true/false, или error_code. При возврате false
    // или непустого error_code, клиент отвергается, error_code логгируется.
    .on_connect (&on_connect)

#if 0
    // регистрируем обработчик запросов. В данном случае будет вызываться
    // "my_path" на каждый запрос, если он вернут true, то будет вызываться
    // второй аргумент = обработчик запроса.
    .on_request (
      my_path (),

      // обработчик запросов. получает метод и разобранный запрос. 
      // по умолчанию получает std::iostream (или совместимую) структуру.
      // NB !!! передача параметров в обработчики пока не реализована !!!
#if __cplusplus >= 201103L
      [] (/*::http::method method, auto const& parsed, std::iostream& io*/) { 
          std::cout << "FOUND\n"; 
          // io << "200 OK\r\n";
      }
#else
			request_handler ()
#endif
    )

#if __cplusplus >= 201301L
    // предикат может быть лямбдой или функтором. Есть несколько возможных форм
    // описания функтора (разные набор параметров). Например, если мы хотим
    // проверять только http method, то можно написать [] (http::method m) {...}
    // если хотим проверять разобранный запрос, то можно 
    // [] (http::method m, 
    //    /* boost::iterator_range<Iterator> */ auto const& path, 
    //    /* http::uri::parts<Iterator> */ auto const& parsed) {...}
    // В данном случае лябмдами будет неудобно, но см. пример выше с my_path.
    // Форматы допустимых параметров описаны в request_predicate.h
    .on_request (
      [] (http::method method) { return method == http::method::Get; },

      // такая версия обработчика получает asio socket, stream или ssl_stream.
      // tag::asio_stream указывает библиотеке использовать вызов с 
      // asio iostream.
      [] (/*::http::method method, auto const& parsed, 
          ::http::tag::asio_stream, auto& stream*/)
      { 
     	  // ...
      }
    )
#endif

#if 0 && __cplusplus >= 201103L
    // Вариант сигнатуры предиката для облегчения использования лямбд с
    // auto аргуметами, а так же для функтором с конфликтующими 
    // operator() (...). Пока не реализовано.
    .on_request (
      [] (predicates::call_tag::MethodAndPath, ::http::method method,
          auto const& path)
      {
     	  return false;
      },

      request_handler ()
    )
#endif

    // еще один вариант предиката. http::url::path здесь - экстрактор (описан в
    // placeholders.h). Предписывает извлечь path от запроса. istarts_with
    // сравнивает извлеченную часть с вторым аргументом (реализация в
    // predicates.h). Здесь реализован концепт только с одной функцией
    // istart_with, если понравится, то надо писать реализации для всех прочих 
    // предикатов сравнения, что несложно, но муторно.
    .on_request (
      predicates::istarts_with (url::path, "/a/b/c"),

#if __cplusplus >= 201301L
      [] (/*:http::method method, auto const& parsed, std::iostream& io*/) 
      { 
      	std::cout << "FOUND\n"; 
      }
#else
			request_handler ()
#endif
    )

#endif

    // Callback-style handler
    .on_request (
      // predicates::istarts_with (url::path, "/callback/"),
#if __cplusplus >= 201301L
      [] (http::method, auto const& parsed, auto sock_ptr)
      {
      	return true;
      }
#else
			my_handler ()
#endif
    )

    // вызывает io_service.run () со всеми вытекающими последствиями...
    // удобно для тестов или для однотредных корутиновых серверов. 
    .run ()
  ;
}

