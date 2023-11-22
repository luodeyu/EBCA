//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2015 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//


// 
// Represents a transaction in the network.
// kind=0 表示request 红色
// kind=1 表示transaction 绿色
// kind=2 表示区块头 蓝色
// kind=3 表示prepare

packet Package
{
    int srcAddr @packetData;
    int destAddr @packetData;
    // long seq;
}