#!/bin/bash
#清理，初始化
clear
rm *.csv

sendIaTime=(166.6666667 83.33333333 55.55555556 41.66666667 33.33333333 27.77777778 23.80952381 20.83333333 18.51851852	16.66666667	15.15151515	13.88888889	12.82051282)
# batchsize=(1 2 10 50)
batchsize=(100 200)
for j in "${batchsize[@]}"; do
    sed -i "s/batchSize\s*=\s*\([0-9.]\+\)\s*/batchSize = $j /" omnetpp.ini
    for i in "${sendIaTime[@]}"; do
        sed -i "s/sendIaTime\s*=\s*exponential(\([0-9.]\+\)\s*ms)/sendIaTime = exponential($i ms)/" omnetpp.ini
        opp_runall -j1 ./mynet -u Cmdenv -c mynet   
        sleep 1
        fileName="batchsize=""$j""+sendIaTime="$i
        opp_scavetool x -f 'type =~ scalar' results/*.sca -F CSV-R -o "$fileName.csv"
        # rm results/*
    done
done
python3 batchsize.py
