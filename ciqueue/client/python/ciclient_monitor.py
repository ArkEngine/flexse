# 检测消息队列服务器进程是否存在
import json, struct, sys, socket, random, ciclient_config

class ciclient:
    def __init__(self):
        self.ServerList = ciclient_config.ciqueue_server_list
        self.sock = None
    def _rand_connect(self):
		si = random.randint(0, len(self.ServerList))
		rand_server_list = self.ServerList[si:] + self.ServerList[:si]
		for serv in rand_server_list:
			if self._connect(serv["Host"], serv["Port"]):
				return True
		return False
    def _connect(self, strHost, intPort):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.settimeout(0.1)
        try:
            self.sock.connect((strHost, intPort));
            self.sock.settimeout(1)
            return True
        except socket.timeout:
            print "connect TimeOut"
            self.sock.close()
            self.sock = None
            return False
        except socket.error, arg:
            (errno, err_msg) = arg
            self.sock.close()
            self.sock = None
            print "Connect server failed: %s, errno=%d" % (err_msg, errno)
            return False

    def _check_connect(self):
        rstr="a"
        if self.sock == None:
            return self._rand_connect()
        else :
            try:
                self.sock.setblocking(False)
                rstr= self.sock.recv(1)
                if rstr == "" or len(rstr) == 1:
                    print "remote server closed or left data len[%u]" % (len(rstr),)
                    self.sock.close()
                    self.sock = None
                    return self._rand_connect()
            except socket.error, e:
                self.sock.settimeout(1)
                print 'rstr[%s] len[%u] socket:%s' % (rstr, len(rstr), e,)
                return True

    def monitor(self, log_id):
        FMT_XHEAD = "I16sIII"
        sbuf = struct.pack(FMT_XHEAD, log_id, "pymonitor", 0, 0, 0)
        if self._check_connect():
            try:
                self.sock.send(sbuf)
            except socket.error, arg:
                print "error message [%s]" % (arg, )
                return False
            rbuf = self.sock.recv(32)
            log_id, srvname, version, reserved, detail_len = struct.unpack(FMT_XHEAD, rbuf)
            print "logid[%u] srvname[%s] version[%u] reserved[%u] detail_len[%u]" % (log_id, \
                    srvname, version, reserved, detail_len)
            return reserved == 0
        else:
            return False

if __name__ == "__main__":
    mycc = ciclient()
    for x in range(int(sys.argv[1])):
        mycc.monitor(x)
