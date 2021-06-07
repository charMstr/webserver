HOW TO MAKE:

For activating the debugging, compile with:
	```make re DEBUG=1```
From there any debuging funtion will be called (TRACE_LEVEL is defaulted to 0).
If you want to avoid some useless TRACE messages, set a higher value to
TRACE_LEVEL like so:
	```make re DEBUG=1 TRACE_LEVEL=2```

if you want to take advantage of fsanitize or valgrind for example, set the
"flags" variable like so:
	```make re flags=f```


CONFIGURATION FILE:

The construct and the synthad are really similar to nginx's. The main block and 
and upstream blocks dont exist though. Only the server blocks, and they nested
location blocks are used.

To declare a server block: 
server { }

To declare a location block, within a server block:
location path { }

Within server and location blocks, any directive must start with a reserved
keyword and end with a semi-column ';'.

list of reserved keywords:
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

__server__: it is used to declare a server block, just like in an nginx
configuration file. Within a server block, many directives can be defined, and
many location blocks can be defined. syntax:
```server { }```

__location__: it is used to declare a location block, just like in an nginx
configuration file. Within a location block, many directives can be defined.
syntax:
```location { }```

__alias__: Just like in nginx, the alias directive tells to replace what is
defined in the location block with the path specified by the alias directive.
 ```exemple: With a configuration like so:
 server
 {
	location /test1/
	{
		alias /var/www/static;
	}
}
a request made to localhost:8080/test1/salut.txt will end up in a file_path of:
/var/www/static/salut.txt ("/test1/" disapears).```

__root__: just like nginx, it can be found in a location block, or outside, in
a server block. The root directive is prepended to the uri.
 ```exemple: With a configuration like so:
 server
 {
	location /test1/
	{
		root /var/www/static;
	}
}
a request made to localhost:8080/test1/salut.txt will end up in a file_path of:
/var/www/static/test1/salut.txt```

NOTE: both the alias and the root directive, can be absolute path, in this case
no further change is made. If the path is relative, it is relative to the 
directory in which the webserv binary is launched.

__error_page__: the error_page directive has the folowing synthax:
error_page "list of ints" file_name;
with:
"list of ints": a list of at least one int, space separated.
The "file_name" is only one string.
The "file_name" is concatenated with the basemame of the URI, if a status code
different than 200 and belonging in the "list of ints" is yielded.
note: we cannot have two different "error_page" directive for the same location
block without overiding the first.

__index__: the index directive has the following synthax:
index file_name;
The file_name will be used if the uri requested is a directory. The file_name
is then concatenated with the uri (+- separating '/').

__allow__: this directive lists the methods allowed for a given location block.
NOTE: by default, only the GET method is allowed in the server and the location
blocs.

__autoindex__: this directive can be either omited, or set to "on" or "off".
It can be set in a server bloc, and in its location blocs.
```syntax:
autoindex on | off;```
If set to on, and the method is GET, and the uri requested is actually a
directory, the listing of this directory is printed on screen to the client.

__cgi_pass__: this directive can be omited. It can be present at the server
block level, and/or within a location block level. Its purpose is to give a
file extension, and the binary that is launched for that file extension if a
client's request is a CGI request. (method is either GET or POST)
```syntax:
cgi_pass .<string> PATH METHOD;```
with .<string>: a string that starts with a dot '.', and at least one lowercase
character from 'a' to 'z'
with PATH: being a path to a file.
with METHOD: being one of the methods like GET, POST, HEAD...
NOTE: if .<string> is replaced with kleen star operator ('*"), any file name is
accecpted.


 //TODO(Mon 24/05/2021 at 07:16:33 
 - listen
 - cli_max_size
 - server_name
