<?php

// ������CLoggerʹ��
$GLOBALS['CLOG_CONFIG'] = array (

    // ��־�������ã�0x07 = LOG_LEVEL_FATAL|LOG_LEVEL_WARNING|LOG_LEVEL_NOTICE
    'intLevel'			=> 0xff,
    // ��־�ļ�·����wf��־Ϊfront.log.wf
    'strLogFile'		=> dirname(__FILE__)."/../log/sdk.log",
    // 0��ʾ����
    'intMaxFileSize'    => 0,
    // ������־·����������Ҫ��������
    'arrSelfLogFiles'	=> array(),

);
