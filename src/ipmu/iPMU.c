// ./iPMU 50 5000 5001 AUTO 25 
// ./iPMU 46 4600 4601 csV /home/nitesh/Desktop/newiPDC/distributed_iPDC/iPMU46.csv

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <math.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/signal.h>
#include <unistd.h>
#include "iPMU.h"
#include "function.h"
#include "connection.h"
#include <openssl/aes.h>
//#include "enc.h"

#define MAXBUFLEN  1600

const static unsigned char aes_key[]={0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF};


AES_KEY enc_key;
int pmuse = 0;
long int prev_soc = 0;

float phasorV_meg[] = {63.48,63.50,63.52};	// Voltage nominal as 63.5V
float phasorI_meg[] = {0.998,1.0,0.994};	// Current nominal as 1.0Amp
float phase_ang[] 	= {-120.6,0.65,120.45}; 
  /* Encrypt the plaintext */


  

/* ----------------------------------------------------------------------------	*/
/* FUNCTION  create_cfg(int,int,int,int)* : Default iPMU					*/
/* It generate the Configuration Frame as per user entered information.		*/
/* ----------------------------------------------------------------------------	*/

void auto_create_cfg(int pmuID,int Frequency)
{
 
     char buff[20],stn[16],c=' ';
     int i,j,k,frmt,indx=0;
     unsigned char temp1[1],temp2[2];
     unsigned char temp3[3],temp4[4];
     char *channelNames[] = {"V1","V2","V3","I1","I2","I3"};
	
	/* Allocate the memory for the ConfigurationFrame object and assigned fields */
	cfg_info = malloc(sizeof(struct ConfigurationFrame));
	cfg_info->cfg_STNname = malloc(16);
	memset(cfg_info->cfg_STNname,'\0',16);

	cfg_info->cfg_pmuID = pmuID;
	cfg_info->cfg_phnmr_val = 6;	// Default & Fixed
	cfg_info->cfg_annmr_val = 0;
	cfg_info->cfg_dgnmr_val = 0;
	cfg_info->cfg_dataRate = Frequency;

	if(Frequency == 50)
		cfg_info->cfg_fnom = 1;
	else
		cfg_info->cfg_fnom = 0;

	memset(stn,'\0',16);
	strcpy(stn,"iPMU-");
	sprintf(buff,"%d",cfg_info->cfg_pmuID);
	strcat(stn,buff);
	i = strlen(stn);

	for(; i<16; i++)
	{
		stn[i] =  c;
	}
	strcpy(cfg_info->cfg_STNname,stn);

	/* Allocate the memory for Phasor channel names */
	cfg_info->cfg_phasor_channels = (char *)malloc((cfg_info->cfg_phnmr_val*16) * sizeof(char));
	memset(cfg_info->cfg_phasor_channels,'\0',sizeof(cfg_info->cfg_phasor_channels));	

	for(i=0,k=0; i<cfg_info->cfg_phnmr_val; i++)
	{
		memset(stn,'\0',16);
		strcpy(stn,channelNames[i]);
		j = strlen(stn);
		
		for(; j<16; j++)
		{
			stn[j] =  c;
		}

		for(j=0,k; j < 16 ; j++,k++)
		{
			cfg_info->cfg_phasor_channels[k] = stn[j];
		}
	}

	/* Calculate the total size of CFG Frame */
	cfg2_frm_size = 0;
	cfg2_frm_size = 54 + (16*cfg_info->cfg_phnmr_val) + (4*cfg_info->cfg_phnmr_val); // No Analog & Digital?

	/* Store FORMAT bits from global data structure */
	cfg_info->cfg_fdf = 1; // Frequency : Floating Point
	cfg_info->cfg_af  = 1; // Analog : Floating Point
	cfg_info->cfg_pf  = 1; // Phasor : Floating Point
	cfg_info->cfg_pn  = 1; // Phasor Type : Polar

	frmt = 15;

	data_frm_size = 18; // Fixed fields sixe
	data_frm_size = data_frm_size + (8*cfg_info->cfg_phnmr_val) + 8; // 8:FREQ+DFREQ;

	/* Insert the fields in CFG Frame */
	memset(cfg2_frm,'\0',sizeof(cfg2_frm));

	/* Sync Word */
	cfg2_frm[indx++] = 0xAA;
	cfg2_frm[indx++] = 0x31;

	/* Frame Size */
	i2c(cfg2_frm_size,temp2);
	B_copy(cfg2_frm,temp2,indx,2);
	indx = indx + 2;

	/* iPMU ID */
	i2c(cfg_info->cfg_pmuID,temp2);
	B_copy(cfg2_frm,temp2,indx,2);
	indx = indx + 2;

	/* SOC */
	indx = indx + 4;	
	
	/* Time Quality Fields + Fraction of Second */
	indx = indx + 4;	
	
	/* Time Base */
	li2c(TB,temp4);
	B_copy(cfg2_frm,temp4,indx,4);
	indx = indx + 4;

	/* Number of PMU: Static 1 */
	i2c(1,temp2);
	B_copy(cfg2_frm,temp2,indx,2);
	indx = indx + 2;
		
	/* Station Name */
	B_copy(cfg2_frm,(unsigned char *)cfg_info->cfg_STNname,indx,16);
	indx = indx + 16;
		
	/* Pmu ID */
	i2c(cfg_info->cfg_pmuID,temp2);
	B_copy(cfg2_frm,temp2,indx,2);
	indx = indx + 2;

	/* Format Word */
	i2c(frmt,temp2);
	B_copy(cfg2_frm,temp2,indx,2);
	indx = indx + 2;
	
	/* PHNMR: Number of Phasor */
	i2c(cfg_info->cfg_phnmr_val,temp2);
	B_copy(cfg2_frm,temp2,indx,2);
	indx = indx + 2;

	/* ANNMR: Number of Analog */
	i2c(cfg_info->cfg_annmr_val,temp2);
	B_copy(cfg2_frm,temp2,indx,2);
	indx = indx + 2;

	/* DGNMR: Number of Digital */
	i2c(cfg_info->cfg_dgnmr_val,temp2);
	B_copy(cfg2_frm,temp2,indx,2);
	indx = indx + 2;
	
	/* Phasor Channel Names */
	B_copy(cfg2_frm,(unsigned char *)cfg_info->cfg_phasor_channels,indx,16*cfg_info->cfg_phnmr_val);
	indx = indx + 16*cfg_info->cfg_phnmr_val;

	/* Digital Channel Names */
	B_copy(cfg2_frm,(unsigned char *)cfg_info->cfg_analog_channels,indx,16*cfg_info->cfg_annmr_val);
	indx = indx + 16*cfg_info->cfg_annmr_val;

	/* Digital Channel Names */
	B_copy(cfg2_frm,(unsigned char *)cfg_info->cfg_digital_channels,indx,16*16*cfg_info->cfg_dgnmr_val);
	indx = indx + 16*16*cfg_info->cfg_annmr_val;
	
	/* FACTOR VALUES for Phasor,in this case its '1' only */
	for(i=0; i<cfg_info->cfg_phnmr_val-3; i++)	
	{
		cfg2_frm[indx++] = 0; // 0-Voltage
		cfg2_frm[indx++] = 0;
		cfg2_frm[indx++] = 0;
		cfg2_frm[indx++] = 1;
	}

	for(; i<cfg_info->cfg_phnmr_val; i++)	
	{
		cfg2_frm[indx++] = 1;	// 1-Current
		cfg2_frm[indx++] = 0;
		cfg2_frm[indx++] = 0;
		cfg2_frm[indx++] = 1;
	}

	/* Nominal Frequency */
	i2c(cfg_info->cfg_fnom,temp2);
	B_copy(cfg2_frm,temp2,indx,2);
	indx = indx + 2;

	/* Configuration Count: Static 0 */
	i2c(0,temp2);
	B_copy(cfg2_frm,temp2,indx,2);
	indx = indx + 2;
	
	/* Data Rate */
	i2c(cfg_info->cfg_dataRate,temp2);
	B_copy(cfg2_frm,temp2,indx,2);
	indx = indx + 2;
	
	/* Current SOC */
	gettimeofday(&tim,NULL);
	soc = (long) tim.tv_sec;
	li2c(soc,temp4);
	B_copy(cfg2_frm,temp4,6,4);

	/* Time Quality Fields */
	temp1[0] = 0;	
	B_copy(cfg2_frm,temp1,10,1);
	
	/* Fraction of Second */
	fracsec = tim.tv_usec;
	li2c_3byte(fracsec,temp3);
	B_copy(cfg2_frm,temp3,11,4);
	
	/* Calculation & insert the checksum VALUE of new CFG frame (up to now) */
	chk = compute_CRC(cfg2_frm,indx);

	cfg2_frm[indx++] = (chk >> 8) & ~(~0<<8);  	/* CHKSUM high byte */
	cfg2_frm[indx++] = (chk ) & ~(~0<<8);     	/* CHKSUM low byte  */

	printf("\nAUTO:\tCFG Frame size = %d-Bytes,\n",cfg2_frm_size);
	printf("\tData Frame size = %d-Bytes.\n",data_frm_size);
	
	
	printf("\n Before encryption:-");
	BIO_dump_fp (stdout, (const char *)cfg2_frm, cfg2_frm_size);

	
	unsigned char iv[AES_BLOCK_SIZE];
	memset(iv, 0x00, AES_BLOCK_SIZE);
	AES_KEY enc_key,dec_key;
	cfg2_frm[175]=0x00;
	cfg2_frm[176]=0x00;

	//unsigned char aes_input[]={0x0,0x1,0x2,0x3,0x4,0x5};
	unsigned char enc_out[cfg2_frm_size+2];
	unsigned char dec_out[cfg2_frm_size+2];
	AES_set_encrypt_key(aes_key, sizeof(aes_key)*8, &enc_key);
	AES_cbc_encrypt(cfg2_frm, enc_out, cfg2_frm_size+2, &enc_key, iv, AES_ENCRYPT);
	printf("\n");
	BIO_dump_fp (stdout, (const char *)enc_out, cfg2_frm_size+2);
	
	//strncpy(cfg2_frm,enc_out,cfg2_frm_size+2);
	//int i;

	for(i=0;i<=176;i++)
	{
	cfg2_frm[i]=enc_out[i];
	}
	printf("\n hi");
	cfg2_frm_size=cfg2_frm_size+2;
	BIO_dump_fp (stdout, (const char *)cfg2_frm, cfg2_frm_size);

	//gettimeofday(&ste,NULL);

	
	//gettimeofday(&ete,NULL);

	//int elapsed_e=((ete.tv_sec - ste.tv_sec)*1000000)+(ete.tv_usec-ste.tv_usec);
  	
  	printf("\n");
	
}


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  create_cfg(int,int,int,int)* : Default iPMU					*/
/* It generate the Configuration Frame as per user entered information.		*/
/* ----------------------------------------------------------------------------	*/

