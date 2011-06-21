import socket, struct, sys

if len(sys.argv) != 4:
    print "python test.py ip port loopcount"
    sys.exit(1)

FMT_XHEAD = "I16sIII"
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((sys.argv[1], int(sys.argv[2])));
for x in range(int(sys.argv[3])):
    sstr = "this is test message. no[%u]" % (x,)
    sbuf = struct.pack(FMT_XHEAD, x, "pyclient", 0, 0, len(sstr))
    sbuf += sstr
    sock.send(sbuf)
    rbuf = sock.recv(32)
    print struct.unpack(FMT_XHEAD, rbuf)
