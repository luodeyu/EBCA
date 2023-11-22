import csv,openpyxl
#输入数据
bandwidth='60000	55000	50000	45000	40000	35000	30000'.split('\t')

#参数设置
filename='bandwidth.xlsx'

nodeNum=5#节点数量,与设置TPS值相关
repeatTime=1#模拟的重复次数,与吞吐求解相关
simTime=400#运行的时间,与吞吐求解相关

workbook = openpyxl.Workbook()
#第一页记录交易平均时延
sheet = workbook.active
sheet.title='delay'
sheet.append(["bandwidth",'mynet','vnet','oldnet'])

#第二页记录TPS
sheet2 = workbook.create_sheet(title='tps')
sheet2.append(["bandwidth",'mynet','vnet','oldnet'])

#数据读取,处理,记录
for i in bandwidth:
    csv_file="bandwidth="+i+".csv"
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
    

    Var=i #自变量带宽
    #交易平均时延
    workbook.active = sheet
    sheet.append([Var,sum(mynet)/len(mynet),sum(vnet)/len(vnet),sum(oldnet)/len(oldnet)])#发送tps，以及相关时延
    #tps
    workbook.active = sheet2
    tpsData=[sum(i)/repeatTime/simTime for i in[mynetTxcount,vnetTxcount,oldnetTxcount]]
    sheet2.append([Var]+tpsData)#发送tps，以及吞吐
workbook.save(filename)