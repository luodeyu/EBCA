
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

Define_Module(App);

App::~App()
{
    cancelAndDelete(generate);
}

void App::initialize()
{
    endToEndDelaySignal = registerSignal("endToEndDelay");
    TxcountSignal = registerSignal("Txcount");
    TxQueueLenSignal = registerSignal("TxQueueLen");

    myAddress = par("address");
    const char *routerPar = par("router");
    cStringTokenizer tokenizer(routerPar);
    const char *token;
    while ((token = tokenizer.nextToken()) != nullptr)
        router.push_back(atoi(token));

    if (find(router.begin(), router.end(), myAddress) == router.end())
    { // 非router的节点才初始化

        packageLengthBytes = &par("packageLength");
        voteLengthBytes = &par("voteLength");
        sendIATime = &par("sendIaTime"); // volatile parameter
        batchSize = &par("batchSize");
        quorum = &par("quorum");
        N = &par("N");

        // 读取地址参数，存入私有变量 vector<int> destAddresses
        const char *destAddressesPar = par("destAddresses");
        cStringTokenizer tokenizer(destAddressesPar);
        const char *token;
        while ((token = tokenizer.nextToken()) != nullptr)
            destAddresses.push_back(atoi(token));

        if (destAddresses.size() == 0)
            throw cRuntimeError("At least one address must be specified in the destAddresses parameter!");
        // 根据设定的发包间隔，发送selfmsg
        generate = new cMessage("next");
        scheduleAt(sendIATime->doubleValue(), generate);
    }
}

// sendType=0表示单播原路返回,等于1表示广播
void App::handleAndSend(Package *pk, char *pkname, int LengthBytes, int kind, bool broadcast)
{
    Package *p = new Package(pkname);
    p->setByteLength(LengthBytes);
    p->setKind(kind);
    p->setSrcAddr(myAddress);

    if (!broadcast) // 单播，回复（原路返回） for follower
    {
        p->setDestAddr(pk->getSrcAddr());
        delete pk;
        send(p, "out");
    }
    else // 广播 for leader
    {
        for (int i = 0; i < destAddresses.size(); i++)
        {
            if (destAddresses[i] == myAddress)
                continue;
            p->setDestAddr(destAddresses[i]);
            send(p->dup(), "out");
        }
        delete p;
    }
}

// leader
void App::handleRequest(Package *pk)
{
    if (hasGUI())
        getParentModule()->bubble("Request Arrived!!!");

    pk->setTimestamp();
    requestQueue.insert(pk); // （检查并）保存请求

    // 生成对应的区块链交易并广播
    char pkname[40];
    sprintf(pkname, "Broadcast transaction from %d", myAddress);
    handleAndSend(pk, pkname, packageLengthBytes->intValue(), 1, true); // 1表示transaction

    // 当交易队列中，交易的数目大于等于设定的batchsize,则从请求队列中取出相应数目的request,生成区块头并广播
    if (requestQueue.getLength() >= batchSize->intValue())
    {
        // cout << getParentModule()->getName() << " requestQueue.getLength():" << requestQueue.getLength() << endl;

        // request->tx
        for (int i = 0; i < batchSize->intValue(); i++)
            TxQueue.insert((Package *)requestQueue.pop());

        // 生成header并广播
        char pkname[40];
        sprintf(pkname, "Broadcast header from %d", myAddress);
        handleAndSend(nullptr, pkname, packageLengthBytes->intValue(), 2, true); // 2表示header,prepare
    }
}

// follower
void App::handleTransaction(Package *pk)
{
    if (hasGUI())
        getParentModule()->bubble("Transaction Arrived!!!");
    delete pk;
    // upgrade:（检查并）保存交易到交易池中
}

// follower
void App::handlePrepare(Package *pk)
{
    if (hasGUI())
        getParentModule()->bubble("Header Arrived!!!");
    // upgrade: 根据区块头中的信息提取交易池中相关交易并打包成块,并投票
    // 回复prepareAck
    char pkname[40];
    sprintf(pkname, "prepareAck:pk-%d-to-%d", myAddress, pk->getSrcAddr());
    handleAndSend(pk, pkname, voteLengthBytes->intValue(), 3, false); // 3表示prepareAck
}

