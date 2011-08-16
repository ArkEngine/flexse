<?php
require_once dirname(__FILE__)."/../source/ciqueueClient.Class.php";

ini_set("display_errors", TRUE);
$message = array("intID"=>123, "strUname"=>"kaka");
$cc = new ciqueueClient();
for ($i=0; $i < 10; $i++)
{
    var_dump($cc->commit($i, "test", "test", $message));
}
