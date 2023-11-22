//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2015 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

/**
 * Point-to-point interface module. While one frame is transmitted,
 * additional frames get queued up; see NED file for more info.
 */
class L2Queue : public cSimpleModule
{
  private:
    intval_t frameCapacity;  // 帧容量

    cQueue queue;  // 消息队列
    cMessage *endTransmissionEvent = nullptr;  // 传输结束事件指针
    bool isBusy;  // 标识当前是否正在传输



  public:
    virtual ~L2Queue();

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void refreshDisplay() const override;
    virtual void startTransmitting(cMessage *msg);
};

Define_Module(L2Queue);

L2Queue::~L2Queue()
{
    cancelAndDelete(endTransmissionEvent);
}

void L2Queue::initialize()
{
    queue.setName("queue");  // 设置队列名称
    endTransmissionEvent = new cMessage("endTxEvent");  // 创建传输结束事件

    if (par("useCutThroughSwitching"))  // 判断是否启用直通交换
        gate("line$i")->setDeliverImmediately(true);  // 如果启用，立即投递

    frameCapacity = par("frameCapacity");  // 获取帧容量参数




    isBusy = false;  // 设置忙碌状态为 false（空闲）
}

void L2Queue::startTransmitting(cMessage *msg)
{
    // EV << "Starting transmission of " << msg << endl;  // 输出日志，表示开始传输消息
    isBusy = true;  // 设置忙碌状态为 true（忙碌）
    send(msg, "line$o");  // 将消息发送到输出门 "line$o"

    // 在最后一位比特离开门时，安排一个事件
    simtime_t endTransmission = gate("line$o")->getTransmissionChannel()->getTransmissionFinishTime();
    scheduleAt(endTransmission, endTransmissionEvent);
}

void L2Queue::handleMessage(cMessage *msg)
{
    if (msg == endTransmissionEvent) {
        // 传输完成，可以开始下一个传输
        // EV << "Transmission finished.\n";  // 输出日志，表示传输完成
        
        if (queue.isEmpty()) {
            isBusy = false;  // 设置忙碌状态为 false（空闲）
        }
        else {
            msg = (cMessage *)queue.pop();  // 从队列中取出下一个消息

            startTransmitting(msg);  // 开始传输消息
        }
    }
    else if (msg->arrivedOn("line$i")) {
        // 收到来自别的Node的消息,向上传递
        send(msg, "out");  // 将消息发送到输出门 "out"
    }
    else {  // 收到来routing的消息，等待转发
        if (isBusy) {//endTransmissionEvent->isScheduled()如果理解的没错的话这样修改是可以的
            // 当前正在忙碌，将消息排入队列
            if (frameCapacity && queue.getLength() >= frameCapacity) {
                //队列已满，丢包
                // EV << "Received " << msg << " but transmitter busy and queue full: discarding\n";  // 输出日志，表示接收到消息但传输器忙碌且队列已满，丢弃消息
                delete msg;  // 释放消息的内存
            }
            else {
                //队列未满，入队
                // EV << "Received " << msg << " but transmitter busy: queueing up\n";  // 输出日志，表示接收到消息但传输器忙碌，将消息排队
                msg->setTimestamp();  // 设置消息的时间戳
                queue.insert(msg);  // 将消息插入队列

            }
        }
        else {
            // 当前空闲，可以立即开始传输
            // EV << "Received " << msg << endl;  // 输出日志，表示接收到消息

            startTransmitting(msg);  // 开始传输消息
        }
    }
}

void L2Queue::refreshDisplay() const
{
    getDisplayString().setTagArg("t", 0, isBusy ? "transmitting" : "idle");  // 设置显示状态标签
    getDisplayString().setTagArg("i", 1, isBusy ? (queue.getLength() >= 3 ? "red" : "yellow") : "");  // 设置显示颜色
}
