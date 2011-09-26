#include <stdio.h>
#include <json/json.h>
#include <iostream>
#include <fstream>
#include "mylog.h"
#include "MyException.h"
#include "connectmanager.h"

using namespace std;

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("./test balanceKey\n");
        exit(1);
    }
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
    int sock[100];
    for (uint32_t i=0; i<50; i++)
    {
        sock[i] = cm->FetchSocket("leaf0", argv[1]);
        PRINT("sock: %d", sock[i]);
    }
    for (uint32_t i=0; i<50; i++)
    {
        MyThrowAssert(0 == cm->FreeSocket(sock[i], false));
    }

    return 0;
}
