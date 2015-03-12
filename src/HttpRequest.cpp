#include "HttpRequest.h"


using namespace asio::ip;
using namespace std;
using namespace ci;
using namespace ci::app;

ScopedAsyncHttpRequest AsyncHttpRequest::create(asio::io_service &service, std::string hostname)
{
	return std::make_shared<AsyncHttpRequest>(service, hostname);
}

AsyncHttpRequest::AsyncHttpRequest(asio::io_service &service, std::string hostname)
: _resolver(service), _socket(service), _hostname(hostname)
{

}

AsyncHttpRequest::~AsyncHttpRequest()
{
    
}

void AsyncHttpRequest::BeginGet(std::string path, std::function<void(std::string)>&& Callback)
{

	tcp::resolver::query	query(_hostname, "http");

	std::ostream request_stream(&_request);
	request_stream << "GET " << path << " HTTP/1.1\r\n";
	request_stream << "Host: " << _hostname << "\r\n";
	request_stream << "Accept: */*\r\n";
	request_stream << "Connection: close\r\n\r\n";

	_callback = Callback;
	_resolver.async_resolve(query,
		std::bind(&AsyncHttpRequest::handle_resolve, shared_from_this(), std::placeholders::_1, std::placeholders::_2));




}

void AsyncHttpRequest::BeginPost(std::string path, JsonTree reqData, std::function<void(std::string)>&& Callback)
{
	tcp::resolver::query query(_hostname, "http");

    string paramstr = reqData.serialize();

	std::ostream request_stream(&_request);
	request_stream << "POST " << path << " HTTP/1.1\r\n";
	request_stream << "Host: " << _hostname << "\r\n";
	request_stream << "Accept: */*\r\n";
	request_stream << "Connection: close\r\n";
	request_stream << "Content-Length: " << paramstr.size() << "\r\n\r\n";
	request_stream << paramstr;
    
    //console() << request_stream.rdbuf();

	_callback = Callback;
	_resolver.async_resolve(query,
		std::bind(&AsyncHttpRequest::handle_resolve, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void AsyncHttpRequest::BeginPut(std::string path, JsonTree reqData, std::function<void(std::string)>&& Callback)
{
	std::string paramstr = reqData.serialize();
	tcp::resolver::query	query(_hostname, "http");

	std::ostream request_stream(&_request);
	request_stream << "PUT " << path << " HTTP/1.1\r\n";
	request_stream << "Host: " << _hostname << "\r\n";
	request_stream << "Accept: */*\r\n";
	request_stream << "Connection: close\r\n";
	request_stream << "Content-Length: " << paramstr.size() << "\r\n\r\n";
	request_stream << paramstr;

	_callback = Callback;
	_resolver.async_resolve(query,
		std::bind(&AsyncHttpRequest::handle_resolve, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void AsyncHttpRequest::BeginDelete(std::string path, JsonTree reqData, std::function<void(std::string)>&& Callback)
{
	_callback = Callback;
    
    std::string paramstr = reqData.serialize();
    tcp::resolver::query	query(_hostname, "http");
    
    std::ostream request_stream(&_request);
    request_stream << "DELETE " << path << " HTTP/1.1\r\n";
    request_stream << "Host: " << _hostname << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n";
    request_stream << "Content-Length: " << paramstr.size() << "\r\n\r\n";
    request_stream << paramstr;
    
    _resolver.async_resolve(query,
            std::bind(&AsyncHttpRequest::handle_resolve, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void AsyncHttpRequest::handle_resolve(const asio::error_code& err, asio::ip::tcp::resolver::iterator endpt_itr)
{
	if (!err)
	{
		_endpoint = *endpt_itr;
		try{
			_socket.async_connect(_endpoint,
				std::bind(&AsyncHttpRequest::handle_connect, shared_from_this(),
				std::placeholders::_1, ++endpt_itr));
		}
		catch (Exception e)
		{
            
		}
	}
	else
	{
		handle_error(err);
	}
}

void AsyncHttpRequest::handle_connect(const asio::error_code& err, asio::ip::tcp::resolver::iterator endpt_itr)
{
	if (!err) {
		asio::async_write(_socket, _request, std::bind(&AsyncHttpRequest::handle_write, shared_from_this(), std::placeholders::_1));
	}
	else {
		handle_error(err);
	}
}

void AsyncHttpRequest::handle_write(const asio::error_code& err)
{
	if (!err)
	{
		// Read the response status line. The response_ streambuf will
		// automatically grow to accommodate the entire line. The growth may be
		// limited by passing a maximum size to the streambuf constructor.

		asio::async_read_until(_socket, _response, "\r\n",
			std::bind(&AsyncHttpRequest::handle_read_status_line, shared_from_this(),
			std::placeholders::_1, std::placeholders::_2));
	}
	else
	{
		handle_error(err);
	}
}

void AsyncHttpRequest::handle_read_status_line(const asio::error_code& err, size_t bytesTransfered)
{
	if (!err)
	{
		// Check that response is OK.
		std::istream response_stream(&_response);
		std::string http_version;
		response_stream >> http_version;
		unsigned int status_code;
		response_stream >> status_code;
		std::string status_message;
		std::getline(response_stream, status_message);
		if (!response_stream || http_version.substr(0, 5) != "HTTP/")
		{
			return;
		}

		if (status_code != 200){
			std::stringstream ss;
			ss << "Response returned with status code ";
			ss << status_code << endl;
//			console() << ss.str();
            _callback(ss.str());
			return;
		}


		// Read the response headers, which are terminated by a blank line.
		asio::async_read_until(_socket, _response, "\r\n\r\n",
			std::bind(&AsyncHttpRequest::handle_read_headers, shared_from_this(),
			std::placeholders::_1, std::placeholders::_2));
	}
	else
	{
		handle_error(err);
	}
}

void AsyncHttpRequest::handle_read_headers(const asio::error_code& err, size_t bytesTransferred)
{
	if (!err)
	{
		// Process the response headers.
		std::istream response_stream(&_response);
		std::string header;

		// Start reading remaining data until EOF.
		asio::async_read(_socket, _response,
			asio::transfer_at_least(1),
			std::bind(&AsyncHttpRequest::handle_read_content, shared_from_this(),
			std::placeholders::_1, std::placeholders::_2));
	}
	else
	{
		handle_error(err);
	}
}

void AsyncHttpRequest::handle_read_content(const asio::error_code& err, size_t bytesTransferred)
{
	if (!err)
	{

		// Continue reading remaining data until EOF.
		asio::async_read(_socket, _response,
			asio::transfer_at_least(1),
			std::bind(&AsyncHttpRequest::handle_read_content, shared_from_this(),
			std::placeholders::_1, std::placeholders::_2));
	}
	else if (err != asio::error::eof)
	{

		//Make sure socket closes
        handle_error(err);
	}
	else
	{
		//Make sure socket closes / call handler
		std::istream response_stream(&_response);
		std::stringstream ss;
		ss << response_stream.rdbuf();

        
		_callback(ss.str());

	}
}

void AsyncHttpRequest::handle_error(const asio::error_code& err)
{
	std::stringstream ss;
	ss << "Client Error Caught By Handle_Resolve: " << err;
	
	_socket.close();
	_callback(ss.str());
}
