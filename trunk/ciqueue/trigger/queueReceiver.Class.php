<?php

define ("XHEAD_SIZE", 32);
define ("FORMAT_XHEAD_DECODE", "Ilog_id/a16hiname/Iversion/Ireserved/Idetail_len");
define ("FORMAT_XHEAD_ENCODE", "Ia16III");
require_once dirname(__FILE__)."/../logger/CLogger.Class.php";

class queueReceiver
{
    private $m_socket;
    private $m_host;
    private $m_port;
    private $m_last_file_no;
    private $m_last_block_id;
    private $m_last_log_id;
    private $m_last_server;
    // 处理多少个消息之后退出
    private $m_break_message_num;
    // 当前处理的消息数目
    private $m_message_done_count;
    // 最多联系工作时间
    private $m_break_interval;
    private $m_begin_time;
    // 回调处理单元
    private $m_user_call_back;

    public function __construct($intPort, $intBreakMessageNum, $intBreakInterval, $callBackFunc)
    {
        $this->m_last_file_no  = -1;
        $this->m_last_block_id = -1;
        $this->m_last_log_id   = -1;
        $this->m_last_server   = "0.0.0.0";
        $this->m_host = $strHost;
        $this->m_port = $intPort;
        $this->m_break_message_num  = $intBreakMessageNum;
        $this->m_message_done_count = 0;
        $this->m_break_interval     = $intBreakInterval;
        $this->m_begin_time         = time();
        $this->m_user_call_back     = $callBackFunc;

        is_callable($this->m_user_call_back) or die("call_back_func invalid\n");
        $this->m_socket = socket_create (AF_INET, SOCK_STREAM, SOL_TCP);
        if (FALSE == $this->m_socket)
        {
            die ("socket_create() failed:".socket_strerror ($this->m_socket)."\n");
        }
        socket_set_option($this->m_socket,SOL_SOCKET,SO_REUSEADDR,1)
            or die("socket_set_option failed.\n");
        socket_bind ($this->m_socket, $this->m_host, $this->m_port)
            or die("socket_bind() failed:".socket_strerror ($bind)."\n");
        socket_listen ($this->m_socket)
            or die ("socket_listen() failed:".socket_strerror ($listen)."\n");
    }

    public function __destruct()
    {
        socket_close ($this->m_socket);
    }

    public function isNeedDie()
    {
        if ($this->m_break_message_num != 0 && 
            $this->m_message_done_count > $this->m_break_message_num)
        {
            return TRUE;
        }
        $intTimeNow = time();
        if ($this->m_break_interval != 0 &&
            ($intTimeNow - $this->m_begin_time) > $this->m_break_interval)
        {
            return TRUE;
        }
        return FALSE;
    }
    public function readSocketForDataLength ($socket, $len)
    {
        $offset = 0;
        $socketData = '';

        while ($offset < $len) {
            if (($data = @socket_read ($socket, $len-$offset)) === false) {
                return false;
            }

            $dataLen = strlen ($data);
            $offset += $dataLen;
            $socketData .= $data;

            if ($dataLen == 0) { break; }
        }

        return $socketData;
    }

    public function processMessage()
    {
        while(true)
        {
            $spawn = socket_accept ($this->m_socket);
            if (!$spawn)
            {
                Clogger::warning( "socket_accept() failed:".socket_strerror ($spawn));
                continue;
            }
            socket_set_option($spawn, SOL_SOCKET, SO_SNDTIMEO, array("sec"=>1, "usec"=>0));
            socket_set_option($spawn, SOL_SOCKET, SO_RCVTIMEO, array("sec"=>1, "usec"=>0));

            // 尽量保持长链接
            while (true)
            {
                if ($this->isNeedDie())
                {
                    die("need to die\n");
                }
                $this->m_message_done_count ++;
                $bindata = socket_read($spawn, XHEAD_SIZE);
                if (strlen($bindata) != XHEAD_SIZE)
                {
                    $errCode = socket_last_error();
                    Clogger::warning ("read xhead size[".strlen($bindata)."] != 32 errCode[$errCode]");
                    socket_close($spawn);
                    break;
                }

                $xhead = unpack(FORMAT_XHEAD_DECODE, $bindata);
                $cltIP = "";
                socket_getpeername($spawn, $cltIP);

                if ($this->m_last_log_id   == $xhead["log_id"] &&
                    $this->m_last_server   == $cltIP &&
                    $this->m_last_file_no  == $xhead["version"] &&
                    $this->m_last_block_id == $xhead["reserved"])
                {
                    Clogger::warning("dup message log_id[$this->m_log_id] file_no[$this->m_last_file_no] ".
                        "block_id[$this->m_last_block_id] server[$cltIP]");
                    $strMessage = $this->readSocketForDataLength($spawn, $xhead["detail_len"]);
                    if (strlen($strMessage) != $xhead["detail_len"])
                    {
                        socket_close ($spawn);
                        break;
                    }
                    else
                    {
                        $binaryData = pack(FORMAT_XHEAD_ENCODE, $xhead["log_id"], "ccphpsdk", 0, 0, 0);
                        socket_write($spawn, $binaryData);
                        continue;
                    }
                }
                $strMessage = $this->readSocketForDataLength($spawn, $xhead["detail_len"]);
                $intMessageLen = strlen($strMessage);
                if ($intMessageLen != $xhead["detail_len"])
                {
                    Clogger::warning( "read message size[$intMessageLen] != ".$xhead['detail_len']);
                    socket_close ($spawn);
                    break;
                }

                $arrMessage = json_decode($strMessage, true);
                //                var_dump($arrMessage);
                if ($arrMessage == FALSE || $arrMessage == NULL)
                {
                    Clogger::warning("json_decode message[$strMessage] fail");
                    socket_close ($spawn);
                    break;
                }

                if (call_user_func($this->m_user_call_back, $arrMessage))
                {
                    $this->m_last_log_id   = $xhead["log_id"];
                    $this->m_last_file_no  = $xhead["version"];
                    $this->m_last_block_id = $xhead["reserved"];
                    $this->m_last_server   = $cltIP;

                    $binaryData = pack("Ia16III", $xhead["log_id"], "ccphpsdk", 0, 0, 0);
                    $ret = socket_write($spawn, $binaryData);
                    Clogger::notice("messageCallBackFunc exec success IP: ".$cltIP." ret: ".$ret." head: ".json_encode($xhead)." message: ".$strMessage);
                }
                else
                {
                    // 处理失败
                    Clogger::warning("messageCallBackFunc exec fail. head: ".json_encode($xhead)." message: ".$strMessage);
                    socket_close ($spawn);
                    break;
                }
            }
        }
    }
};
