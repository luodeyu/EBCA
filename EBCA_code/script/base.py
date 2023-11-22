import csv,openpyxl
#输入数据
sendIaTime='166.6666667	83.33333333	55.55555556	41.66666667	33.33333333	27.77777778	23.80952381	20.83333333	18.51851852	16.66666667	15.15151515	13.88888889	12.82051282'.split('\t')

#参数设置
filename='bandwidth-60000-12000-block10.xlsx'

nodeNum=5#节点数量,与设置TPS值相关
repeatTime=1#模拟的重复次数,与吞吐求解相关
simTime=400#运行的时间,与吞吐求解相关

workbook = openpyxl.Workbook()
#第一页记录交易平均时延
sheet = workbook.active
sheet.title='delay'
sheet.append(["sendTps",'mynet','vnet','oldnet'])
# sheet.append(["bandwidth",'mynet','vnet','oldnet'])

#第二页记录TPS
sheet2 = workbook.create_sheet(title='tps')
sheet2.append(["sendTps",'mynet','vnet','oldnet'])
# sheet2.append(["bandwidth",'mynet','vnet','oldnet'])

#数据读取,处理,记录
for i in sendIaTime:
    csv_file="data/sendIaTime="+i+".csv"
    print("csv:",csv_file)
    mynet=[];vnet=[];oldnet=[]
    mynetTxcount=[];vnetTxcount=[];oldnetTxcount=[]
    with open(csv_file, newline='') as file:        
        reader = csv.reader(file)
        for row in reader:
            if row[0].startswith('mynet') and row[3].startswith('endToEndDelay:mean') and row[4]=='':
                mynet.append(float(row[6]))
                # print(row)
            elif row[0].startswith('mynet') and row[3].startswith('Txcount:sum') and row[4]=='':
                mynetTxcount.append(float(row[6]))
                # print(row)
            elif row[0].startswith('vnet') and row[3].startswith('endToEndDelay:mean') and row[4]=='':
                vnet.append(float(row[6]))
                # print(row)
            elif row[0].startswith('vnet') and row[3].startswith('Txcount:sum') and row[4]=='':
                vnetTxcount.append(float(row[6]))
                #print(row)
            elif row[0].startswith('oldnet') and row[3].startswith('endToEndDelay:mean') and row[4]=='' and row[6]!='NaN':
                oldnet.append(float(row[6]))
                # print(row)
            elif row[0].startswith('oldnet') and row[3].startswith('Txcount:sum') and row[4]=='':
                oldnetTxcount.append(float(row[6]))
                # print(row)        
    file.close()
    

    Var=round(nodeNum*1000/float(i)) #自变量发送tps
    # Var=i #自变量带宽
    #交易平均时延
    workbook.active = sheet
    sheet.append([Var,sum(mynet)/len(mynet),sum(vnet)/len(vnet),sum(oldnet)/len(oldnet)])#发送tps，以及相关时延
    #tps
    workbook.active = sheet2
    tpsData=[sum(i)/repeatTime/simTime for i in[mynetTxcount,vnetTxcount,oldnetTxcount]]
    sheet2.append([Var]+tpsData)#发送tps，以及吞吐
workbook.save(filename)