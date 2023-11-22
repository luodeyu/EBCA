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

#include <map>
#include <omnetpp.h>
#include "Message_m.h"

using namespace omnetpp;

/**
 * Demonstrates static routing, utilizing the cTopology class.
 */
class Routing : public cSimpleModule
{
private:
    int myAddress;

    typedef std::map<int, int> RoutingTable; // destaddr -> gateindex
    RoutingTable rtable;

    // simsignal_t dropSignal;//每丢弃包一次发送一次signal

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(Routing);

void Routing::initialize()
{
    myAddress = getParentModule()->par("address"); // 获取父模块的地址参数，赋值给myAddress

    // dropSignal = registerSignal("drop"); // 注册名为"drop"的自定义信号

    //
    // Brute force approach -- every node does topology discovery on its own,
    // and finds routes to all other nodes independently, at the beginning
    // of the simulation. This could be improved: (1) central routing database,
    // (2) on-demand route calculation
    //

    cTopology *topo = new cTopology("topo"); // 创建名为"topo"的拓扑对象

    std::vector<std::string> nedTypes;                       // 创建存储模块类型名称的字符串向量
    nedTypes.push_back(getParentModule()->getNedTypeName()); // 将当前模块的类型名称添加到nedTypes中
    topo->extractByNedTypeName(nedTypes);                    // 从拓扑中提取与当前模块类型相同的节点

    // EV << "cTopology found " << topo->getNumNodes() << " nodes\n";  // 输出拓扑中找到的节点数量

    cTopology::Node *thisNode = topo->getNodeFor(getParentModule()); // 获取当前模块在拓扑中的节点对象

    // find and store next hops
    for (int i = 0; i < topo->getNumNodes(); i++)
    {
        if (topo->getNode(i) == thisNode)
            continue; // 跳过当前节点

        topo->calculateUnweightedSingleShortestPathsTo(topo->getNode(i)); // 计算当前节点到目标节点的最短路径

        if (thisNode->getNumPaths() == 0)
            continue; // 如果当前节点与目标节点不连通，则跳过

        cGate *parentModuleGate = thisNode->getPath(0)->getLocalGate(); // 获取最短路径中的第一个本地门
        int gateIndex = parentModuleGate->getIndex();                   // 获取本地门的索引
        int address = topo->getNode(i)->getModule()->par("address");    // 获取目标节点的地址参数
        rtable[address] = gateIndex;                                    // 将目标地址和门索引存储在路由表中
        // EV << "  towards address " << address << " gateIndex is " << gateIndex << endl;  // 输出路由表的信息
    }

    delete topo; // 释放拓扑对象的内存
}

void Routing::handleMessage(cMessage *msg)
{
    Package *pk = check_and_cast<Package *>(msg);

    int destAddr = pk->getDestAddr();

    if (destAddr == myAddress)
    { // 如果目标地址与当前路由器地址相同，进行本地传递
        // EV << "local delivery of packet " << pk->getName() << endl;
        send(pk, "localOut"); // 将消息通过 "localOut" 输出门发送出去
        return;
    }

    RoutingTable::iterator it = rtable.find(destAddr); // 在路由表中查找目标地址的迭代器

    if (it == rtable.end())
    { // 如果目标地址在路由表中不存在，无法到达目标地址
        // EV << "address " << destAddr << " unreachable, discarding packet " << pk->getName() << endl;  // 输出日志，表示目标地址不可达
        // emit(dropSignal, (intval_t)pk->getByteLength());  // 发送信号，表明丢弃了数据包，同时记录数据包长度
        delete pk;
        return;
    }

    int outGateIndex = (*it).second; // 获取路由表中目标地址对应的输出门索引

    // EV << "forwarding packet " << pk->getName() << " on gate index " << outGateIndex << endl;  // 输出日志，表示转发消息

    send(pk, "out", outGateIndex); // 将消息通过输出门outGateIndex发送出去
}
