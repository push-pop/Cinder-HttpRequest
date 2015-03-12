#pragma once

#include <iostream>
#include <string>
#include <ostream>
#include <functional>
#include <memory>
#include "asio/read.hpp"
#include "asio/asio.hpp"

#include "cinder/app/App.h"
#include "cinder/Json.h"

typedef	std::shared_ptr<class AsyncHttpRequest> ScopedAsyncHttpRequest;
class AsyncHttpRequest : public std::enable_shared_from_this<AsyncHttpRequest>
{
public:

	static ScopedAsyncHttpRequest create(asio::io_service &service, std::string hostname);

    void BeginGet(std::string path, std::function<void(std::string)>&& Callback = [](std::string s){});
    void BeginPost(std::string path, ci::JsonTree reqData = ci::JsonTree(), std::function<void(std::string)>&& Callback = [](std::string){});
	void BeginPut(std::string path, ci::JsonTree reqData = ci::JsonTree(), std::function<void(std::string)>&& Callback = [](std::string){});
    void BeginDelete(std::string path, ci::JsonTree reqData = ci::JsonTree(), std::function<void(std::string)>&& Callback = [](std::string){});


    
    AsyncHttpRequest(asio::io_service &service, std::string hostname);
	~AsyncHttpRequest();


private:
	
	void handle_resolve(const asio::error_code& err, asio::ip::tcp::resolver::iterator endpt_itr);
	void handle_connect(const asio::error_code& err, asio::ip::tcp::resolver::iterator endpt_itr);
	void handle_write(const asio::error_code& err);

	void handle_read_status_line(const asio::error_code& err, size_t bytesTransferred);
	void handle_read_headers(const asio::error_code& err, size_t bytesTransferred);
	void handle_read_content(const asio::error_code& err, size_t bytesTransferred	);

	void handle_error(const asio::error_code& err);

	std::function <void(std::string)> _callback;

    
	asio::ip::tcp::resolver _resolver;
	asio::ip::tcp::socket	_socket;
	asio::ip::tcp::endpoint	_endpoint;
	std::string				_hostname;
	asio::streambuf			_request;
	asio::streambuf			_response;


};
