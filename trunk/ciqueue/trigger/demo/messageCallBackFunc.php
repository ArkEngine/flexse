<?php
function messageCallBackFunc($arrMessage)
{
	// 返回TRUE表示成功，返回FALSE表示失败，消息会重发，因此如果一直失败，消息会阻塞
	// 通过监控消息队列的每个channel的进度会发现这个异常
	// 返回FALSE的原因可以是包格式不正确，也可能是sdk后面的数据库存在问题，或者是逻辑问题
    $arrBody = $arrMessage["__OPERATION_BODY__"];
	var_dump($arrBody);
    echo "appinfo".urldecode($arrBody["appinfo"])."\n";
    $arrAppInfo = json_decode(urldecode($arrBody["appinfo"]), true);
	var_dump($arrAppInfo);
	return TRUE;
}
