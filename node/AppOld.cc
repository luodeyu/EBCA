
//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2015 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif

#include "App.h"

class AppOld : public App
{
public:
    int leader; // 添加leader
protected:
    virtual void initialize() override;
    virtual void handleRequest(Package *pk) override;
    virtual void handleTransaction(Package *pk) override;
    virtual void toLeader(Package *pk, char *pkname, int LengthBytes, int kind);
};

Define_Module(AppOld);

void AppOld::initialize()
{
    App::initialize();
    leader = par("leader"); // 添加leader传参
}
/*将收到的请求转化为交易并发送给leader*/
void AppOld::toLeader(Package *pk, char *pkname, int LengthBytes, int kind)
{
    Package *p = new Package(pkname);
    p->setByteLength(LengthBytes);
    p->setKind(kind);
    p->setSrcAddr(myAddress);
    p->setDestAddr(leader);
    p->setTimestamp(pk->getTimestamp());

    delete pk;
    send(p, "out");
    // cout << "sneding!!!!!!!!!!!" << endl;
}

// follower(包括leader),将请求转化为交易发送给leader
void AppOld::handleRequest(Package *pk)
{
    if (hasGUI())
        getParentModule()->bubble("Request Arrived!!!");

    pk->setTimestamp();

    // 生成transaction发送给leader
    char pkname[40];
    sprintf(pkname, "Generate transaction from %d", myAddress);
    toLeader(pk, pkname, packageLengthBytes->intValue(), 1); // 1表示transaction
}

// leader 收到足够的交易请求之后打包成块并广播
void AppOld::handleTransaction(Package *pk)
{
    requestQueue.insert(pk); // （检查并）保存请求
    if (requestQueue.getLength() >= batchSize->intValue())
    {

        // request->tx
        for (int i = 0; i < batchSize->intValue(); i++)
            TxQueue.insert((Package *)requestQueue.pop());

        // 生成block并广播
        char pkname[40];
        sprintf(pkname, "Broadcast block from %d", myAddress);
        handleAndSend(nullptr, pkname, packageLengthBytes->intValue()* (batchSize->intValue() + 1) , 2, true); // 2表示header,prepare
    }
}