int csv_create_cfg(int pmuID,char *filePath)
{
	int  tempi,i,j,k;
	int indx,frmt;
	char *d1,*rline=NULL;
	char stn[16],buff[10],c=' ';
	unsigned char temp1[1],temp2[2];
	unsigned char hex,temp3[3],temp4[4];

	size_t len = 0;
	ssize_t read;

	/* Open the saved PMU Setup File and read the CFG frame if any? */
	fp_csv = fopen (filePath,"r");

	if (fp_csv != NULL)
	{
		read = getline(&rline,&len,fp_csv);	// For "Configuration Fields"
		read = getline(&rline,&len,fp_csv); // For "Configuration Items"

		if(read == 0)
			return 1;	// Return 1 if file is empty?
		else
		{
			/* Allocate the memory for the ConfigurationFrame object and assigned fields */
			cfg_info = malloc(sizeof(struct ConfigurationFrame));

			cfg_info->cfg_pmuID = pmuID;
			
			cfg_info->cfg_STNname = malloc(16);
			memset(cfg_info->cfg_STNname,'\0',16);
			strcpy(stn,"iPMU-");
			sprintf(buff,"%d",cfg_info->cfg_pmuID);
			strcat(stn,buff);
			for (i=strlen(stn);i<16;i++)
				stn[i] = c;
			strncpy(cfg_info->cfg_STNname,stn,16);

			d1 = strtok (rline,",");
			//printf("\n%s.\niPMU ID = %d\nStation Name = %s\n",d1,cfg_info->cfg_pmuID,cfg_info->cfg_STNname);

			d1 = strtok (NULL,",");
			tempi = atoi(d1);
			if(tempi == 50)
				cfg_info->cfg_fnom = 1;
			else
				cfg_info->cfg_fnom = 0;
			//printf("Frequency = %d\n",tempi);

			d1 = strtok (NULL,",");
			tempi = atoi(d1);
			cfg_info->cfg_dataRate = tempi;
			//printf("Data Rate = %d\n",tempi);

			d1 = strtok (NULL,",");
			tempi = atoi(d1);
			cfg_info->cfg_phnmr_val = tempi;
			//printf("Phasors = %d\n",tempi);
		
			d1 = strtok (NULL,",");
			tempi = atoi(d1);
			cfg_info->cfg_annmr_val = tempi;
			//printf("Analogs = %d\n",tempi);

			d1 = strtok (NULL,",");
			tempi = atoi(d1);
			cfg_info->cfg_dgnmr_val = tempi;
			//printf("Digitals = %d\n",tempi);

			// Identify the data format polar,rectangular,floating/fixed point?	
			d1 = strtok (NULL,",");
			frmt = atoi(d1);
			i2c(frmt,temp2);
			hex = temp2[1];
			hex <<= 4;
			if((hex & 0x80) == 0x80) cfg_info->cfg_fdf = 1; else cfg_info->cfg_fdf = 0;
			if((hex & 0x40) == 0x40) cfg_info->cfg_af  = 1; else cfg_info->cfg_af  = 0;
			if((hex & 0x20) == 0x20) cfg_info->cfg_pf  = 1; else cfg_info->cfg_pf  = 0;
			if((hex & 0x10) == 0x10) cfg_info->cfg_pn  = 1; else cfg_info->cfg_pn  = 0;
			
			/* Allocate the memory for Phasor channel names */
			cfg_info->cfg_phasor_channels = (char *)malloc((cfg_info->cfg_phnmr_val*16) * sizeof(char));
			memset(cfg_info->cfg_phasor_channels,'\0',sizeof(cfg_info->cfg_phasor_channels));	

			/* Allocate the memory for Phasor channels PHUNIT */
			cfg_info->cfg_phasor_phunit = (unsigned char *)malloc((cfg_info->cfg_phnmr_val*4) * sizeof(unsigned char));
			memset(cfg_info->cfg_phasor_phunit,'\0',sizeof(cfg_info->cfg_phasor_phunit));	

			for (i=0,k=0; i<cfg_info->cfg_phnmr_val; i++)
			{
				d1 = strtok (NULL,",");
				memset(stn,'\0',16);
				strcpy(stn,d1);
				j = strlen(stn);
				
				for (; j<16; j++)
				{
					stn[j] = c;
				}
				strncat(cfg_info->cfg_phasor_channels,stn,16);						

				d1 = strtok (NULL,",");
				if (!strcasecmp(d1,"v"))
					cfg_info->cfg_phasor_phunit[k++] = 0x00; // Voltage
				else
					cfg_info->cfg_phasor_phunit[k++] = 0x01; // Current
				
				d1 = strtok (NULL,",");
				li2c_3byte (atol(d1),temp3);
				cfg_info->cfg_phasor_phunit[k++] = temp3[0];
				cfg_info->cfg_phasor_phunit[k++] = temp3[1];
				cfg_info->cfg_phasor_phunit[k++] = temp3[2];
			}
			
			/* Allocate the memory for Analog channel names */
			cfg_info->cfg_analog_channels = (char *)malloc((cfg_info->cfg_annmr_val*16) * sizeof(char));
			memset(cfg_info->cfg_analog_channels,'\0',sizeof(cfg_info->cfg_analog_channels));	

			/* Allocate the memory for Analog channels ANUNIT */
			cfg_info->cfg_analog_anunit = (unsigned char *)malloc((cfg_info->cfg_annmr_val*4) * sizeof(unsigned char));
			memset(cfg_info->cfg_analog_anunit,'\0',sizeof(cfg_info->cfg_analog_anunit));	

			for (i=0,k=0; i<cfg_info->cfg_annmr_val; i++)
			{
				d1 = strtok (NULL,",");
				memset(stn,'\0',16);
				strcpy(stn,d1);
				j = strlen(stn);
				
				for (; j<16; j++)
				{
					stn[j] = c;
				}
				strncat(cfg_info->cfg_analog_channels,stn,16);						
				
				d1 = strtok (NULL,",");
				if (!strcasecmp(d1,"rms"))
					cfg_info->cfg_analog_anunit[k++] = 0x00; // RMS
				else if (!strcasecmp(d1,"pow"))
					cfg_info->cfg_analog_anunit[k++] = 0x01; // POW
				else
					cfg_info->cfg_analog_anunit[k++] = 0x10; // Peak
				
				d1 = strtok (NULL,",");
				li2c_3byte (atol(d1),temp3);
				cfg_info->cfg_analog_anunit[k++] = temp3[0];
				cfg_info->cfg_analog_anunit[k++] = temp3[1];
				cfg_info->cfg_analog_anunit[k++] = temp3[2];
			}
			
			/* Close the file as there is no more channels! */
			fclose(fp_csv);

			/* Calculate the total size of CFG Frame */
			cfg2_frm_size = 0;
			cfg2_frm_size = 54 + (16*cfg_info->cfg_phnmr_val) + (16*cfg_info->cfg_annmr_val)+ (16*cfg_info->cfg_dgnmr_val*16) + (4*cfg_info->cfg_phnmr_val) + (4*cfg_info->cfg_annmr_val) + (4*cfg_info->cfg_dgnmr_val);

			/* Calculate the total size of Data Frame */
			data_frm_size = 18; // Fixed fields size

			/* Calculate 4/8 bytes for each PHNMR */
			if (cfg_info->cfg_pf == 0) 
			{
				data_frm_size = data_frm_size + (4*cfg_info->cfg_phnmr_val);
			}
			else 
			{
				data_frm_size = data_frm_size + (8*cfg_info->cfg_phnmr_val);
			}

			/* Calculate 2/4 bytes for each ANNMR */
			if (cfg_info->cfg_af == 0) 
			{
				data_frm_size = data_frm_size + (2*cfg_info->cfg_annmr_val);
			}
			else 
			{
				data_frm_size = data_frm_size + (4*cfg_info->cfg_annmr_val);
			}

			/* Calculate 2/4 bytes for both (FREQ + DFREQ) */
			if (cfg_info->cfg_fdf == 0) 
			{				
				data_frm_size = data_frm_size + 4;
			}
			else 
			{
				data_frm_size = data_frm_size + 8;
			}

			/* Calculate 2 bytes for each DGNMR */
			data_frm_size = data_frm_size + (2*cfg_info->cfg_dgnmr_val);

			/* Insert the fields in new CFG Frame: */
			memset(cfg2_frm,'\0',sizeof(cfg2_frm));

			/* sync word */
			indx = 0;
			cfg2_frm[indx++] = 0xAA; 
			cfg2_frm[indx++] = 0x31; 

			/* Frame Size */
			i2c(cfg2_frm_size,temp2);
			B_copy(cfg2_frm,temp2,indx,2);
			indx = indx + 2;

			/* PMU ID */
			i2c(cfg_info->cfg_pmuID,temp2);
			B_copy(cfg2_frm,temp2,indx,2);
			indx = indx + 2;

			/* SOC */				
			indx = indx + 4;

			/* Fraction of Second */
			indx = indx + 4;

			/* Time Base is static 1000000μs */
			li2c(1000000,temp4);
			B_copy(cfg2_frm,temp4,indx,4);
			indx = indx + 4;

			/* Number of PMU: static "0001" */
			i2c(1,temp2);
			B_copy(cfg2_frm,temp2,indx,2);
			indx = indx + 2;

			/* Station Name */
			B_copy(cfg2_frm,(unsigned char *)cfg_info->cfg_STNname,indx,16);
			indx = indx + 16;

			/* PMU ID */					
			i2c(cfg_info->cfg_pmuID,temp2);
			B_copy(cfg2_frm,temp2,indx,2);
			indx = indx + 2;

			/* Format Word */
			i2c(frmt,temp2);
			B_copy(cfg2_frm,temp2,indx,2);
			indx = indx + 2;

			/* PHNMR: Number of Phasors */
			i2c(cfg_info->cfg_phnmr_val,temp2);
			B_copy(cfg2_frm,temp2,indx,2);
			indx = indx + 2;

			/* ANNMR: Number of Analogs */
			i2c(cfg_info->cfg_annmr_val,temp2);
			B_copy(cfg2_frm,temp2,indx,2);
			indx = indx + 2;

			/* DGNMR: Number of Digitals */
			i2c(cfg_info->cfg_dgnmr_val,temp2);
			B_copy(cfg2_frm,temp2,indx,2);
			indx = indx + 2;
			
			/* Phasor channel Names */ 
			B_copy(cfg2_frm,(unsigned char *)cfg_info->cfg_phasor_channels,indx,16*cfg_info->cfg_phnmr_val);
			indx = indx + 16*cfg_info->cfg_phnmr_val;

			/* Analog channel Names */ 
			B_copy(cfg2_frm,(unsigned char *)cfg_info->cfg_analog_channels,indx,16*cfg_info->cfg_annmr_val);
			indx = indx + 16*cfg_info->cfg_annmr_val;

			/* Digital channel Names */ 
			B_copy(cfg2_frm,(unsigned char *)cfg_info->cfg_digital_channels,indx,16*cfg_info->cfg_dgnmr_val);
			indx = indx + 16*cfg_info->cfg_dgnmr_val;
			
			/* FACTOR VALUES for Phasor */
			B_copy(cfg2_frm,cfg_info->cfg_phasor_phunit,indx,4*cfg_info->cfg_phnmr_val);
			indx = indx + 4*cfg_info->cfg_phnmr_val;
			
			/* FACTOR VALUES for Analog */
			B_copy(cfg2_frm,cfg_info->cfg_analog_anunit,indx,4*cfg_info->cfg_annmr_val);
			indx = indx + 4*cfg_info->cfg_annmr_val;

			/* FACTOR VALUES for Digital */
/*			B_copy(cfg2_frm,cfg_info->cfg_digital_dgunit,indx,4*cfg_info->cfg_dgnmr_val);
			indx = indx + 4*cfg_info->cfg_dgnmr_val; */
			
			/* Nominal Frequency */
			i2c(cfg_info->cfg_fnom,temp2);
			B_copy(cfg2_frm,temp2,indx,2);
			indx = indx + 2;
			
			/* Configuration Count: Static '0000' */
			i2c(0,temp2);
			B_copy(cfg2_frm,temp2,indx,2);
			indx = indx + 2;
			
			/* Data Rate */
			i2c(cfg_info->cfg_dataRate,temp2);
			B_copy(cfg2_frm,temp2,indx,2);
			indx = indx + 2;
			
		   	/* SOC */
		   	gettimeofday(&tim,NULL);
		   	soc = (long) tim.tv_sec;
		   	li2c(soc,temp4);
		   	B_copy(cfg2_frm,temp4,6,4);

		   	/* Time quality fields */
		   	temp1[0] = 0; 
		   	B_copy(cfg2_frm,temp1,10,1);

		   	/* Fraction of Second */
		   	fracsec = tim.tv_usec;
		   	li2c(fracsec,temp3);
		   	B_copy(cfg2_frm,temp3,11,3);

			/* CRC-Checksum (up to now) */
			chk = compute_CRC(cfg2_frm,indx);
			cfg2_frm[indx++] = (chk >> 8) & ~(~0<<8);  	/* CHKSUM high byte */
			cfg2_frm[indx++] = (chk ) & ~(~0<<8);     	/* CHKSUM low byte  */

			printf("CFG Frame size  = %d.\n",cfg2_frm_size);
			//printf("Data Frame size = %d-Bytes.\n",data_frm_size);
		}		
	}
	else
	{
		printf("CSV File not present in the system!\n");
	}
	
	return 0;
}


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  generate_data_frame():	               					*/
/* Function to generate the data frame. Based on the Configuration Frame 	     */
/* attributes.                          					               */
/* ----------------------------------------------------------------------------	*/

