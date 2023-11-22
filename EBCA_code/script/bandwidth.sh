#!/bin/bash
#初始化
clear
rm results/* 
rm  ./*.csv

bandwidth=(60000	55000	50000	45000	40000	35000	30000)
index=($(seq 3 1 12))
flag=7
for i in "${bandwidth[@]}"; do
    sleep 2
    for j in "${index[@]}"; do
        if ((j < flag)); then
            sed -i "$j"'s/\b[1-9][0-9]\{3,\}\b/'"$((i*2))"'/' networks/connections-5.txt
        else
            sed -i "$j"'s/\b[1-9][0-9]\{3,\}\b/'"$i"'/' networks/connections-5.txt
        fi

    done

    opp_runall -j1 ./mynet  -u Cmdenv -c mynet
    opp_runall -j1 ./mynet  -u Cmdenv -c oldnet
    opp_runall -j1 ./mynet  -u Cmdenv -c vnet

    filename="bandwidth="$i
    opp_scavetool x -f 'type =~ scalar' results/*.sca -F CSV-R -o "$filename.csv"
done
python3 bandwidth.py
