#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif

#include "App.h"

class AppV : public App
{
protected:
    // virtual void initialize() override;
    virtual void handleRequest(Package *pk) override;
    // virtual void handleTransaction(Package *pk) override;
    // virtual void toLeader(Package *pk, char *pkname, int LengthBytes, int kind);
};

Define_Module(AppV);

//leader将请求转化为交易并保存
void AppV::handleRequest(Package *pk)
{
    if (hasGUI())
        getParentModule()->bubble("Request Arrived!!!");

    pk->setTimestamp();
    requestQueue.insert(pk); // （检查并）保存请求


    // 当交易队列中，交易的数目大于等于设定的batchsize,则从请求队列中取出相应数目的request,生成区块头并广播
    if (requestQueue.getLength() >= batchSize->intValue())
    {
        // cout << getParentModule()->getName() << " requestQueue.getLength():" << requestQueue.getLength() << endl;

        // request->tx
        for (int i = 0; i < batchSize->intValue(); i++)
            TxQueue.insert((Package *)requestQueue.pop());

        // 生成block并广播
        char pkname[40];
        sprintf(pkname, "Broadcast block from %d", myAddress);
        handleAndSend(nullptr, pkname, packageLengthBytes->intValue()* (batchSize->intValue()), 2, true); // 2表示header,prepare
    }
}