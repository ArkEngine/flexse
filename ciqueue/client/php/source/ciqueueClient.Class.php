<?php
require_once dirname(__FILE__)."/CLogger.Class.php";
require_once dirname(__FILE__)."/../config/ciqueueClient.Config.php";

class ciqueueClient
{
    private $m_queueServerList;
    private $m_sock;

    public function __construct()
    {
		$this->m_queueServerList = queueClientConfig::getInstance()->getConfig();
        $this->m_sock = NULL;
        // $this->_connect();
        // 初始化函数里，连接服务器这种蠢事我居然干的出来。。
    }
    public function __destruct()
	{
		if ($this->m_sock != NULL)
		{
			socket_close($this->m_sock);
			$this->m_sock = NULL;
		}
	}

    private function _connect()
    {
		if ($this->m_sock != NULL)
		{
			socket_close($this->m_sock);
			$this->m_sock = NULL;
		}
		$srv_count = count($this->m_queueServerList);
		$si = rand() % $srv_count;
		$step = 0;
		for ($i=$si; $step<$srv_count; $i = ($i+1) % $srv_count, $step++)
		{
			$this->m_sock = socket_create(AF_INET, SOCK_STREAM, SOL_TCP); 
			socket_set_option($this->m_sock, SOL_SOCKET, SO_SNDTIMEO,
				array( "sec"=>0, "usec"=>100000 ) );
			$strHost = $this->m_queueServerList[$i]["Host"];
			$intPort = $this->m_queueServerList[$i]["Port"];
			if (FALSE == @socket_connect($this->m_sock, $strHost, $intPort))
			{
				socket_close($this->m_sock);
				$this->m_sock = NULL;
				//				return false;
				continue;
			}
			else
			{
				return true;
			}
		}
		return false;
	}

	private function _check_conntect()
	{
		$ret = @socket_recv($this->m_sock, $rstr, 1, MSG_DONTWAIT);
		if (0 === $ret)
		{
			//            echo "remote close\n";
			if ($this->m_sock != NULL)
			{
				socket_close($this->m_sock);
				$this->m_sock = NULL;
			}
			return $this->_connect();
		}
		if ($ret === false)
		{
			//            echo "timeout\n";
			return true;
		}
		else
		{
			//            echo "ooxx\n";
			if ($this->m_sock != NULL)
			{
				socket_close($this->m_sock);
				$this->m_sock = NULL;
			}
			return $this->_connect();
		}
		return true;
	}

	/* 返回 FALSE 表示失败 */
	public function commit($intLogID, $strQueueName, $strOperation, $arrOperationBody)
	{
		if ($this->_check_conntect())
		{
			$sendArray = array();
			$sendArray['__QUEUE_NAME__']     = $strQueueName;
			$sendArray['__OPERATION__']      = $strOperation;
			$sendArray['__OPERATION_BODY__'] = $arrOperationBody;

			$jsonstr = json_encode($sendArray);
			$FORMAT_XHEAD = "Ilogid/a16hiname/Iversion/Ireserved/Idetail_len";
			$binaryData = pack("Ia16III", $intLogID, "ccphp", 0, 0, strlen($jsonstr));
			$binaryData .= $jsonstr;
			//            var_dump(unpack($FORMAT_XHEAD, $binaryData));
			socket_set_option($this->m_sock, SOL_SOCKET, SO_RCVTIMEO, array( "sec"=>1, "usec"=>0 ) );
			socket_set_option($this->m_sock, SOL_SOCKET, SO_SNDTIMEO, array( "sec"=>1, "usec"=>0 ) );
			socket_write($this->m_sock, $binaryData);
			$rstr = socket_read($this->m_sock, 32);
			if (32 != strlen($rstr))
			{
                $len = strlen($rstr);
                Clogger::warning("socket_read error len: ".$len." queuename: ".$strQueueName." operation: ".$strOperation);
                return FALSE;
            }
            $retArray = unpack($FORMAT_XHEAD, $rstr);
            Clogger::notice("ciqueue queuename: ".$strQueueName." operation: ".$strOperation." response: ".json_encode($retArray));
            //            var_dump($retArray);
            return $retArray["reserved"] == 0; 
        }
        else
        {
            Clogger::warning("connect error queuename: ".$strQueueName." operation: ".$strOperation);
            return FALSE;
        }
    }
}
