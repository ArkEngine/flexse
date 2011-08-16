<?php
require_once ("../queueReceiver.Class.php");
require_once ("messageCallBackFunc.php");

// 12345 表示端口
// message_break_num 表示处理多少个消息后die, 为0表示无限制
// interval_break_time 表示处理多长时间后die，为0表示无限制
// 设置处理一段时间后退出是为了防止php资源泄漏，定期重启即可。
$message_break_num = 100;
$interval_break_time = 100;
$qc = new queueReceiver(12345, $message_break_num, $interval_break_time, "messageCallBackFunc");
echo "xxxxoooo\n";
while (1)
{
	$qc->processMessage();
}

// 如果不是普通的回调函数，比如是类的成员函数
// $obj = new MyClass();
// $qc = new queueReceiver("0.0.0.0", 12345,
//     $message_break_num, $interval_break_time, array($obj, "your_class_method_name"));
// 如果是类的静态函数，见下
// $qc = new queueReceiver("0.0.0.0", 12345,
//     $message_break_num, $interval_break_time, array('MyClass', 'myCallbackMethod'));
// 请参见url: http://au2.php.net/manual/en/language.pseudo-types.php#language.types.callback
