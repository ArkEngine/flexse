# -*- coding: utf-8 -*-
#socket server端
#获取socket构造及常量
import struct, socket, sys
if len(sys.argv) != 3 :
    print "python queueReceiver.py port longconnect"
    sys.exit(1)
#''代表服务器为localhost
myHost = ''
#在一个非保留端口号上进行监听
myPort = int(sys.argv[1])
longconnect = (0 != int(sys.argv[2]))

#设置一个TCP socket对象
sockobj = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sockobj.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
#绑定它至端口号
sockobj.bind((myHost, myPort))
#监听，允许100个连结
sockobj.listen(100)

#直到进程结束时才结束循环
while True:
    #等待下一个客户端连结
    connection, address = sockobj.accept()
    #连结是一个新的socket
    print 'Server connected by', address
    if not longconnect:
        connection.close()
