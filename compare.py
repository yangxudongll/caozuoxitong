inputfile="input.txt"
outputfile="output.txt"

fp1=open(inputfile,"r");
fp2=open(outputfile,"r");
i=0

while(1):
	line1=fp1.readline()
	line2=fp2.readline()
	i=i+1
	if(line1!=line2):
		print("第{}行有差异".format(i))
		break
	if(line1==''):
		print("两个文件相同,共{}行".format(i))
		break
