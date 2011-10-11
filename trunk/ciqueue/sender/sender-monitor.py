# -*- coding:utf-8 -*-
# sender程序的监控脚本
# 仅对配置中enable为1的消息通道进行监控
# 当一个消息通道由正常变为异常时，会发送一个警报
# 当一个消息通道由异常变为正常时，会发送一个喜报
# 可能存在的问题，当消息队列超过检查间隔期间没有收到任何消息时，会误报
import json, sys, time

def get_offset(offset_file):
    # 读取offset文件夹下的进度文件
    # 以键值对的方式放入一个字典中并返回
    # 字典内容如下
    # {'offset': 833523976, 'block_id': 1615361, 'file_no': 4}
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
    # 每次重新读取配置文件
    try:
        json_config = json.load(file(config_file))
    except IOError:
        print "file[%s] NOT exist." % (config_file)
        sys.exit(1)

    for x in json_config["followers"]:
        # 对每个消息通道进行检查
        if x["enable"]:
            # 如果这个通道是开启的，则开始检查进度文件
            offset_file = sender_runing_dir+"/offset/"+x['name']+".offset"
            new_qoffset = get_offset(offset_file)
            if cur_queue_status.has_key(x['name']):
                old_qoffset = cur_queue_status[x['name']]['qoffset']
                status_ok = cur_queue_status[x['name']]['status_ok']
                if new_qoffset == old_qoffset and status_ok == True:
                    # 正常状态 变成 异常状态，需要发送警报
                    print "queue[%s] update STOP. remote[%s:%u]" % (x['name'], x['host'], x['port'],)
                    cur_queue_status[x['name']]['status_ok'] = False
                elif new_qoffset == old_qoffset and status_ok == False:
                    # 这个不需要报警了
                    print "I know this guy[%s - %s:%u] in trouble now."  % (x['name'], x['host'], x['port'],)
                elif new_qoffset != old_qoffset and status_ok == False:
                    # 异常状态 变成 正常状态，需要发送喜报
                    print "queue[%s] update OK. remote[%s:%u]" % (x['name'], x['host'], x['port'],)
                    cur_queue_status[x['name']]['qoffset'] = new_qoffset
                    cur_queue_status[x['name']]['status_ok'] = True
                elif new_qoffset != old_qoffset and status_ok == True:
                    # 这个也不需要报警了
                    print "I know this guy[%s - %s:%u] is OK."  % (x['name'], x['host'], x['port'],)
                    cur_queue_status[x['name']]['qoffset'] = new_qoffset
            else:
                # 此通道刚刚开启或者是监控程序第一次启动
                qstatus = {}
                qstatus["qoffset"] = new_qoffset
                qstatus["status_ok"]  = True
                cur_queue_status[x['name']] = qstatus
    # 休眠指定的间隔，单位为秒
    time.sleep(check_interval_seconds)
