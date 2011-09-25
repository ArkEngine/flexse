#include <stdio.h>
#include <json/json.h>
#include <iostream>
#include <fstream>
#include "mylog.h"
#include "MyException.h"
#include "connectmanager.h"

using namespace std;

int main()
{
    SETLOG(0, "test.log");
    Json::Value root;
    Json::Reader reader;
    ifstream in("./conf/test.config.json");
    if (! reader.parse(in, root))
    {
        FATAL("json format error.");
        MyToolThrow("json format error.");
    }

    ConnectManager* cm = new ConnectManager(root);
    MyThrowAssert(NULL != cm);
    for (uint32_t i=0; i<5; i++)
    {
        int sock = cm->FetchSocket("/search/leaf0", "test");
        PRINT("sock: %d", sock);
        MyThrowAssert(0 == cm->FreeSocket(sock, false));
    }

    return 0;
}