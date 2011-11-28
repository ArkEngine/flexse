# -*- coding: utf-8 -*-
import socket, struct, sys, json

if len(sys.argv) != 2:
    print "python test.py query"
    sys.exit(1)

dd = {}
dd["termlist"] = []
term1 = {}
term1["term"] = "美女"
term1["weight"] = 30
term1["synonyms"] = ["美人"]
dd["termlist"].append(term1)
term2 = {}
term2["term"] = sys.argv[1]
term2["weight"] = 10
dd["termlist"].append(term2)
term3 = {}
term3["term"] = "烦恼"
term3["weight"] = 60
term3["synonyms"] = []
dd["termlist"].append(term3)
#filtlist = []
#f_type = {}
#f_type["method"] = 0
#f_type["field"] = "type"
#f_type["value"] = 1
#filtlist.append(f_type)
##f_duration = {}
##f_duration["method"] = 3
##f_duration["field"] = "reserved"
##f_duration["min"] = 4
##f_duration["max"] = 8
##filtlist.append(f_duration)
#dd["filtlist"] = filtlist
#
#ranklist = []
#r_type = {}
#r_type["method"] = 1
#r_type["field"] = "type"
#r_type["value"] = 4
#ranklist.append(r_type)
##r_duration = {}
##r_duration["method"] = 3
##r_duration["field"] = "duration"
##r_duration["min"] = 4
##r_duration["max"] = 8
##ranklist.append(r_duration)
#dd["ranklist"] = ranklist

jsonstr = json.dumps(dd)
print jsonstr
FMT_XHEAD = "I16sIIII"
sbuf = struct.pack(FMT_XHEAD, 1234, "pyclient", 0, 0, 0, len(jsonstr))
sbuf += jsonstr
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(("127.0.0.1", 1983));
sock.send(sbuf)
rbuf = sock.recv(36)
log_id, srvname, headid, headversion, status, detail_len = struct.unpack(FMT_XHEAD, rbuf)
print "log_id[%u] srvname[%s] detail_len[%u]" % (log_id, srvname, detail_len)
if detail_len > 0:
    rrbuf = sock.recv(detail_len)
    print struct.unpack("I"*(detail_len/4), rrbuf)
