# Cinder-HttpRequest
ASIO - Scoped Async Http Request


This is a quick and dirty HTTP Request class based on ASIO for Cinder. 
Just a single line can make a request, and pass back the response in a 
std::function callback (including lambda functions.)

using a std::shared_ptr means the request will clean itself up after the 
request, close the socket connection and destroy the object which is 
nice for prototyping quick API REST calls.

The usage is like this:
'''
    AsynHttpRequest::create(io_service(), hostname)->BeginPost( endpoint, data, [this](std::string resp){

	}); 

'''

where 'io_service()' is from the Cinder app context, hostname is the address of the server, endpoint is the RESTful endpoint you are trying to hit. Data is a JsonTree.

The response resp is a string with the header removed. You only get back the body. If you need more than that for your project, this class is not for you!

The module currently supports GET, POST, PUT, and DELETE requests.