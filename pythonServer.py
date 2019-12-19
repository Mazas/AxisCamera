#! /usr/bin/python
# a simple tcp server
import socket,os
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  
sock.bind(('192.168.20.245', 1025))  
sock.listen(5)  
while True:  
    connection,address = sock.accept() 
    print 'connection accepted' 
    buf = connection.recv(255)  
    print buf
    connection.send(buf)    		
    connection.close()