void generate_data_frame()
{
	float f,valf;
	long int vall;
	int j,k,indx=0,vali;
     unsigned char temp1[1],temp2[2];
     unsigned char temp3[3],temp4[4];

     
     struct timespec *cal_timeSpec;
     cal_timeSpec = malloc(sizeof(struct timespec));
	
	memset(data_frm,'\0',sizeof(data_frm_size));

	/* Insert SYNC Word in data frame */
	data_frm[indx++] = 0xAA; 
	data_frm[indx++] = 0x01; 

	/* Insert data frame size in data frame */
	i2c(data_frm_size,temp2);
	B_copy(data_frm,temp2,indx,2);
	indx = indx + 2;

	/* Insert PMU ID in data frame */
	i2c(cfg_info->cfg_pmuID,temp2);
	B_copy(data_frm,temp2,indx,2);
	indx = indx + 2;

	/* Insert SOC value in data frame */
     /* No PPS,so have to manage by seeing local time */
     clock_gettime(CLOCK_REALTIME,cal_timeSpec);
     
	if (fsecNum >= cfg_info->cfg_dataRate)
	{	
		cal_timeSpec->tv_sec ++;
        	fsecNum = 0;
	}
	soc = (long)cal_timeSpec->tv_sec;
	li2c(soc,temp4);
	B_copy(data_frm,temp4,indx,4);
	indx = indx + 4;

     /* Insert Time Quality flag of one byte,default is zero,user setable in future? */
	temp1[0] = 0;
	B_copy(data_frm,temp1,indx,1);
	indx = indx + 1;

     /* Insert fraction of second in data frame of three bytes */
     fracsec = roundf(fsecNum*TB/cfg_info->cfg_dataRate);
	li2c_3byte(fracsec,temp3);
	B_copy(data_frm,temp3,indx,3);
	indx = indx + 3;
	fsecNum += 1;   

	/* Insert STAT Word in data frame Default or Changed */
	time_t curnt_soc = time(NULL);

	if(pmuse == 0) 
	{
		prev_soc = curnt_soc;
	} 

	if((curnt_soc-prev_soc) > 1)
	{ 
		printf("\tSTAT word Changed due to PMU SYNC Error.");
		data_frm[indx++] = 0x20;
		data_frm[indx++] = 0x00;
	}
	else
	{
		/* If not insert default STAT Word: 0000 */
		data_frm[indx++] = 0x00;
		data_frm[indx++] = 0x00;
	}

	prev_soc = curnt_soc;
	pmuse = 1;

	/* iPMU Default Mode: Auto Generated Data */	
	if(data_mode == 0)
	{
		/* Floating Point Phasor: Polar Format */
		for(j=0; j<3; j++)
		{
			f2c(phasorV_meg[j],temp4);
			B_copy(data_frm,temp4,indx,4);
			indx = indx + 4;
			
			f2c(phase_ang[j],temp4);
			B_copy(data_frm,temp4,indx,4);
			indx = indx + 4;
		}

		for(j=0; j<3; j++)
		{
			f2c(phasorI_meg[j],temp1);
			B_copy(data_frm,temp4,indx,4);
			indx = indx + 4;
			
			f2c(phase_ang[j],temp4);
			B_copy(data_frm,temp4,indx,4);
			indx = indx + 4;
		}

	     if(cfg_info->cfg_fnom == 1)	// for 50 Hz measurements data
	     {
			/* Insert Floating point Frequency & DFrequency values in data frame */
			f = (rand() % 4 + 998);  /* Returns b/w 49.9 to 50.1 Hz */
			f = (f/1000)*50;
	     	f2c(f,temp1);
		     B_copy(data_frm,temp4,indx,4);
		     indx = indx + 4;

		     f = 0;//(rand() % 5 + 1);     // Only positive rate of change of frequency!
		     //f = f*0.0639;
		     f2c(f,temp1);
		     B_copy(data_frm,temp4,indx,4);
		     indx = indx + 4;
	     }
	     else		// for 60 Hz measurements data
	     {
			/* Insert Floating point Frequency & DFrequency values in data frame */
			f = (rand() % 4 + 998);  /* Returns b/w 59.9 to 60.1 Hz */
			f = (f/1000)*60;
		     f2c(f,temp1);
		     B_copy(data_frm,temp4,indx,4);
		     indx = indx + 4;

		     f = 0;//(rand() % 3 + 1);     // Only positive rate of change of frequency!
		     //f = f*0.0639;
		     f2c(f,temp1);
		     B_copy(data_frm,temp4,indx,4);
		     indx = indx + 4;
	     }
	
	} // End if,iPMU Default 

	/*----------------Read measurements from file------------------*/	
	else
	{
		char *mData,*d1,*d2;
		int j;

		mData = measurement_Return ();

		d1 = strtok (mData,","); 
		//d1 = strtok (NULL,","); 

		/* Fix Point Phasor Measurements */
		if(cfg_info->cfg_pf == 0)		
		{
			for(j=0,k=0; j<cfg_info->cfg_phnmr_val; j++)
			{
				k++;
				temp3[0] = cfg_info->cfg_phasor_phunit[k++];
				temp3[1] = cfg_info->cfg_phasor_phunit[k++];
				temp3[2] = cfg_info->cfg_phasor_phunit[k++];
				vall = c2li_3byte(temp3);

				d1 = strtok (NULL,","); 
				vali = atof(d1)*1e5/vall;
				//phasorI = (atof(d1)*100000/temp_PHUNIT_val[j]);
				i2c(vali,temp2);
				B_copy(data_frm,temp2,indx,2);
				indx = indx + 2;

				d2 = strtok (NULL,","); 
                if (cfg_info->cfg_pn==0){ // Rectangular format
                    vali = atof(d2)*1e5/vall;
                }
                else {   // Polar format
                    vali = (atof(d2)*M_PI/180)*1e4;
                };
				i2c(vali,temp2);
				B_copy(data_frm,temp2,indx,2);
				indx = indx + 2;
//printf("ph = %s-%s\n", d1, d2);
			}
		}
		else	      /* Floating Point Phasor */
		{
			for(j=0,k=0; j<cfg_info->cfg_phnmr_val; j++)
			{
				k++;
/*				temp3[0] = cfg_info->cfg_phasor_phunit[k++];
				temp3[1] = cfg_info->cfg_phasor_phunit[k++];
				temp3[2] = cfg_info->cfg_phasor_phunit[k++];
				vall = c2li_3byte(temp3);
*/
				d1 = strtok (NULL,","); 
				//valf = atof(d1)*vall;
				valf = atof(d1);
				f2c(valf,temp4);
				B_copy(data_frm,temp4,indx,4);
				indx = indx + 4;

				d2 = strtok (NULL,","); 
                if (cfg_info->cfg_pn==0){ // Rectangular format
                    valf = atof(d2);
                }
                else {   // Polar format
                    valf = (atof(d2)*M_PI/180);
                };
				f2c(valf,temp4);
				B_copy(data_frm,temp4,indx,4);
				indx = indx + 4;
			}
		}

		/* Fix Point Frequency & DFrequency */
		if(cfg_info->cfg_fdf == 0)
		{
			if(cfg_info->cfg_fnom == 1)
				vali = 50;
			else
				vali = 60;
				 
			d1 = strtok (NULL,",");
			vali = (atof(d1)-vali)*1e3;
			i2c(vali,temp2);
			B_copy(data_frm,temp2,indx,2);
			indx = indx + 2;

			d2 = strtok (NULL,","); 
			vali = (atof(d2)*1e2);
			i2c(vali,temp2);
			B_copy(data_frm,temp2,indx,2);
			indx = indx + 2;
//printf("FREQ = %s-%s\n", d1, d2);
		}
		else	      	/* Floating Point Frequency & DFrequency */
		{
			d1 = strtok (NULL,","); 
			valf = atof(d1);
			f2c(valf,temp4);
			B_copy(data_frm,temp4,indx,4);
			indx = indx + 4;

			d2 = strtok (NULL,","); 
			valf = atof(d2);
			f2c(valf,temp4);
			B_copy(data_frm,temp4,indx,4);
			indx = indx + 4;
		}

		/* Fix Point Analog */
		if(cfg_info->cfg_af == 0)
		{
			for(j=0,k=0; j<cfg_info->cfg_annmr_val; j++)
			{
				k++;
				temp3[0] = cfg_info->cfg_analog_anunit[k++];
				temp3[1] = cfg_info->cfg_analog_anunit[k++];
				temp3[2] = cfg_info->cfg_analog_anunit[k++];
				vall = c2li_3byte(temp3);

				d1 = strtok (NULL,",");
				vali = atof(d1)*1e5/vall;
				//analogI = (atof(d1)*1e5/temp_ANUNIT_val[j]);
				i2c(vali,temp2);
				B_copy(data_frm,temp2,indx,2);
				indx = indx + 2;
//printf("AN = %s\n", d1);
			}
		}
		else      /* Insert Floating point Analog values in data frame */
		{
			for(j=0,k=0; j<cfg_info->cfg_annmr_val; j++)
			{
				k++;
/*				temp3[0] = cfg_info->cfg_analog_anunit[k++];
				temp3[1] = cfg_info->cfg_analog_anunit[k++];
				temp3[2] = cfg_info->cfg_analog_anunit[k++];
				vall = c2li_3byte(temp3);
*/
				d2 = strtok (NULL,","); 
				valf = (atof(d2));
				f2c(valf,temp4);
				B_copy(data_frm,temp4,indx,4);
				indx = indx + 4;
			}
		}
	} /* end of measurements from file */
	
	/* Calculate and insert the Checksum value in data frame (till now) */
	chk = compute_CRC(data_frm,indx);

     data_frm[indx++] = (chk >> 8) & ~(~0<<8);  	/* CHKSUM high byte; */
     data_frm[indx++] = (chk ) & ~(~0<<8);     	/* CHKSUM low byte;  */

/*	printf("\nData Frame Size = %d : ",indx);
	for (i=0; i<indx; i++)
	{
		printf("%X-",data_frm[i]);
	}
*/
	unsigned char iv[AES_BLOCK_SIZE];
	memset(iv, 0x00, AES_BLOCK_SIZE);

	
	/* Buffers for Encryption and Decryption */
	unsigned char enc_out[80];
	AES_KEY enc_key;
	AES_set_encrypt_key(aes_key, sizeof(aes_key)*8, &enc_key);
	AES_cbc_encrypt(data_frm, enc_out, 80, &enc_key, iv, AES_ENCRYPT);
//gettimeofday(&ste,NULL);
//  gettimeofday(&ete,NULL);
// int elapsed_e=((ete.tv_sec - ste.tv_sec)*1000000)+(ete.tv_usec-ste.tv_usec);
//  printf("\n \n Time for encryption %d in miroseconds\n ",elapsed_e);


} // End of generate_data_frame;


