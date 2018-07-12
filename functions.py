import paramiko
import csv
from operator import itemgetter
from time import sleep
import os
from globals import *
from math import ceil

def topologyRead(filename):
	#Create a dictionary and read the data into it

	print 'Reading WAMSTopology.csv .......................'
	print '.............................................'

	csvFile = csv.reader(open(filename, "rb"))
	index = -1;
	tCount = 0
	pdcFlag = -1
	for row in csvFile:
	  	print row
		if(row):
			row = row[0].split('\t')
			#print row
			if((index != -1) and (row[1] == 'PMU')):	
			  	#print str(row[0])
				#print 'hello'
			  	topology[IDCODE].append(row[0])
			  	topology[TYPE].append(row[1])			  	
			  	topology[SRC_ID].append(row[2].split('::'))
			  	topology[DEST_ID].append(row[3].split('::'))
			  	topology[UDPPORT].append(row[4])
			  	topology[TCPPORT].append(row[5])
			   	topology[DATAINPUT].append(row[6])
			  	topology[FILENAME].append(row[7])			  	
			  	topology[DATARATE].append(row[8])				  	
			  	index = index + 1
			  	COUNT[NoOfPMUs] += 1
				#print COUNT
			
			elif((index != -1) and (row[1] == 'PDC')):
				if(pdcFlag == -1):
					print 'hello'
					#print str(index)
					topology[IDCODE].append(row[0])
			  		topology[TYPE].append(row[1])
			  		topology[SRC_ID].append(row[2].split('::'))			  	
			  		topology[DEST_ID].append(row[3].split('::'))
			  		topology[UDPPORT].append(row[4])
			  		topology[TCPPORT].append(row[5])
			   		topology[DATAINPUT].append(row[6])
			  		topology[FILENAME].append(row[7])			  	
			  		topology[DATARATE].append(row[8])	
				  	
				  	if (row[2].split('::')[0] == 'NA'):
				  		srcCount = 0
				  	else:				  		
				  		srcCount = len(row[2].split('::'));
				  	if (row[3].split('::')[0] == 'NA'):
				  		dstCount = 0
				  	else:	
				  		dstCount = len(row[3].split('::'))	

			  		tCount =  srcCount + dstCount
					#print tCount
			  		index = index + 1
			  		t = 0 # Source
			  		k = 0 # Destination
			  		pdcFlag = 0
			  		ipdcSetupDetails = []
			  		ipdcSetupDetails.append([row[0],row[4],row[5],'127.0.0.1',srcCount,dstCount])
 # IDCODE, UDPPORT, TCPPORT,DBSERVER IP,SRC COUNT,DST COUNT
			  		ipdcSourceDetails = []
			  		ipdcDestDetails = []
					print topology

					#print topology
					#print t
					#print k
					#print ipdcSourceDetails

			  	elif((pdcFlag == 0) and (t+k<tCount)):
				#if(pdcFlag == 0):
					

			  		if(row[2] != 'NA'):  # Source IDCode
			  			#print row[2]
			  			# The logic to extract the index of row[2] and then use it to get the port No			  						  		
			  			tempList = []
			  			tempList.append(topology) 
						#print tempList			  			
			  			lst = map(itemgetter(IDCODE), tempList)
			  			#print lst
			  			ind = lst[0].index(row[2])
						#print ind
			  			tempList =  []
						#print ipdcSourceDetails
			  			if(row[9] == 'UDP'):
			  				portNo = topology[UDPPORT][ind]
			  				ipdcSourceDetails.append([row[2],portNo,'UDP'])		
			  			elif(row[9] == 'TCP'):
							portNo = topology[TCPPORT][ind]	
							ipdcSourceDetails.append([row[2],portNo,'TCP'])
						t = t + 1
						PDC_DETAILS[SOURCE_DETAILS].append(ipdcSourceDetails)
						print ipdcSourceDetails

						#print tCount
						#print k
						#print t
						if( t+k  == tCount):

							#print 'hiii'

			  				pdcFlag = -1
			  				#print int(row[0]) 
							#print ipdcSetupDetails
			  				PDC_DETAILS[PDC_SETUP].append(ipdcSetupDetails)
			  				PDC_DETAILS[SOURCE_DETAILS].append(ipdcSourceDetails)
							#print ipdcSourceDetails
		  					PDC_DETAILS[DESTINATION_DETAILS].append(ipdcDestDetails)
			  		
			  		elif(row[3] != 'NA'): # Dest IDCode
			  			
			  			#print 'Dest'
				  		if(row[9] == 'UDP'):			  				
			  				ipdcDestDetails.append([row[3],'UDP'])		
			  			elif(row[9] == 'TCP'):						
							ipdcDestDetails.append([row[3],'TCP'])
			  
			  			k = k + 1
			  			#print str(row[0])+ ' ' +str(tCount) + ' ' + str(t)+' '+str(k)
			  			if(t+k == tCount):
			  		
			  				pdcFlag = -1
			  				PDC_DETAILS[PDC_SETUP].append(ipdcSetupDetails)
			  				PDC_DETAILS[SOURCE_DETAILS].append(ipdcSourceDetails)
			  				PDC_DETAILS[DESTINATION_DETAILS].append(ipdcDestDetails)
			  		else:
			  			print 'Prob'			

			else:
				index = index + 1;
				print 'index' + str(index)
		else:
			 break	

