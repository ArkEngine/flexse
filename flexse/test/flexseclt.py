import socket, struct, sys, json

#if len(sys.argv) != 1:
#    print "python crash.py"
#    sys.exit(1)
#if int(sys.argv[1]) > int(sys.argv[2]) :
#    print "begin > end"
#    sys.exit(1)
#
FMT_XHEAD = "I16sIII"
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(("127.0.0.1", 1984));
dd = {}
dd['OPERATION'] = "INSERT"
dd['id'] = 1
dd['uid'] = 123
dd['title'] = "awesome search framwork "
dd['type'] = 9
dd['content'] = "this is a message "
dd['tags'] = ["nasty", "sexy", "pretty"]
jsonstr = json.dumps(dd)
sbuf = struct.pack(FMT_XHEAD, 123, "pyclient", 0, 0, len(jsonstr))
sbuf += jsonstr
sock.send(sbuf)
rbuf = sock.recv(32)
#    print struct.unpack(FMT_XHEAD, rbuf)
