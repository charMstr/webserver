# WEBSERV
## Project's Description:
This project was a pedagogic project. It was developed as part of our curicullum at 42 school by a team of three students.
We aimed at coding a small webserver, written in c++98 and architectured around the select() unix function, and the idea of asynchronism.

## How to use it:
### How to build:
````make````
### How to run:
- With a default configuration file (see define in includes/webserv.hpp):

````./webserv````

- With a provided configuration file:

````./webserv <configuration_file.conf>````

__Note:__ *file's extention must be ".conf"*
## Configuration file:
The configuration file has a synthax inspired/really similar to nginx's configuration files.
The main block and and upstream blocks dont exist though. Only the server blocks, and their nested
location blocks can be declared.

To declare a server block:

````server { }````

To declare a location block, within a server block:

````location <path> { }````

Within a server and a location block, any directive must start with a reserved
keyword and end with a semi-column: '````;````'.

### list of reserved keywords:
- server
- location
- server_name
- listen
- root
- index
- error_page
- cli_max_size
- allow
- autoindex
- alias
- cgi_pass

### __server__:
It is used to declare a server block, just like in an nginx
configuration file. Within a server block, many directives can be defined, and
many location blocks can be defined. Its syntax is:

````server { }````

### __location__:
It is used to declare a location block, just like in an nginx
configuration file. Within a location block, many directives can be defined.
Its syntax is:

````location { }````

### __alias__:
Just like in nginx, the alias directive tells to replace what is
defined in the location block with the path specified by the alias directive.
exemple: With a configuration like so:
````
server
{
	location /test1/
	{
		alias /var/www/static;
	}
}
````
A request made to localhost:8080/test1/salut.txt will end up in a file_path of:
/var/www/static/salut.txt ("/test1/" disapears).

