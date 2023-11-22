#!/bin/bash
#清理，初始化
clear
rm results/*
rm data/*.csv

nodenum=(5 6 7 8 9 10)
sendtps=(210)
for j in "${nodenum[@]}"; do
    inifile="omnetpp-"$j".ini"
    for i in "${sendtps[@]}"; do
        sendIaTime=$(echo "scale=2;1000.0*$j/$i" |bc) #计算发送间隔
        sed -i "s/sendIaTime\s*=\s*exponential(\([0-9.]\+\)\s*ms)/sendIaTime = exponential($sendIaTime ms)/" $inifile
        opp_runall -j1 ./mynet -u Cmdenv -f $inifile -c mynet
        opp_runall -j1 ./mynet -u Cmdenv -f $inifile -c vnet
        opp_runall -j1 ./mynet -u Cmdenv -f $inifile -c oldnet
        sleep 1
        fileName="nodenum=""$j""+sendtps="$i
        opp_scavetool x -f 'type =~ scalar' results/*.sca -F CSV-R -o "data/$fileName.csv"
        
    done
done
python3 node.py
