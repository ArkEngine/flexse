<?php
ini_set("display_errors", true);
require_once dirname(__FILE__)."/../CLogger.Class.php";

while(1)
{
    CLogger::warning("this is warning.");
    sleep(1);
    CLogger::notice("this is notice.");
    sleep(1);
    CLogger::debug("this is debug.");
    sleep(1);
}
