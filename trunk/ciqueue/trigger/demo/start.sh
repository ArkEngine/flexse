#!/bin/sh
cd $(dirname $0)
ulimit -c unlimited
nohup ./sv_php_ciqueuetrigger_demo . &
