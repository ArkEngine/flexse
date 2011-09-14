import socket, struct, sys, json

if len(sys.argv) != 4:
    print "python flexseclt.py begin end step"
    sys.exit(1)
if int(sys.argv[1]) > int(sys.argv[2]) :
    print "begin > end"
    sys.exit(1)

begin = int(sys.argv[1])
end   = int(sys.argv[2]) + 1
step  = int(sys.argv[3])

FMT_XHEAD = "I16sIII"
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(("127.0.0.1", 1984));
for x in range(begin, end):
    for k in range(0, 10):
        dd = {}
        dd['OPERATION'] = "INSERT"
        dd['id'] = x + (step * 10 + k) * ( end-1 )
        dd['uid'] = x
        dd['title'] = "awesome#%u# search#%u# framwork#%u#" % (x, x, x)
        dd['type'] = x
        dd['content'] = "a[%u] b[%u] c[%u] d[%u] e[%u] f[%u] g[%u] h[%u] i[%u] j[%u]"\
                " k[%u] l[%u] m[%u] n[%u] o[%u] p[%u] q[%u] r[%u] s[%u] t[%u]" % \
                (x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x,)
        dd['tags'] = ["nasty*"+str(x), "sexy*"+str(x), "pretty*"+str(x)]
        jsonstr = json.dumps(dd)
#        print jsonstr
        sbuf = struct.pack(FMT_XHEAD, 123, "pyclient", 0, 0, len(jsonstr))
        sbuf += jsonstr
        sock.send(sbuf)
        rbuf = sock.recv(32)
#        print struct.unpack(FMT_XHEAD, rbuf)
