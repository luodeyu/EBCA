import csv,openpyxl
import subprocess

filename='batchsize-band60000'+'.xlsx'

#输入数据
sendIaTime='166.6666667	83.33333333	55.55555556	41.66666667	33.33333333	27.77777778	23.80952381	20.83333333	18.51851852	16.66666667	15.15151515	13.88888889	12.82051282'.split('\t')
batchsize='100 200'.split(' ')
#参数设置

nodeNum=5 #节点数量,与设置TPS值相关

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
for j_index,j in enumerate(batchsize):
    for i_index,i in enumerate(sendIaTime):
        csv_file="batchsize="+j+"+sendIaTime="+str(i)+".csv"
        print("csv:",csv_file)
        mynet=[]
        mynetTxcount=[]
        with open(csv_file, newline='') as file:        
            reader = csv.reader(file)
            for row in reader:
                if row[3].startswith('endToEndDelay:mean') and row[4]=='':
                    mynet.append(float(row[6]))
                    # print(row)
                elif row[3].startswith('Txcount:sum') and row[4]=='':
                    mynetTxcount.append(float(row[6]))       
        file.close()
        #交易平均时延
        workbook.active = sheet
        sheet.cell(row=i_index+1, column=j_index+1, value=sum(mynet)/len(mynet))
        #tps
        workbook.active = sheet2
        sheet2.cell(row=i_index+1, column=j_index+1, value=sum(mynetTxcount)/repeatTime/simTime)
workbook.save(filename)
print(filename)

# result = subprocess.run("rm results/*;rm ./*.csv", shell=True, capture_output=True, text=True)
