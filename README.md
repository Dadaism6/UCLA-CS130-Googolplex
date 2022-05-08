# Contributor Documentation
####  Team: *Group A - Googolplex*
####  Authors: *Chenda Duan, Wenjie Mo, Wensi Ai, Zhengtong Liu*

## Table of contents

1. [Objective](#objective)
2. [Source Code Structure](#source-code-structure) 
3. [Build, Test, Run](#build,-test,-run)

    a. [Build](#build)
    
    b. [Test](#test)
4. [File Explanation](#file-explanation)
5. [Everything about Request Handler](#everything-about-request-handler)

    a. [How the current workflow goes](#how-the-current-workflow-goes)

    b. [Existing request handler](#existing-request-handler)

    c. [How to add new request handler](#how-to-add-new-request-handler)
## Objective
The objective of this document is to explain everything about our project so that other contributors can contribute to part of the project. We will explain the general source code structure, how to build, test, and run the code, and how specifically our request handler is designed.

Note that this document serves as a draft for the README file in the repo.

## Source Code Structure
Here is the Folder layout of the source code, we ignored .git folder, build folder, and any intermediately generated folder/file is ignored for simplicty. Tests folders contains too many testing files, which is also ignored

```
📦googolplex
 ┣ 📂config
 ┃ ┗ 📜default.config
 ┣ 📂docker
 ┃ ┣ 📜Dockerfile
 ┃ ┣ 📜base.Dockerfile
 ┃ ┣ 📜cloudbuild.yaml
 ┃ ┗ 📜coverage.Dockerfile
 ┣ 📂include
 ┃ ┣ 📂http
 ┃ ┃ ┣ 📜header.h
 ┃ ┃ ┣ 📜reply.h
 ┃ ┃ ┣ 📜request.h
 ┃ ┃ ┗ 📜request_parser.h
 ┃ ┣ 📜config_arg.h
 ┃ ┣ 📜config_parser.h
 ┃ ┣ 📜content_type.h
 ┃ ┣ 📜log.h
 ┃ ┣ 📜request_handler.h
 ┃ ┣ 📜request_handler_echo.h
 ┃ ┣ 📜request_handler_not_found.h
 ┃ ┣ 📜request_handler_static.h
 ┃ ┣ 📜server.h
 ┃ ┗ 📜session.h
 ┣ 📂src
 ┃ ┣ 📂http
 ┃ ┃ ┣ 📜reply.cc
 ┃ ┃ ┗ 📜request_parser.cc
 ┃ ┣ 📜config_parser.cc
 ┃ ┣ 📜content_type.cc
 ┃ ┣ 📜log.cc
 ┃ ┣ 📜request_handler_echo.cc
 ┃ ┣ 📜request_handler_not_found.cc
 ┃ ┣ 📜request_handler_static.cc
 ┃ ┣ 📜server.cc
 ┃ ┣ 📜server_main.cc
 ┃ ┗ 📜session.cc
 ┣ 📂static
 ┃ ┣ 📂static1
 ┃ ┃ ┣ 📜bad_request.txt
 ┃ ┃ ┣ 📜expected.txt
 ┃ ┃ ┣ 📜expected_bad.txt
 ┃ ┃ ┣ 📜index.html
 ┃ ┃ ┣ 📜minion.jpg
 ┃ ┃ ┣ 📜minion.jpg.zip
 ┃ ┃ ┣ 📜no_arg_log.log
 ┃ ┃ ┣ 📜normal_log.log
 ┃ ┃ ┗ 📜not_found.html
 ┃ ┣ 📂static2
 ┃ ┃ ┣ 📜README.txt
 ┃ ┃ ┣ 📜index.html
 ┃ ┃ ┣ 📜spongebob.png
 ┃ ┃ ┗ 📜test.gif
 ┃ ┣ 📂static3
 ┃ ┃ ┗ 📜test.txt
 ┃ ┗ 📜.DS_Store
 ┣ 📂tests
 ┃ ┣ 📂config_files
 ┃ ┣ 📜config_parser_test.cc
 ┃ ┣ 📜integration.sh
 ┃ ┣ 📜request_handler_test.cc
 ┃ ┣ 📜server_test.cc
 ┃ ┗ 📜session_test.cc
 ┣ 📜CMakeLists.txt
 ┗ 📜README.md
```
## Build, Test, Run
### Build
Please refer to the following command for build
```bash
mkdir build
cd build
cmake ..
make
```
### Test
For running all the test, please use the following command
```bash
cd build
make test
```
For unit test with more detailed message, please run
```bash
cd tests
../build/bin/config_parser_test
../build/bin/request_handler_test
../build/bin/server_test
../build/bin/session_test
```
For integration test, please run
```bash
./tests/integration.sh
```
### Run
To run the code locallly, please use the following command
```bash
cd build
./bin/webserver ../config/default.config 
```
And then start a new terminal (within the same docker environment) and try
```bash
curl localhost:80/static1/minion.jpg --output ./tmp.jpg
```
You can use 
- **/static1** to access files under **./static/static1**
- **/static2** to access files under **./static/static2**
- **/static3** to access files under **./static/static3**
- **echo** for the echo handler
Please see the **./config/default.config** file for referencing

### Docker
If you want to test the docker locally, you can use the following code to build an image and run it.
```bash
docker build -f docker/base.Dockerfile -t base .
docker tag docker.io/library/base googolplex:base
docker build -f docker/Dockerfile -t my_image .
docker run --rm -p 8080:80 --name my_run my_image:latest
```


## File Explanation
In this section, I will explanation what each header file is doing.
### config_arg.h 
This header file defines a struct called **config_arg**, which contains
- string handler_type: which handler this address want to use, echo, static, etc.
- string location: url location, for example, /static means 127.0.0.1/static
- string root: local address that the url location is bind to.

The config parser will create a map that maps each valid url location to a config_arg struct. 

For short, it is a struct that carry information for each location binding in the config file.

It also contains a set of string specifying all the valid handler types (those that we implemented).

### config_parser.h
This is the same config_parser header file we used for assignment1. It defines 3 classes
- NginxConfigStatement: representation of a single config statement.
- NginxConfig: representation of the entire config, with many statements
- NginxConfigParser: Actual parser that parse a config file into NginxConfig

The most important function are the two functions
```cpp
bool Parse(std::istream* config_file, NginxConfig* config, int* port, std::map<std::string, config_arg> * addrmap);
bool Parse(const char* file_name, NginxConfig* config, int* port, std::map<std::string, config_arg> * addrmap);
```
We modified it so that it can catch the address binding information and store them into the addrmap map.

### content_type.h
It defines a stuct named **content_type**. 
- It has a map **content_type_map** that will map the file extension (eg. .txt) to the http response file type (eg. text/plain).
- It has a function *get_content_type* that will return corresponding content type.
### log.h
It defines 5 log severities: DEBUG, INFO, WARNING, ERROR, FATAL

It also include all the necessary log libraries.

### request_handler
It defines a class **request_handler** that defines the common interface of all the request_handler (all other handler class will inherit from this class).

The **request_handler**  defines a constructor that takes the url location and corresponding local address as input.
```cpp
request_handler(std::string location, std::string root)
```
The most important function for request_handler is the **handle_request** function that takes the request as input and set corresponding response, it returns a boolean status code indicating whether the handle process is succcess.
```cpp
bool handle_request(http::request<http::string_body> request, http::response<http::string_body>& response)
```

### request_handler_factory
This defines a **RequestHandlerFactory** class that will use polymorphism to create correct request_handler class. It has a constructor and a create method.
```cpp
RequestHandlerFactory(config_arg arg);
request_handler* create();
```
The constructor takes the config_arg as explained above and store it.
The **create** function use the stored config_arg to create corresponding request_handler and return its pointer.

It also defines a **HandlerType** enumeration. Currently, we support
- Echo_Handler
- Static_Handler
- Not_Found_Handler

### request_handler_echo
It defines a class **request_handler_echo** that will set the response body to the requst, setting the content type as text/plain, and send it back.

### request_handler_static
It defines a class **request_handler_static** that will find the corresponding local address of the request file, read it, set corresponding content type, and send it back.

### request_handler_not_found
It defines a class **request_handler_not_found** that will only be called if the corresponding handler is not found (not speicifed in the config file). It will return a 404 not found response.

### server.h
This file defines a **server** class
The **server** class is responsible for starting a server. It receives the **port** and **addrmap** parameter from server_main, call **create_dispatcher** to use **RequestHandlerFactory** to create corresponding request_handler for the given addrmap, and starts the server on the given port number. 
```cpp
server(boost::asio::io_service& io_service, short port, std::map<std::string, config_arg> addrmap);
```

It has three functions: start_accpet(), handle_accept(), and create_dispatcher()
```cpp
void start_accept();
bool handle_accept(session* new_session, const boost::system::error_code& error);
void create_dispatcher(std::map<std::string, config_arg> addrmap);
```
During the initialization, the server will call start_accept() to start a new session, and all the requests will be passed to the handle_accpet() function, which in turn will call session->start().

The **create_dispatcher** function will go over all the address-config pair in the addressmap and for each of them, create a corresponding request handler. It will set the map called routes, with key as url location and value being the pointer to the corresponding request handler.

### session.h

This file defines a **session** class
The **session** class is in charge of recursively handling reads and writes from the socket. Its constructor receives io service and the routes mapping created in the server class.
```cpp
session(boost::asio::io_service& io_service, std::map<std::string, RequestHandlerFactory*> routes);
```
When reading a new piece of data from the socket, the session calls the **handle_read()** function, with the error code and number of bytes transfered. 
```cpp
session(boost::asio::io_service& io_service, std::map<std::string, RequestHandlerFactory*> routes);
```
It will check the error code, and if success, if will in turn call
which will in turn call **get_reply()** function to generate a reply, it passes the request data and the length of the data.
```cpp
std::string get_reply(char* request_data, int data_len);
```
The **get_reply** will call **generate_response** function, pass the same parameters, get the http response, can return its string.

```cpp
http::response<http::string_body> session::generate_response(char* request_data, int data_len)
```

**generate_response** function takes the major charge of generating corresponding responses. It will call **parse_request** to check if the request is valid, call **search_addr_binding** to search for corresponding local address or corresponding request handler using longest common substring. And then return the responses.


## Everything about Request handler
### How the current workflow goes

Basically, the **server_main.cc** will first call the **config_parser**. 
```cpp
config_parser.Parse(argv[1], &config, &port, &addrmap);
```
The **config_parser** will read through the config file, and insert the location-config pair into the addrmap
```cpp
std::map<std::string, config_arg>* addrmap
```
As said above, **config_arg** is defined in **config_arg.h**, which contains 
```cpp
std::string handler_type;
std::string location;
std::string root;
```
For example, if we have following in the config file
```config
location /static1 StaticHandler {
    root ../static/static1;
}
```
Then, config_parser will insert a pair, with key "/static1", and config_arg that has handler_type = "StaticHandler", location = "/static1", and root = "root".

The **server_main.cc**, once gets the addrmap, will go over it and check to see if any of the handler is not valid.
```cpp
for (auto const& x : addrmap) 
{
    if(default_arg.valid_handler_types.find(x.second.handler_type) == default_arg.valid_handler_types.end())
    {
        valid_handler = false;
        error_msg += "handler type " + x.second.handler_type + " invalid; ";
    }
}
```

Then, it will pass the addrmap to the **server**.

The **server**, once being contruct, will call
```cpp
void server::create_dispatcher(std::map<std::string, config_arg> addrmap) 
{
    for (auto const& mapping : addrmap) {
        routes[mapping.first] = new RequestHandlerFactory(mapping.second);
    }
} 
```
To loop over all the pairs in the addrmap, create a new map called routes that has key to still be location, and value to be a **RequestHandlerFactory**. Note that during construction, **RequestHandlerFactory** will not create request handler, it will only create corresponding request handler when called "RequestHandlerFactory->create()".

The server will call sessions, the session will receive **routes** listed above. 
In the core function of the session **generate_response**, it will call the create function to generate the **request_handler**, and then delete it afer usage.
```cpp
request_handler_ = routes[location]->create();
request_handler_ -> set_client_ip(client_ip_);

if(request_handler_ -> handle_request(request, response))
    INFO << "Successfully handled the request, return response to client " << client_ip_ << "\n";
else
    INFO << "Something went wrong when handling the request, return response to client " << client_ip_ << "\n";
delete request_handler_;  // short life cycle request handler
```

### Existing request handler
As said above, there are 3 existing request handler, all inherit from the **request_handler** class. They share the same constructor
```cpp
request_handler(std::string location, std::string root) {prefix_ = location; dir_ = root; }
```
Where location is the url location, and root is the local binding address.
- request_handler_not_found: It will be call when no corresponding address can be found in the config file. It will simply set the http status to 404 not found, and return a html saying not found.
- request_handler_echo: It will be call when corresponding url is said in the config to be echo address. It will set the responding content to be the same as the request and send it back.
- request_handler_static: It will be call when corresponding url is said in the config to be static serving address. It will use the location and root receiving during construction, along with the request.target() to find local files. It found, it will read and set the response content. If not found, it will perform same operation as request_handler_not_found.

We also use the **RequestHandlerFactory** class to create corresponding request_handler. The core function is its create
```cpp
request_handler* RequestHandlerFactory::create() 
{
    INFO << "Create " << arg.handler_type << " handler\n";
    if (arg.handler_type == "StaticHandler")
        return new request_handler_static(arg.location, arg.root);
    if (arg.handler_type == "EchoHandler")
        return new request_handler_echo(arg.location, arg.root);
    INFO << "No matching handler called " << arg.handler_type << " . Use NotFound handler instead\n";
    return new request_handler_not_found(arg.location, arg.root);
}
```
### How to add new request handler

To add new request handler, you first need to modify **config_arg.h**, adding
```cpp
std::set<std::string> valid_handler_types = {"StaticHandler", "EchoHandler", "NotFoundHandler", "ToBeAddHandler};
```

Then, in the **request_handler_factory.cc**, you need to midify 
```cpp
request_handler* RequestHandlerFactory::create() 
{
    INFO << "Create " << arg.handler_type << " handler\n";
    if (arg.handler_type == "StaticHandler")
        return new request_handler_static(arg.location, arg.root);
    if (arg.handler_type == "EchoHandler")
        return new request_handler_echo(arg.location, arg.root);
    if (arg.handler_type == "ToBeAddHandler")
        return new request_handler_tobeadd(arg.location, arg.root);
    INFO << "No matching handler called " << arg.handler_type << " . Use NotFound handler instead\n";
    return new request_handler_not_found(arg.location, arg.root);
}
```
You also need to write your own request handler that inherit from the **request_handler** defined in **request_handler.h**, and include that in the **request_handler_factory.cc**

You also need to modify CMakeLists.txt to include your new .cc source file
```
add_library(request_handler src/request_handler_echo.cc src/request_handler_static.cc src/request_handler_not_found.cc src/request_handler_tobeadd.cc src/log.cc)

add_library(request_handler_factory src/request_handler_factory.cc src/request_handler_echo.cc src/request_handler_static.cc src/request_handler_not_found.cc src/request_handler_tobeadd.cc src/log.cc)
```


