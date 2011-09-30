import socket, struct, sys, json, creat_sign

def query(sock, strQuery) :
    FMT_XHEAD = "I16sIIII"
    dd = {}
    dd['QUERY'] = strQuery
    jsonstr = json.dumps(dd)
    sbuf = struct.pack(FMT_XHEAD, 1234, "pyclient", 0, 0, 0, len(jsonstr))
    sbuf += jsonstr
    sock.send(sbuf)
    rbuf = sock.recv(36)
    log_id, srvname, headid, headversion, status, detail_len = struct.unpack(FMT_XHEAD, rbuf)
    #print "log_id[%u] srvname[%s] detail_len[%u]" % (log_id, srvname, detail_len)
    if detail_len > 0:
        rrbuf = sock.recv(detail_len)
        return struct.unpack("I"*(detail_len/4), rrbuf)
    else:
        return ()

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print "python test.py query beginInnerID"
        sys.exit(1)
    strstr = "abcdefghijklmnopqrst"
    innerID = int(sys.argv[2])
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(("127.0.0.1", 1983));
    for x in range(1, 999999):
        for y in range(0,20):
            strQuery = "%s[%u]" % (strstr[y], x)
            postinglist = query(sock, strQuery)
            if (postinglist):
                prev = postinglist[0]
                for z in range(1, len(postinglist)):
                    assert postinglist[z] == prev - 1
                    prev = postinglist[z]
            else:
                (s1, s2) = creat_sign.creat_sign(strQuery, len(strQuery))
                print strQuery, (s2<<32) + s1, "------"
