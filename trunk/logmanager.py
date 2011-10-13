# -*- coding: utf-8 -*-
import os
import os.path 
import sys
import time

def removeFile(targetDir, targetPrefix): 
    timestring = time.strftime("%Y%m%d", (time.localtime(time.time()-3*86400)));
    deletetime = int(timestring)*100
    for filename in os.listdir(targetDir): 
        targetFile = os.path.join(targetDir,  filename) 
        if os.path.isfile(targetFile): 
            spilt_filename = filename.split(targetPrefix)
            if len(spilt_filename) == 2 and int(spilt_filename[1]) < deletetime:
                os.remove(targetFile)
                print "delete %s" % (targetFile,)

def renameFile(targetDir, targetName): 
    dstString = targetName + "." + time.strftime("%Y%m%d%H");
    for filename in os.listdir(targetDir): 
        if targetName == filename:
            srcFile = os.path.join(targetDir,  filename)
            dstFile = os.path.join(targetDir,  dstString)
            if os.path.isfile(srcFile):
                print "'%s' rename to '%s'" % (srcFile, dstFile,)
                os.rename(srcFile, dstFile)
            break

def printHelp():
    print "--- remove ---"
    print "python logmanager.py remove dir fileprefix"
    print "只保留4天之内的日志，比如log/下有很多日志文件，如ciqueue.log.2011081521"
    print "执行 python logmanager.py remove log 'ciqueue.log.' 即可删除4天前的日志，保留今天在内的4天日志"
    print "--- rename ---"
    print "python logmanager.py rename dir filename"
    print "用于日志切分，比如log/下有日志文件sdk.log，需要切分为sdk.log.YYYYMMDDHH，如sdk.log.2011081521"
    print "执行 python logmanager.py rename log 'sdk.log' 即可切割为当前的sdk.log.YYYYMMDDHH"

if __name__ == "__main__":

    if len(sys.argv) != 4:
        printHelp()
        sys.exit(1)

    if sys.argv[1] != "rename" and sys.argv[1] != "remove":
        printHelp()
        sys.exit(1)

    if sys.argv[1] == "remove":
        removeFile(sys.argv[2], sys.argv[3])
    else:
        renameFile(sys.argv[2], sys.argv[3])
