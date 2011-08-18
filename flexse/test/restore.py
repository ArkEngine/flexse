import socket, struct, sys, json

if len(sys.argv) != 5:
    print "python restore.py ip port begin end"
    sys.exit(1)

FMT_XHEAD = "I16sIII"
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((sys.argv[1], int(sys.argv[2])));

dd = {}
dd['OPERATION'] = "RESTORE"
dd['id_list'] = range(int(sys.argv[3]), 1 + int(sys.argv[4]))
jsonstr = json.dumps(dd)
sbuf = struct.pack(FMT_XHEAD, int(sys.argv[4]), "pyclient", 0, 0, len(jsonstr))
sbuf += jsonstr
sock.send(sbuf)
rbuf = sock.recv(32)
print struct.unpack(FMT_XHEAD, rbuf)