// leader
void App::handlePrepareAck(Package *pk)
{
    if (hasGUI())
        getParentModule()->bubble("PrepareAck Arrived!!!");
    // upgrade: 收集并聚合prepareAck
    delete pk;
    prepareCount += 1;

    // 收到一定数量的prepareAck后,广播commit
    if (prepareCount == quorum->intValue() - 1) // 算上自己的,一共quorum票
    {
        char pkname[40];
        sprintf(pkname, "Broadcast commit from %d", myAddress);
        handleAndSend(nullptr, pkname, voteLengthBytes->intValue(), 4, true); // 4表示commit
    }
    else if (prepareCount == N->intValue() - 1) // 收到全部票
    {
        // cout << getParentModule()->getName()<<" Get all vote for prepare!" << endl;
        prepareCount = 0; // 计数器清零
    }
}

// follower
void App::handleCommit(Package *pk)
{
    if (hasGUI())
        getParentModule()->bubble("commit Arrived!!!");
    // upgrade: 根据区块头中的信息提取交易池中相关交易并打包成块,并投票

    // 回复commitAck
    char pkname[40];
    sprintf(pkname, "commitAck:pk-%d-to-%d", myAddress, pk->getSrcAddr());
    handleAndSend(pk, pkname, voteLengthBytes->intValue(), 5, false); // 5表示commitAck
}

// leader
void App::handleCommitAck(Package *pk)
{
    if (hasGUI())
        getParentModule()->bubble("CommitAck Arrived!!!");
    // upgrade: 收集并聚合commit
    commitCount += 1;
    delete pk;

    // 收到一定数量的commitAck后,广播decide,统计交易时延
    if (commitCount == quorum->intValue() - 1) // 算上自己的,一共quorum票
    {
        char pkname[40];
        sprintf(pkname, "Broadcast decide from %d", myAddress);
        handleAndSend(nullptr, pkname, voteLengthBytes->intValue(), 6, true); // 6表示decide

        
        // 统计交易时延
        for (int i = 0; i < batchSize->intValue(); i++) // 不知道这样搞会不会有逻辑问题!!!!!!!!!!!!!!!!!!!
        {
            Package *tx = (Package *)TxQueue.pop();
            emit(endToEndDelaySignal, simTime() - tx->getTimestamp()); // 记录端到端时延，从创建消息到到达终点
            emit(TxcountSignal, 1);                                    // 记录交易数量
            // cout << simTime() - tx->getTimestamp() << endl;
            delete tx;
        }
    }
    else if (commitCount == N->intValue() - 1) // 收到全部票
    {
        // cout << getParentModule()->getName()<<"Get all vote for commit!" << endl;
        commitCount = 0; // 计数器清零
    }
    emit(TxQueueLenSignal,TxQueue.getLength());
}

// follower
void App::handleDecide(Package *pk)
{
    if (hasGUI())
        getParentModule()->bubble("Decide Arrived!!!");
    // upgrade: 投票过程结束,执行交易
    delete pk;
}


void App::handleMessage(cMessage *msg)
{
    if (msg == generate)
    {
        // random Sending package
        volatile int randomdest;
        do
        {
            randomdest = intuniform(0, destAddresses.size() - 1);
        } while (destAddresses[randomdest] == myAddress);
        int destAddress = destAddresses[randomdest];
        char pkname[40];
        sprintf(pkname, "request:pk-%d-to-%d", myAddress, destAddress);
        Package *request = new Package(pkname);
        request->setByteLength(packageLengthBytes->intValue());
        request->setKind(0); // message kind -- 0>= user-defined meaning, <0 reserved,这里设定为0,表示request
        request->setSrcAddr(myAddress);
        request->setDestAddr(destAddress);
        send(request, "out");
        // 同上，根据设定的发包间隔，发送selfmsg
        scheduleAt(simTime() + sendIATime->doubleValue(), generate);
        if (hasGUI())
            getParentModule()->bubble("Generating package...");
    }
    else
    {
        Package *pk = check_and_cast<Package *>(msg);
        switch (pk->getKind())
        {
        case 0: // 收到request
            handleRequest(pk);
            break;
        case 1: // 收到Transaction
            handleTransaction(pk);
            break;
        case 2: // 收到header
            handlePrepare(pk);
            break;
        case 3: // 收到prepareAck
            handlePrepareAck(pk);
            break;
        case 4: // 收到prepareack
            handleCommit(pk);
            break;
        case 5:
            handleCommitAck(pk);
            break;
        case 6:
            handleDecide(pk);
            break;
        default:
            throw cRuntimeError("This is impossible!!!");
            break;
        }
    }
}
