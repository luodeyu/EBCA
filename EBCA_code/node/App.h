#ifndef APP_H
#define APP_H

#include <vector>
#include <omnetpp.h>
#include <algorithm>
#include <iostream>
#include "Message_m.h"

using namespace omnetpp;
using namespace std;

class App : public cSimpleModule
{
// private:
public:
    // configuration
    int myAddress;
    vector<int> destAddresses; // 目的地址列表
    vector<int> router;        // router地址列表
    cPar *sendIATime;          // 发送间隔
    cPar *packageLengthBytes;
    cPar *voteLengthBytes;
    cPar *batchSize;
    cQueue requestQueue;  // Request缓存队列，等待打包
    cQueue TxQueue;       // 交易缓存队列，等待投票
    int prepareCount = 0; // prepare计数器，当收集到一定数量的prepare之后回复ack
    int commitCount = 0;  // commit计数器，当收集到一定数量的commit之后回复decide
    cPar *quorum;
    cPar *N;

    // state
    cMessage *generate = nullptr;

    // signals
    simsignal_t endToEndDelaySignal;
    simsignal_t TxQueueLenSignal;
    simsignal_t TxcountSignal;

// public:
    virtual ~App();

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void handleRequest(Package *pk);
    virtual void handleTransaction(Package *pk);
    virtual void handlePrepare(Package *pk);
    virtual void handlePrepareAck(Package *pk);
    virtual void handleCommit(Package *pk);
    virtual void handleCommitAck(Package *pk);
    virtual void handleDecide(Package *pk);
    // virtual void handleDecideAck(Package *pk);
    virtual void handleAndSend(Package *pk, char *pkname, int LengthBytes, int kind, bool broadcast);
};

#endif  // app_H