### __root__:
Just like nginx, it can be found in a location block, or outside, in
a server block. The root directive is prepended to the uri.
exemple: With a configuration like so:
````
server
{
	location /test1/
	{
		root /var/www/static;
	}
}
````
A request made to localhost:8080/test1/salut.txt will end up in a file_path of:
/var/www/static/test1/salut.txt (use alias instead if you want "/test1/" to disapear from the resource's path)

__Note:__ *both the alias and the root directive, can be absolute path, in this case
no further change is made. If the path is relative, it is relative to the 
directory in which the webserv binary is launched.*

### __error_page__:
The error_page directive has the folowing synthax:

````error_page <list of ints> <file_name>;````

- <list of ints>: a list of at least one int, space separated.
- <file_name>: a unix style file_name (starting by a letter or underscore etc...)

__Note:__ *The "file_name" is concatenated with the basemame of the URI, if a status code
different than 200 and belonging in the "list of ints" figures in the response to client.*

__Note:__ *We cannot have two different "error_page" directive for the same location
block without overiding the first.*

### __index__:
The index directive has the following synthax:

````index <file_name>;````

The <file_name> will be used if the URI requested is a directory. The file_name
is then appended to the URI (+- separating '/').

### __allow__:
This directive lists the methods allowed for the server block, or a given location block.
Its synthax is:

````allow <list_of_methods>;````

- list_of_methods>: at least of method among (GET, HEAD, DELETE, OPTION, POST, PUT) space separated.

__Note:__ *By default, only the GET method (not HEAD!) is allowed in the server and the location
blocs.*

### __autoindex__:
This directive can be either omited, or set to "on" or "off".
It can be set in a server bloc, and in its location blocs.
Its syntax is:

````autoindex on | off;````

If set to on, and the method is GET, and the uri requested is actually a
directory, the listing of this directory is printed on screen to the client.

### __cgi_pass__:
This directive can be omited. It can be present at the server
block level, and/or within a location block level. Its purpose is to give a
file extension, and the binary that is launched for that file extension if a
client's request is a CGI request. (method is either GET or POST)
Its syntax is :

````cgi_pass <.file_extension> <cgi_script> <list_of_methods>;````

- <.file_extension>: a string that starts with a dot '.', and contains at least one lowercase
character from 'a' to 'z'. If <.file_extension> is only the character "*" it means any file is considered valid to trigger the cgi script.
- <cgi_script>: being the path to a cgi script to launch.
- <list_of_methods>: space separated list of methods valid in the request to trigger the cgi_script (either GET or POST).

### __listen__:
This directive lets us declare the port on which we are listening with our server bloc. It can only be declared in the server bloc.
Its syntax is :

````listen <int>;````

### __cli_max_size__:
This directive give a max size to a body in a request made to the server. It is location bloc specific.
By default cli_max_size is set to 0 and is ignored. (no max body size in requests!)
Its syntax is :

````cli_max_size <int>;````

### __server_name__:
This directive gives a name to the server, used in the response headers to client's requests.
Its syntax is :

````server_name <string>;````

## Things we explored:

### libregex and lexer:
A library of regex has been implementd to allow a a tokenisation very flexible. It is really easy to add a new configuration reserved keyword in the code.
A lexer takes care of tokenising according to the regex created.

### Avoid using dynamic casts:
For all the different requests methods, we created a matching class: "<xxx>_service", example: get_service.
Each class is derivated from the Service abstract base class. We have a linked list of pointers to this abstract base class, and we can use any element of that list, being totally agnostic of the real underlying object type. Thanks to the overiding of the pure virtual functions of the abstract base class (write_svc() and read_svc in our case), and to the fact that unix filedescriptors, we could implement some efficient polymorphism.

### asynchronism:
Through the use of select() and the non blocking fd, we discovered the realm of asynchronism.

We did not create multiple workers and multiple thread. We only used the asynchronism provided with the select() function.

### Keep alive on http:

## More Implementation Details (Architecture within cpp classes)

### Server class:
The class Server is used to instanciate one object that represents all our virtual servers at once. It holds a link list of virtual servers(generated from the configuration file) and will respond to any request attempting to reach one of those virtual servers, in an asynchronous way.

It is in charge of:

1. Accepting incoming connections on the listening sockets --> creating connection sockets.
2. Managing the objetcs generated for each connection sockets, reading on them in order to understand the clients requests. --> creating services.
3. Managing the objects representing a particular service for a client.

### Request_reader class:
An object of this class is instanciated each time a client tries to connect to one of our virtual servers. It holds to a connection socket, and keep reading
on it until it has enough information to generate a Service object. It will always keep reading on the connection socket, assuring the keep-alive feature of the HTTP/1.1 standard.

It is in charge of:
1. Reading on the connection socket until the status line and headers and body are fully yielded
2. When the status line and headers are read (full headers section until the double "\r\n\r\n") it will generate a Object derivated from the Service class.

### Service class:
This class is an abstract base class. We chose to create multiple classes for the different services provided by our server. A service means a response to a request of a certain method among GET, POST, HEAD, PUT, DELETE, OPTIONS, ...
This abstract base class provids different services (named <xxx>_service) with all the necessary elementary function. write_svc() and read_svc() are made pure virtual and are reimplemented in each specific services like get_service or cgi_service, or put_service for example.
With decided to not do any dynamic casts, we simply use pointer to the base class and the reimplementations of the virtual function from the base class accelerate the process, with the elasticity of encapsulating a response a to method in its own service class.

## HOW TO DEBUG

Any lines staring with DEBUG are part of a conditional macro. They will be hidden under a 0 for loop if __DEBUG__ is not defined at compile time.

For activating the debug, compile with:

````make re DEBUG=1````

From there any debuging funtion will be called (TRACE_LEVEL is defaulted to 0).

If you want to avoid some useless TRACE messages, set a higher value to
TRACE_LEVEL like so:

````make re DEBUG=1 TRACE_LEVEL=2````

__Note:__ *Refer to includes/debug_and_traces_macros.hpp*

If you want to take advantage of fsanitize or valgrind for example, set the
"flags" variable like so:

````make re flags=f````
