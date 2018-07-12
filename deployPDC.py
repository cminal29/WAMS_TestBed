import csv
from operator import itemgetter
from globals import *
from functions import createSSHConnection,createDirWithPermissions,copyFileToRemoteMachine
from time import sleep

def createPDCFileNames():

	print 'Creating csv names of iPDC setup files ......'
	NoOfPDC = len(PDC_DETAILS[PDC_SETUP])	
	print 'NoOfPDC',NoOfPDC
	for i in range(0,NoOfPDC):

		idcode = PDC_DETAILS[PDC_SETUP][i][0][0]
		pdcFileNames[idcode]='iPDC'+str(idcode)+'.csv'
		print pdcFileNames[idcode]

def populateDataInPDCFile(ipIndex,pmuIndex):

	print 'Populated data in iPDC csv files ........................'
	print '.............................'
	NoOfPDC = len(PDC_DETAILS[PDC_SETUP])

	# store map of ipdc idcode to ip
	for i in range(pmuIndex,pmuIndex + NoOfPDC):

		IPToIDMapList[topology[IDCODE][i]] = ipList[IP][ipIndex]
		ipIndex = ipIndex + 1

	for z in range(0,NoOfPDC):

		deviceId = PDC_DETAILS[PDC_SETUP][z][0][0] # get the idcode
		ipdcCSVFileName = pdcFileNames[deviceId]
		file_name = LOCAL_PDC_CSV_PATH+str(ipdcCSVFileName)
		result_writer = csv.writer(open(file_name,'wb'),dialect='excel')			
		result_writer.writerow(CSV_HEADER)
		result_writer.writerow(['iPDC SETUP FIELD VALUES',PDC_DETAILS[PDC_SETUP][z][0][0],PDC_DETAILS[PDC_SETUP][z][0][1],PDC_DETAILS[PDC_SETUP][z][0][2],PDC_DETAILS[PDC_SETUP][z][0][3],PDC_DETAILS[PDC_SETUP][z][0][4],PDC_DETAILS[PDC_SETUP][z][0][5]])
		result_writer.writerow([])
		result_writer.writerow(CSV_SOURCE_HEADER)
		populateSourceDeviceDetailsInCSV(result_writer,z)
		result_writer.writerow([])
		result_writer.writerow(CSV_DEST_HEADER)
		populateDestDeviceDetailsInCSV(result_writer,z)

	return ipIndex	

def populateSourceDeviceDetailsInCSV(result_writer,z):
	srcDeviceCount =  PDC_DETAILS[PDC_SETUP][z][0][4]
	print srcDeviceCount
	for i in range(0,srcDeviceCount):

		deviceId = PDC_DETAILS[SOURCE_DETAILS][z][i][0]
		
		if(i == 0):		

			result_writer.writerow(['SOURCE DEVICE FIELD Values',deviceId,IPToIDMapList[deviceId],PDC_DETAILS[SOURCE_DETAILS][z][i][1],PDC_DETAILS[SOURCE_DETAILS][z][i][2]])
		else:
			result_writer.writerow(['',deviceId,IPToIDMapList[deviceId],PDC_DETAILS[SOURCE_DETAILS][z][i][1],PDC_DETAILS[SOURCE_DETAILS][z][i][2]])
	
def populateDestDeviceDetailsInCSV(result_writer,z):

	destDeviceCount = PDC_DETAILS[PDC_SETUP][z][0][5]
	
	for i in range(0,destDeviceCount):

		deviceId = PDC_DETAILS[DESTINATION_DETAILS][z][i][0]
		
		if(i == 0):

			result_writer.writerow(['DEST DEIVICE FIELDS VALUES',IPToIDMapList[deviceId],PDC_DETAILS[DESTINATION_DETAILS][z][i][1]])
		else:
			result_writer.writerow(['',IPToIDMapList[deviceId],PDC_DETAILS[DESTINATION_DETAILS][z][i][1]])
#-----------------------------------------------------------------------------------------------------------------------------------	
def deployPDC(ipIndex,pmuIndex):	
	
	print '\nDeploy PDCs\n'
	print '.........................\n............................................' 

	createPDCFileNames()
	ipIndex = populateDataInPDCFile(ipIndex,pmuIndex)	
	NoOfPDC = len(PDC_DETAILS[PDC_SETUP])

	for i in range(0,NoOfPDC):

		deviceId = PDC_DETAILS[PDC_SETUP][i][0][0] # get the idcode
		ipAdress = IPToIDMapList[deviceId]  # get the ipaddress of the idcode

		tempList = [] # Get the index corresponding to the ipadress in the ipList
		tempList.append(ipList) 			  			
		lst = map(itemgetter(IP), tempList)
		ind = lst[0].index(ipAdress)
		tempList = []

		username = ipList[NAME][ind]
		password = ipList[PASSWD][ind]

		ssh_client = createSSHConnection(ipAdress,username,password) # create the client connections for PDC deployment
		
		runiPDCOnRemoteMachine(i,ind,deviceId,ssh_client)	
	
#---------------------------------------------------------------------------------------------------------------------------------
def runiPDCOnRemoteMachine(i,ind,deviceId,ssh_client):

	print 'Deploy iPDC ',deviceId,'on ', IPToIDMapList[deviceId]
	print '.............................'
	userName = ipList[NAME][ind]
	ipdcCSVFileName = pdcFileNames[deviceId]
# Create Remote diectory for WAMS
	s = 'cd /home/' + userName + ' && mkdir -m 777 WAMS'
#  	createDirWithPermissions(ssh_client,s)
	s = 'cd /home/' + userName + ' && mkdir -m 777 '+ REMOTE_PDC_PATH 
  	createDirWithPermissions(ssh_client,s)

# Remote copy iPDC setup file	
	s1 = LOCAL_PDC_CSV_PATH + ipdcCSVFileName
	s2 = '/home/'+userName+'/'+REMOTE_PDC_PATH+ipdcCSVFileName
	copyFileToRemoteMachine(ssh_client,s1,s2)

# Remote copy iPDC EXE file
	s1 = 'bin/iPDC'
	s2 = '/home/'+userName+'/'+REMOTE_PDC_PATH+'iPDC'
	copyFileToRemoteMachine(ssh_client,s1,s2)
# Run iPDC on remote machine
	print 'Start running iPDC ...............'			
	s = 'cd /home/'+userName+'/'+REMOTE_PDC_PATH+' && chmod 777 '+ ipdcCSVFileName+' && chmod 777 iPDC'#+' &&./iPDC '+str(deviceId)
	print s
	ssh_client.exec_command(s)	
	s = 'cd /home/'+userName+'/'+REMOTE_PDC_PATH+ ' && ./iPDC '+ deviceId +' &'
	print s
	stdin, stdout, stderr =  ssh_client.exec_command(s)		
        exit_status = stdout.channel.recv_exit_status()
	print stdout
	x = stdout.readlines()
	print x
	for line in x:
		print line
	ssh_client.close()
