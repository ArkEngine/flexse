import socket, struct, sys, json

if len(sys.argv) != 4:
    print "python test.py ip port query"
    sys.exit(1)

FMT_XHEAD = "I16sIII"
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((sys.argv[1], int(sys.argv[2])));
dd = {}
dd['QUERY'] = sys.argv[3]
jsonstr = json.dumps(dd)
print jsonstr
sbuf = struct.pack(FMT_XHEAD, 1234, "pyclient", 0, 0, len(jsonstr))
sbuf += jsonstr
sock.send(sbuf)
rbuf = sock.recv(32)
log_id, srvname, headid, headversion, detail_len = struct.unpack(FMT_XHEAD, rbuf)
print "log_id[%u] srvname[%s] detail_len[%u]" % (log_id, srvname, detail_len)
if detail_len > 0:
    rrbuf = sock.recv(detail_len)
    print struct.unpack("I"*(detail_len/4), rrbuf)
