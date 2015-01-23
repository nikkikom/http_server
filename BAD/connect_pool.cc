#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/utility/addressof.hpp>
#include <boost/move/utility_core.hpp>
#include <boost/throw_exception.hpp>

#if __cplusplus >= 201103L
# define USE_STD_VECTOR 1
#endif

#include <sstream>

#if USE_STD_CONTAINER
# include <deque>
# define CONNECTION_CONTAINER std::deque
#else
# include <boost/container/deque.hpp>
# define CONNECTION_CONTAINER boost::container::deque
#endif

#define print_pretty_function() // std::cout << __PRETTY_FUNCTION__ << "\n\n"

namespace sys = ::boost::system;

template <class Connection, class Manager>
class pool
{
public:
  typedef Connection* connection_type;
  typedef Manager manager_type;

  pool (unsigned capacity = 1, manager_type const& m = manager_type ())
    : manager_ (m)
    , capacity_ (capacity)
  {
  	print_pretty_function ();

  	for (unsigned i=0; i < capacity; ++i)
  		connect_connection (manager_.create ());
  }

  connection_type acquire ()
  {
  	print_pretty_function ();
    connection_type tmp = ready_.front ();
    ready_.pop_front ();
    return tmp;
  }

  void release (connection_type c)
  {
  	print_pretty_function ();
  	ready_.push_back (c);
  }

protected:
  void connect_connection (connection_type conn)
  {
  	print_pretty_function ();

    boost::lock_guard<boost::mutex> lock (mux_);
	  closed_.push_back (conn);
  	++total_;
  	manager_.connect (conn, boost::bind (
  	    &pool::handle_create_connection, this, _1, conn
  	  )
  	);
  }

  void handle_create_connection (sys::error_code const& ec, connection_type conn)
  {
  	print_pretty_function ();

  	{
      boost::lock_guard<boost::mutex> lock (mux_);

      for (typename connection_container::iterator i = closed_.begin ();
          i != closed_.end (); ++i)
        if (boost::addressof (conn) == boost::addressof (*i))
        {
          closed_.erase (i);
          break;
        }

      if (!ec)
      {
        ready_.push_back (conn);
        return;
      }
    } // unlock


    std::cerr << "socket connect: " 
      << manager_.endpoint () 
      << ": " << ec.message () << "\n";

    if (manager_.is_open (conn)) manager_.close (conn);
    connect_connection (conn);
  }

private:
  boost::mutex mux_;
  manager_type manager_;
  unsigned int capacity_;
  unsigned int total_;

  typedef CONNECTION_CONTAINER<connection_type> connection_container;
  connection_container ready_;
  connection_container closed_;

};

#include <iostream>

class connectionX
{
private:
  BOOST_MOVABLE_BUT_NOT_COPYABLE(connectionX)

public:
  connectionX () 
  {
  	std::cout << "connectionX::connectionX ()\n";
  }
  ~connectionX () 
  {
  	std::cout << "connectionX::~connectionX ()\n";
  }

  connectionX (BOOST_RV_REF(connectionX) x) 
  {
  	std::cout << "connectionX::connectionX (connectionX&&)\n";
  }

  connectionX& operator= (BOOST_RV_REF(connectionX) x)
  {
  	std::cout << "connectionX::operator= (connectionX&&)\n";
  	return *this;
  }
};

namespace asio = ::boost::asio;
using asio::ip::tcp;

template <typename Socket, typename Handler>
class connector
{
private:
  Socket& sock_;
  tcp::endpoint ep_;
  Handler handler_;

public:
	connector (Socket& sock, tcp::endpoint ep, Handler handler)
	  : sock_ (sock)
	  , ep_ (boost::move (ep))
	  , handler_ (boost::move (handler))
	{
  	print_pretty_function ();
  }

  template <typename YHandler>
  void operator() (asio::basic_yield_context<YHandler> yield)
  {
  	print_pretty_function ();
  	sys::error_code ec;
  	sock_.async_connect (ep_, yield[ec]);

  	handler_ (ec, sock_);

#if 0
  	if (ec) 
  	{
  		std::ostringstream os;
  		os << "socket connect: " << ep_;
  		BOOST_THROW_EXCEPTION (sys::system_error (ec, os.str ()));
    }
#endif
  }
};

template <typename Socket, typename Handler>
connector<Socket,Handler> 
make_connector (Socket& sock, tcp::endpoint ep, Handler handler)
{
 	print_pretty_function ();
	return connector<Socket, Handler> (sock, boost::move (ep), boost::move (handler));
}


template <typename Socket>
struct socket_traits
{
	typedef typename Socket::endpoint_type endpoint_type;
};

template <typename Socket, typename Traits = socket_traits<Socket> >
class socket_manager 
{
private:
  asio::io_service& io_;
  tcp::endpoint ep_;

public:
  typedef Traits traits_type;
  typedef typename traits_type::endpoint_type endpoint_type;

  socket_manager (asio::io_service& io, tcp::endpoint ep) 
    : io_ (io)
    , ep_ (boost::move (ep))
  {}

  Socket* create ()
  {
  	print_pretty_function ();
    return new Socket (io_);
  }

  template <typename Handler>
  void connect (Socket* sock, Handler handler)
  {
  	print_pretty_function ();
    asio::spawn (sock->get_io_service (), make_connector (*sock, ep_, handler));
  }

  static sys::error_code close (Socket* sock)
  {
  	sys::error_code ec;
  	return sock->close (ec);
  }

  static bool is_open (Socket const* sock) { return sock->is_open (); }

  static bool ping (Socket& sock);
  static void destroy (Socket& sock);

  tcp::endpoint endpoint () const { return ep_; }
};

int main ()
{
	typedef tcp::socket connection;

  try {
    asio::io_service io;
    tcp::endpoint ep (asio::ip::address::from_string ("127.0.0.1"), 12345);
	  pool<connection, socket_manager<connection> > cpool (1, 
	    socket_manager<connection> (io, ep));

#if 0 
    asio::spawn (io, make_connector<connection> (io, ep));


    cpool.release (connection ());
    connection con = cpool.acquire ();
#endif

    io.run ();
  } 
  catch (std::exception const& e)
  {
  	std::cout << "Error: " << e.what () << "\n";
  	exit (1);
  }
}
