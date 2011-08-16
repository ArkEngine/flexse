<?php
class queueClientConfig
{
	private static $instance = null;
	private function __construct()
	{
	}
	public static function getInstance()
	{
		if(self::$instance == null)
		{
			self::$instance = new queueClientConfig();
		}

		return self::$instance;
	}

	public $arrayQueueServerConfig = array (
		array("Host" => "10.1.123.15",  "Port" => 1983 ),
		array("Host" => "10.1.122.226", "Port" => 1983 ),
		array("Host" => "10.1.122.215", "Port" => 1983 ),
	);

	public function getConfig()
	{
		return $this->arrayQueueServerConfig;
	}
};