def displayTopology():

	print "\n++++++++++++++++++++++++++++++  WAMS TOPOLOGY  +++++++++++++++++++++++++++++++\n"
	print IDCODE ,' ',TYPE,' ', SRC_ID,' ',DEST_ID, '',UDPPORT,' ',TCPPORT,' ',DATAINPUT, ' ',FILENAME,' ',DATARATE 

	print '-------------------------------------------------------------------------------------------------------------'
	for i in range(0,len(topology[IDCODE])):		
		print '| '+str(topology[IDCODE][i]) + '\t' +str(topology[TYPE][i]) +'\t' + str(topology[SRC_ID][i]) + '\t'+ str(topology[DEST_ID][i]) + '\t' + str(topology[UDPPORT][i]) +'\t'+str(topology[TCPPORT][i])+'\t'+str(topology[DATAINPUT][i])+'\t'+str(topology[FILENAME][i]) +'\t' +str(topology[DATARATE][i])+' |' #+ ' ' +str(topology[PROTOCOL][i])
	print '-------------------------------------------------------------------------------------------------------------'

def displayPDCs():

	print len(PDC_DETAILS[PDC_SETUP])

	print "\n++++++++++++++++++++++++++++++  PDC DETAILS  ++++++++++++++++++++++++++++++++\n"

	print '-------------------------------------------------------------------------------------------------------------'
	for i in range(0,len(PDC_DETAILS[PDC_SETUP])):		
		print 'PDC : '+ str(PDC_DETAILS[PDC_SETUP][i][0][0]) ,'\nSETUP : ',PDC_DETAILS[PDC_SETUP][i]
		print 'SRC : ',PDC_DETAILS[SOURCE_DETAILS][i]
		print 'DST : ',PDC_DETAILS[DESTINATION_DETAILS][i]
		print '-------------------------------------------------------------------------------------------------------------'


def ipRead(filename):
    
    print 'Reading IPFile.csv................'
    print '.............................'
    csvFile = csv.reader(open(filename, "rb"))

    for row in csvFile:
    	if(row): 
	
			print(row)
	  		if(COUNT[NoOfMachines] != -1):	

				row = row[0].split('\t')
				ipList[IP].append(row[0])
				ipList[NAME].append(row[1])
				ipList[PASSWD].append(row[2])
				COUNT[NoOfMachines]= COUNT[NoOfMachines] +1;
			else:
				COUNT[NoOfMachines] = COUNT[NoOfMachines] +1;
        else:
			break	
	
    return ipList

def checkAvaialbleMachines():
	print 'Check machine availabilty ..........'
	#tempCount = 0
	#for i in range(0,len(topology[TYPE])):
	#	if(topology[TYPE][i] == 'PDC'):
	#		tempCount = tempCount+1
	MachinesForPDC = len(PDC_DETAILS[PDC_SETUP])
	print str(MachinesForPDC)
	MachinesAvailable = len(ipList[IP])
	
	print COUNT[(NoOfPMUs)]
	#print COUNT[(NoOfPMUsOnOneMachine)]
	COUNT[(NoOfPMUsOnOneMachine)]=1
	print COUNT[(NoOfPMUsOnOneMachine)]


	MachinesForPMU = int(ceil(COUNT[NoOfPMUs]/COUNT[NoOfPMUsOnOneMachine]))	
	print MachinesForPMU

	if((MachinesForPMU + MachinesForPDC)>(MachinesAvailable)):	
		return False
	else:
		return True

def createSSHConnection(hostname,username,password):

	print 'connecting '+ str(hostname)+'...................'
	ssh_client = paramiko.SSHClient()
	ssh_client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
	ssh_client.connect(hostname, username=username, password=password)

	return ssh_client	

def createDirWithPermissions(ssh_client,s):

	print 'Create directory with permissions ...........'
	print(s)
	try:
  		ssh_client.exec_command(s)
#  		sleep(0.1)
	except IOError, e:
		print '(assuming directory exists)', e
	
def copyFileToRemoteMachine(ssh_client,local_file_path,remote_file_path):
	
	ftp = ssh_client.open_sftp()
	try:
        
		print 'Copying', local_file_path, 'to ', remote_file_path,'..........'
		ftp.put(local_file_path, remote_file_path)
   		ftp.close()    

	except Exception, e:
		print '*** Caught exception: %s: %s' % (e.__class__, e)	
