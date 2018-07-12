from globals import *
from functions import createSSHConnection,createDirWithPermissions,copyFileToRemoteMachine
from time import sleep
from math import floor

def deployAllPMUs(ipIndex,pmuIndex):
	
	COUNT[NoOfPMUsOnOneMachine]=1
	
	print COUNT[NoOfPMUsOnOneMachine]
	print '\nDeploy all PMUs\n'
	print '....................\n...................................'

	#NoOfMachinesForPMU = int(floor(COUNT[NoOfPMUs]/COUNT[NoOfPMUsOnOneMachine])) original
	NoOfMachinesForPMU = int(COUNT[NoOfPMUs])
	
	
	print '\n number of machines for PMU' + str(NoOfMachinesForPMU)
	for i in range(0,NoOfMachinesForPMU):

		ipIndex,pmuIndex = deploySetOfPMUs(COUNT[NoOfPMUsOnOneMachine],ipIndex,pmuIndex) 
		#ipIndex,pmuIndex = deploySetOfPMUs(COUNT[NoOfPMUs],ipIndex,pmuIndex) new not working properly
		
	PMUsRemaining = COUNT[NoOfPMUs]%COUNT[NoOfPMUsOnOneMachine]
	print PMUsRemaining
	if(PMUsRemaining != 0):

		ipIndex,pmuIndex = deploySetOfPMUs(PMUsRemaining,ipIndex,pmuIndex)

	print 'IP index',ipIndex,'pmuIndex',pmuIndex	
	return 	ipIndex,pmuIndex

def deploySetOfPMUs(n,ipIndex,pmuIndex):

# +++++++++++++++++++++++ SSH Connection +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	host = ipList[IP][ipIndex]

	print host

	remoteMachineLoginName = ipList[NAME][ipIndex]
	password = ipList[PASSWD][ipIndex]
	ssh_client = createSSHConnection(host,remoteMachineLoginName,password)
	
	print 'ssh_client' + str(ssh_client)

# +++++++++++++++++++++ Create Working Dir +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	s = 'cd /home/' + remoteMachineLoginName+ ' && mkdir -m 777 WAMS'
	createDirWithPermissions(ssh_client,s)  	

	s = 'cd /home/' + remoteMachineLoginName+ ' && mkdir -m 777 '+ REMOTE_PMU_PATH 
	createDirWithPermissions(ssh_client,s)  	

# ++++++++++++++++++++++++++ scp iPMU to remote machine and change permissions ++++++++++++++++++++++++ 	

	print 'Copy iPMU to'+'/home/'+remoteMachineLoginName+'/'+REMOTE_PMU_PATH
	s1 = 'bin/iPMU'
	
	s2 = '/home/'+remoteMachineLoginName+'/'+REMOTE_PMU_PATH+'iPMU'
	copyFileToRemoteMachine(ssh_client,s1,s2)
#	sleep(0.1)
	s = "chmod 777 /home/"+remoteMachineLoginName+'/'+REMOTE_PMU_PATH+'iPMU'
	ssh_client.exec_command(s)

	

#******************* Start the instances of iPMU with arguments from the file*******************
	sshCount = 0	
	Count = 0;

	print topology[TYPE][pmuIndex]

	print str(topology[DATAINPUT][pmuIndex])

	while(topology[TYPE][pmuIndex] == 'PMU'):

		if(str(topology[DATAINPUT][pmuIndex]) == 'AUTO'):
			
			pmuIdcode = topology[IDCODE][pmuIndex]
			udpPort = topology[UDPPORT][pmuIndex]
			tcpPort =  topology[TCPPORT][pmuIndex]				
			dataInputType = topology[DATAINPUT][pmuIndex]
			dataRate = topology[DATARATE][pmuIndex]
			s = 'cd /home/'+remoteMachineLoginName+'/'+ REMOTE_PMU_PATH +' && ./iPMU '+pmuIdcode+' '+ udpPort+' '+tcpPort+' '+ dataInputType +' '+ dataRate
			print s
			
			IPToIDMapList[topology[IDCODE][pmuIndex]] = ipList[IP][ipIndex]
			sshCount = sshCount +1
			if(sshCount == 5):
				ssh_client.close()
				sshCount = 0;
				ssh_client = createSSHConnection(host,remoteMachineLoginName,password)	

			stdin, stdout, stderr = ssh_client.exec_command(s)
			pmuIndex = pmuIndex + 1				
			Count = Count + 1
			if(Count == n):
				break;	
#			sleep(0.1)
			continue

		if(str(topology[DATAINPUT][pmuIndex]) == 'FILE'):	

			print ipList[IP][ipIndex]

			IPToIDMapList[topology[IDCODE][pmuIndex]] = ipList[IP][ipIndex]
			fName = topology[FILENAME][pmuIndex]
			
			print LOCAL_PMU_CSV_PATH
			print fName

			s1 = LOCAL_PMU_CSV_PATH + fName
			s2 = '/home/'+remoteMachineLoginName+'/'+REMOTE_PMU_PATH+fName
			copyFileToRemoteMachine(ssh_client,s1,s2)
			pmuIdcode = topology[IDCODE][pmuIndex]
			udpPort = topology[UDPPORT][pmuIndex]
			tcpPort =  topology[TCPPORT][pmuIndex]				
			dataInputType = 'CSV'
			s = 'cd /home/'+remoteMachineLoginName+'/'+REMOTE_PMU_PATH+' && chmod 777 '+ fName+' &&./iPMU '+pmuIdcode+' '+ udpPort+' '+tcpPort+' '+ dataInputType +' '+fName
			print s

			#sshCount = sshCount +1

			#if(sshCount == 5):
			#	ssh_client.close()
			#	ssh_client = createSSHConnection(host,remoteMachineLoginName,password)	
			#	sshCount = 0

			ssh_client.exec_command(s)
#					sleep(0.1)	
			pmuIndex = pmuIndex + 1		
			Count = Count + 1
			
			print str(pmuIndex)
			print str(Count) 

			if(Count == n):
				break;	

			else : 'problem'		
	ssh_client.close()
	ipIndex = ipIndex + 1  # Now assign remaining IP addresses to PDCs

	return ipIndex,pmuIndex

