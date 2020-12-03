***************

PA3 Andrew Higgins

Should begin inside the main folder
Compile the program using "make"
	executable will be created as proxy

Then execute the program with ./proxy <portNumber>
  ex: ./proxy 8000

Lastly on a seperate terminal connect to the proxy using telnet
	ex: telnet localhost 8000

At this point the client can make requests to the proxy to be sent
to the server of the client's choice.

This is about the extent of my program. What happens next is purely theoretical,
given that I was unable to implement what needed to be done.

Proxy only supports GET requests
Proxy only supports HTTP servers that exist.

After parsing Client request proxy will forward request to HTTP server
Proxy will then receive server response and return the reponse the client.

Proxy will store webpage information in local cache via buffer, for <timeout> duration
After timeout duration, cache pages will be wiped from memory

Proxy will also confer with blacklisted webpages to ensure the host with which the
client wishes to communicate has not been blacklisted.
