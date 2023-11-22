import csv,openpyxl
# import subprocess

filename='nodenum'+'.xlsx'

#输入数据
sendIaTime='210'.split('\t')
nodenum='5 6 7 8 9 10'.split(' ')
#参数设置

repeatTime=1 #模拟的重复次数,与吞吐求解相关

simTime=1000 #运行的时间,与吞吐求解相关

workbook = openpyxl.Workbook()
#第一页记录交易平均时延
sheet = workbook.active
sheet.title='delay'
# sheet.append(["tps",'mynet'])

#第二页记录TPS
sheet2 = workbook.create_sheet(title='tps')
# sheet2.append(["tps",'mynet'])

#数据读取,处理,记录
for j_index,j in enumerate(nodenum): 
    for i_index,i in enumerate(sendIaTime):
        csv_file="data/nodenum="+j+"+sendtps="+str(i)+".csv"
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
        
    Var=j
    #交易平均时延
    workbook.active = sheet
    # sheet.cell(row=i_index+1, column=j_index+1, value=sum(mynet)/len(mynet))
    sheet.append([Var,sum(mynet)/len(mynet),sum(vnet)/len(vnet),sum(oldnet)/len(oldnet)])#节点数，以及相关时延

    #tps
    workbook.active = sheet2
    tpsData=[sum(i)/repeatTime/simTime for i in[mynetTxcount,vnetTxcount,oldnetTxcount]]
    sheet2.append([Var]+tpsData)#发送tps，以及吞吐
    # sheet2.cell(row=i_index+1, column=j_index+1, value=sum(mynetTxcount)/repeatTime/simTime)
workbook.save(filename)
print(filename)

# result = subprocess.run("rm results/*;rm ./*.csv", shell=True, capture_output=True, text=True)