/* ----------------------------------------------------------------------------	*/
/* FUNCTION  main():								                    */
/* ----------------------------------------------------------------------------	*/

int main(int argc,char **argv)
{
	if (argc != 6 )
	{
		printf("Passing arguments does not match with the iPMU inputs. Try Again?\n");
		exit(EXIT_SUCCESS);
	}

	int ret=-1;
	int id,frq;
	char *s;

	fsecNum = 0;
   	id = atoi(argv[1]);
	sprintf(logfilename,"log%d.txt",id); // KK
	logFile = fopen(logfilename,"a");
	char *eBuf = malloc(60);
	memset(eBuf,'\0',60);
	time_t mytime;
	mytime = time(NULL);
	char rr[40];
	strcpy(rr,ctime(&mytime));
	
	sprintf(eBuf,"Date and time %s",rr);	 
//	fputs(eBuf,logFile);
	fclose(logFile);		
	free(eBuf);

	if (!strcasecmp(argv[4],"AUTO"))
	{
		data_mode = 0;
		frq = atoi(argv[5]);
		auto_create_cfg(id,frq);
		
		/* Start the iPMU Server */
		server(id,atoi(argv[2]),atoi(argv[3]));
	}
	else if (!strcasecmp(argv[4],"CSV"))
	{
		data_mode = 1;
		s = argv[5];
		ret = csv_create_cfg(id,s);

		/* Start iPMU Server */
		if (ret == 0)
		{
			/* Open the CSV file */
			fp_csv = fopen (s,"r");

			/* Start the iPMU Server */
			server(id,atoi(argv[2]),atoi(argv[3]));
		}
	}
	else
	{
		printf("UNKNOWN Mode. Extiting iPMU\n");
		exit(EXIT_SUCCESS);
	}
	
	return 0;
	
} /* end of main */

/*************************************** End of Program ***********************************************/ 
