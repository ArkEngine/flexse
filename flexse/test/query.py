import socket, struct, sys, json

if len(sys.argv) != 2:
    print "python test.py query"
    sys.exit(1)

FMT_XHEAD = "I16sIIII"
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(("127.0.0.1", 1983));
dd = {}
dd['QUERY'] = sys.argv[1]
jsonstr = json.dumps(dd)
print jsonstr
sbuf = struct.pack(FMT_XHEAD, 1234, "pyclient", 0, 0, 0, len(jsonstr))
sbuf += jsonstr
sock.send(sbuf)
rbuf = sock.recv(36)
log_id, srvname, headid, headversion, status, detail_len = struct.unpack(FMT_XHEAD, rbuf)
print "log_id[%u] srvname[%s] detail_len[%u]" % (log_id, srvname, detail_len)
if detail_len > 0:
    rrbuf = sock.recv(detail_len)
    print struct.unpack("I"*(detail_len/4), rrbuf)
