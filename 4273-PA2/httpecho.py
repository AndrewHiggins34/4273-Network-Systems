#!/usr/bin/env python

from socket import * 
import thread

serverPort = 8000
httpmsg = "HTTP/1.1 200 Document Follows\r\nContent-Type:text/html\r\nContent-Length:41\r\n\r\n<html><h1>Hello Data Communications!</h1>\n" 

def handler(clientsock,addr):
    while 1:
        data = clientsock.recv(1024)
        if not data: break
        clientsock.send(httpmsg)


if __name__ =='__main__':
        
    tcpsocket=socket(AF_INET,SOCK_STREAM)
    tcpsocket.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
    tcpsocket.bind(('',serverPort))
    tcpsocket.listen(5)

while 1:
    (clientsock,addr)=tcpsocket.accept()
    thread.start_new_thread(handler, (clientsock, addr))
