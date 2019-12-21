#! /usr/bin/python
# a simple tcp server
import socket,os
import sys
import pymongo
import calendar;
import time;

#client = pymongo.MongoClient("mongodb://axis:axis@cluster0-shard-00-00-ivdnn.mongodb.net:27017,cluster0-shard-00-01-ivdnn.mongodb.net:27017,cluster0-shard-00-02-ivdnn.mongodb.net:27017/test?ssl=true&replicaSet=Cluster0-shard-0&authSource=admin&retryWrites=true&w=majority")
client = pymongo.MongoClient("mongodb://axis:axis123@ds259377.mlab.com:59377/motion-events?retryWrites=false")

#db = client['motion-events']
db = client.get_default_database()
timestamp = db["events"]



sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  
#sock.bind(('192.168.20.245', 1025))  
sock.bind(('127.0.0.1',1025))
sock.listen(5)  
connection,address = sock.accept() 
print 'connection accepted'
while True:  
    buf = connection.recv(13)  
    print buf

    entry = {"timestamp": buf}  
    timestamp.insert_one(entry)
    connection.send(buf)    		
connection.close()