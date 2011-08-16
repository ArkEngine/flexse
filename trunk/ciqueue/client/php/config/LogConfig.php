<?php

// 导出给CLogger使用
$GLOBALS['CLOG_CONFIG'] = array (

    // 日志级别配置，0x07 = LOG_LEVEL_FATAL|LOG_LEVEL_WARNING|LOG_LEVEL_NOTICE
    'intLevel'			=> 0xff,
    // 日志文件路径，wf日志为front.log.wf
    'strLogFile'		=> dirname(__FILE__)."/../log/sdk.log",
    // 0表示无限
    'intMaxFileSize'    => 0,
    // 特殊日志路径，根据需要自行配置
    'arrSelfLogFiles'	=> array(),

);
