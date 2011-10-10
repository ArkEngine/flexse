import json, sys, time

def get_offset(offset_file):
    f=file(offset_file)
    qoffset = {}
    tmpstr = f.readline().strip().replace(" ", "");
    lst = tmpstr.split(':')
    qoffset[lst[0]] = int(lst[1])
    tmpstr = f.readline().strip().replace(" ", "");
    lst = tmpstr.split(':')
    qoffset[lst[0]] = int(lst[1])
    tmpstr = f.readline().strip().replace(" ", "");
    lst = tmpstr.split(':')
    qoffset[lst[0]] = int(lst[1])
    return qoffset

if len(sys.argv) != 3:
    print "python sender-monitor.py sender-running-dir check-interval-seconds"
    sys.exit(1)

sender_runing_dir = sys.argv[1]
check_interval_seconds = int(sys.argv[2])
if check_interval_seconds < 10 :
    print "check-interval-seconds tooo short, at least above 10s"
    sys.exit(1)
config_file = sender_runing_dir+"/conf/sender.config.json"
cur_queue_status={}
while True:
    try:
        json_config = json.load(file(config_file))
    except IOError:
        print "file[%s] NOT exist." % (config_file)
        sys.exit(1)

    for x in json_config["followers"]:
        offset_file = sender_runing_dir+"/offset/"+x['name']+".offset"
        new_qoffset = get_offset(offset_file)
        if x["enable"]:
            if cur_queue_status.has_key(x['name']):
                old_qoffset = cur_queue_status[x['name']]['qoffset']
                status_ok = cur_queue_status[x['name']]['status_ok']
                if new_qoffset == old_qoffset and status_ok == True:
                    print "queue[%s] update STOP." % (x['name'])
                    cur_queue_status[x['name']]['status_ok'] = False
                elif new_qoffset == old_qoffset and status_ok == False:
                    print "I know this guy in trouble now."
                elif new_qoffset != old_qoffset and status_ok == False:
                    print "queue[%s] update OK." % (x['name'])
                    cur_queue_status[x['name']]['qoffset'] = new_qoffset
                    cur_queue_status[x['name']]['status_ok'] = True
                elif new_qoffset != old_qoffset and status_ok == True:
                    print "I know this guy is OK."
                    cur_queue_status[x['name']]['qoffset'] = new_qoffset
            else:
                qstatus = {}
                qstatus["qoffset"] = new_qoffset
                qstatus["status_ok"]  = True
                cur_queue_status[x['name']] = qstatus
    time.sleep(check_interval_seconds)
