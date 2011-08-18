import socket, struct, sys, json

if len(sys.argv) != 4:
    print "python test.py ip port loopcount"
    sys.exit(1)

FMT_XHEAD = "I16sIII"
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((sys.argv[1], int(sys.argv[2])));
for x in range(int(sys.argv[3])):
    dd = {}
    dd['OPERATION'] = "INSERT"
    dd['DOC_ID'] = x
    dd['CONTENT'] = "this is test message. no[%u] jingle[%u] buffalo[%u]" % (x, x, x, )
    #dd['CONTENT'] = "buffalo[%u]" % (x, )
    jsonstr = json.dumps(dd)
    sbuf = struct.pack(FMT_XHEAD, x, "pyclient", 0, 0, len(jsonstr))
    sbuf += jsonstr
    sock.send(sbuf)
    rbuf = sock.recv(32)
#    print struct.unpack(FMT_XHEAD, rbuf)
