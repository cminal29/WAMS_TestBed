import paramiko
import sys
import os
import os.path
from functions import topologyRead,ipRead,checkAvaialbleMachines,displayTopology,displayPDCs
from deployPMU import deployAllPMUs
from deployPDC import deployPDC
import os

deployStatus = 0;
def main():

	os.system('cls' if os.name=='nt' else 'clear')

	print '-----------------------------'
	# print '*******************fdfaada',NoOfPMU

	toplogyfname='setup files/WAMSTopology.csv'
	ipfname='setup files/IPFile.csv'

	topologyRead(toplogyfname)
	displayTopology()
	displayPDCs()
	ipRead(ipfname)
	if(int(checkAvaialbleMachines()) == 0):
		
		print 'No Sufficient machines'
		sys.exit(0)
	else:
		print 'Sufficient machines'	

	ipIndex = 0
	pmuIndex = 0
	pdcIndex = 0

	ipIndex,pmuIndex = deployAllPMUs(ipIndex,pmuIndex)

	deployPDC(ipIndex,pmuIndex)
	print 'hiii'

if __name__ == "__main__":
	main()
	deployStatus = 1;
#print 'Enter e to exit'
#	while True:
#		reply = raw_input('Enter text:')
#		if(reply == 'e'):
#                      sys.exit(0)
			
