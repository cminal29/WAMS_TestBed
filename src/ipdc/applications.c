/* ----------------------------------------------------------------------------- 
 * align_sort.c
 * 
 * iPDC - Phasor Data Concentrator
 *
 * Copyright (C) 2011-2012 Nitesh Pandit
 * Copyright (C) 2011-2012 Kedar V. Khandeparkar
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Authors: 
 *		Nitesh Pandit <panditnitesh@gmail.com>
 *		Kedar V. Khandeparkar <kedar.khandeparkar@gmail.com>			
 *
 * ----------------------------------------------------------------------------- */

#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <math.h>       /* fabs */
#include "parser.h"
#include "global.h" 
#include "align_sort.h" 
#include "dallocate.h"
#include "applications.h"


/* ----------------------------------------------------------------------------		*/
/* FUNCTION  averageFrequency():                               	     				*/
/* Computes average of frequency from all PMUs 										*/
/* ----------------------------------------------------------------------------		*/

float averageFrequency(int i) {

	struct data_frame *df = TSB[index].first_data_frame
	struct cfg_frame *temp_cfg = cfgfirst;
	float tempFreq = 0,avgFrequency;
	int noOfPMU,j,count = 0;

	while(temp_cfg != NULL){
		
		noOfPMU = to_intconvertor(temp_cfg->num_pmu);	 /
		count += noOfPMU;

		if(df != NULL) {
			
			if(temp_cfg->pmu[j]->fmt->freq == '1') {

				for(j=0;j<noOfPMU;j++) {		  					
				
					tempFreq += decode_ieee_single(df->dpmu[j]->freq); 					
				}

			} else {

				for(j=0;j<noOfPMU;j++) {		  					

					tempFreq = to_intconvertor(df->dpmu[j]->freq); 					
				}
			}	
			df = df->dnext;
		}
		temp_cfg = temp_cfg->cfgnext;	
	}	
	avgFrequency = tempFreq/count;	
	return avgFrequency;
}


/* ----------------------------------------------------------------------------		*/
/* FUNCTION  frequencyInstability():                           	     				*/
/* Computes weighted average of frequency from some PMUs 							*/
/* ----------------------------------------------------------------------------		*/

int frequencyInstability(int i) {

	struct data_frame *df = TSB[index].first_data_frame
	struct cfg_frame *temp_cfg = cfgfirst;

	float tempFreq = 0,tempFc=0,totalFreq =0,totalHi=0;
	float timeDiff = 20*1e-6
	float deltaFreq = 0;
	int noOfPMU = 0,totalPMUs=0,j;

	while(temp_cfg != NULL){

			totalPMUs += to_intconvertor(temp_cfg->num_pmu);	
			temp_cfg = temp_cfg->cfgnext;
	}	

	int generatorPMUs = (totalPMUs*ratio) + 2;  // Considered 1/5*noofPMUs+2 frequencies
	int x = 0,*Hi;
	Hi = randomGenerator(minHi,maxHi,generatorPMUs);
	
	temp_cfg = cfgfirst;


	while(temp_cfg != NULL){
		
		noOfPMU = to_intconvertor(temp_cfg->num_pmu);	 /

		if(df != NULL) {
			
			if(temp_cfg->pmu[j]->fmt->freq == '1') {

				for(j=0;j<noOfPMU;j++) {		  					

					if(x == generatorPMUs)
						break;
					tempFreq = decode_ieee_single(df->dpmu[j]->freq); 
					totalFreq += Hi[x]*tempFreq;	// Sum of Hi*fi (numerator)
					totalHi += Hi[x];				// Sum of Hi (denominator)
					x++;					
				}

			} else {

				for(j=0;j<noOfPMU;j++) {		  					

					if(x == generatorPMUs)
						break;					
					tempFreq = to_intconvertor(df->dpmu[j]->freq); 
					totalFreq += Hi[x]*tempFreq; // Sum of Hi*fi (numerator)
					totalHi += Hi[x];   		  // Sum of Hi (denominator)
					x++;
				}
			}	
			df = df->dnext;
		}
		temp_cfg = temp_cfg->cfgnext;
		
	}

	free(Hi);

	if(fc == 0) {

		fc = totalFreq/totalHi;
		return 0;

	} else {

		tempFc = totalFreq/totalHi;
		deltaFreq = fabs(fc-tempFc)/timeDiff;
		/// Need to check deltaFreq < Threshold
		fc = tempFc;
		if(deltaFreq>delFThreshold)
			return 1;
		else return 0;
	}
}


/* ----------------------------------------------------------------------------		*/
/* FUNCTION  angleInstability():                           	     					*/
/* Computes angular deviations from center of inertia 								*/
/* ----------------------------------------------------------------------------		*/

int angleInstability(int i) {

	struct data_frame *df = TSB[index].first_data_frame
	struct cfg_frame *temp_cfg = cfgfirst;
	float temp = 0;
	float totalPi,totalTheta = 0,localAreaPi = 0,*localAreaTheta;
	int totalPMUs = 0,i = 0,noOfPMU,j,noOfAreas,noOfPMUsInOneArea = 10,tempTotalPMUs,k;

	while(temp_cfg != NULL){

			totalPMUs += to_intconvertor(temp_cfg->num_pmu);	
			temp_cfg = temp_cfg->cfgnext;
	}	

	tempTotalPMUs = totalPMUs;
	float tt;
	tt = float(totalPMUs/noOfPMUsInOneArea); // Number of areas
	noOfAreas = nearbyint(tt);
	if(fabs(tt-noOfAreas) >0) {

		noOfAreas++;
	}

	localAreaTheta = malloc((noOfAreas + 1)*sizeof(float));	

	temp_cfg = cfgfirst;

	/*  ------------------- */

	unsigned int phnmr;
	float fp_r,fp_i,fp_angle;
	unsigned int f_r,f_l;
	unsigned char *d;
	int x = 0;
	unsigned char *fp_left,*fp_right,*fx_left,*fx_right;
	fp_left = malloc(5*sizeof(unsigned char));
	fp_right = malloc(5*sizeof(unsigned char));
	fx_left = malloc(3*sizeof(unsigned char));
	fx_right = malloc(3*sizeof(unsigned char));

	while(temp_cfg != NULL){ // Main Loop starts
	
		if(df != NULL) {
			
			noOfPMUs = to_intconvertor(temp_cfg->num_pmu);	
			
			for(j=0;j<noOfPMUs;j++) {
				phnmr = temp_cfg->pmu[j]->phnmr;	
				if(phnmr != 0) {
					if(temp_cfg->pmu[j]->fmt->phasor == 1) { // Floating	

						for(i = 0;i<phnmr;i++) {		

							copy_cbyc (phasorType,(unsigned char *)cfg->pmu[j]->phunit[i],1);
							pType = to_intconvertor1(phasorType);
							if(pType == 0) { // pTYpe = 0 indicates V

								d = df->dpmu[j]->phasors[i];	
								polar = temp_cfg->pmu[j]->fmt->polar;	
								fp_angle = getPhasorAngle(1,fp_left,fp_right,d,polar,temp_cfg,i,j); // 1 -> floating pt
							}								
						}
						x++;
						if(x == generatorPMUs)
							break; 
					} else { // fixed point

						for(i = 0;i < phnmr; i++){	

							copy_cbyc (phasorType,(unsigned char *)cfg->pmu[j]->phunit[i],1);
							pType = to_intconvertor1(phasorType);
							if(pType == 0) {

		                        d = df->dpmu[j]->phasors[i];	
								polar = temp_cfg->pmu[j]->fmt->polar;	
								fp_angle = getPhasorAngle(0,fp_left,fp_right,d,polar,temp_cfg,i,j); // 0 -> fixed pt
							}	                        
						}
						x++;
						if(x == generatorPMUs)
							break; 
					}											
				}
			}	
			df = df->dnext;	
		}

		temp_cfg = temp_cfg->cfgnext;
	} // Main loop ends

	free(localAreaTheta);	
	free(fp_right);
	free(fp_left);
	free(fx_right);
	free(fx_left);
}


/* ----------------------------------------------------------------------------		*/
/* FUNCTION  phasorAggregation():                          	     					*/
/* Computes voltage phasor phasorAggregation 										*/
/* ----------------------------------------------------------------------------		*/

float voltagePhasorAggregation(int i) {

	struct data_frame *df = TSB[index].first_data_frame
	struct cfg_frame *temp_cfg = cfgfirst;

	float totalPi,totalTheta,localAreaPi,*localAreaTheta;
	totalPi = totalTheta = localAreaPi = 0;

	int i,j,noOfPMUs;
	i = j = 0;

	while(temp_cfg != NULL){

			totalPMUs += to_intconvertor(temp_cfg->num_pmu);	
			temp_cfg = temp_cfg->cfgnext;
	}

	int generatorPMUs = (totalPMUs*ratio) + 4;
	temp_cfg = cfgfirst;
	
	unsigned int phnmr,polar;
	float fp_r,fp_i,fp_mag,avgVoltagePhasor = 0;
	unsigned int f_r,f_l;
	unsigned char *d;
	int x = 0;

	unsigned char *fp_left,*fp_right,*fx_left,*fx_right;
	fp_left = malloc(5*sizeof(unsigned char));
	fp_right = malloc(5*sizeof(unsigned char));
	fx_left = malloc(3*sizeof(unsigned char));
	fx_right = malloc(3*sizeof(unsigned char));
	unsigned char phasorType;
	int pType;

	while(temp_cfg != NULL){ // Main Loop starts
	
		if(df != NULL) {
			
			noOfPMUs = to_intconvertor(temp_cfg->num_pmu);	
			
			for(j=0;j<noOfPMUs;j++) {
				phnmr = temp_cfg->pmu[j]->phnmr;	
				if(phnmr != 0) {
					if(temp_cfg->pmu[j]->fmt->phasor == 1) { // Floating	

						for(i = 0;i<phnmr;i++) {		

							copy_cbyc (phasorType,(unsigned char *)cfg->pmu[j]->phunit[i],1);
							pType = to_intconvertor1(phasorType);
							if(pType == 0) {

								d = df->dpmu[j]->phasors[i];	
								polar = temp_cfg->pmu[j]->fmt->polar;	
								getPhasorAngle(1,fp_left,fp_right,d,polar,temp_cfg,i,j); // 1 -> floating pt
							}	
							
						}
						x++;
						if(x == generatorPMUs)
							break; 
					} else { // fixed point

						for(i = 0;i < phnmr; i++){	

							copy_cbyc (phasorType,(unsigned char *)cfg->pmu[j]->phunit[i],1);
							pType = to_intconvertor1(phasorType);
							if(pType == 0) {

		                        d = df->dpmu[j]->phasors[i];	
								polar = temp_cfg->pmu[j]->fmt->polar;	
								getPhasorAngle(0,fp_left,fp_right,d,polar,temp_cfg,i,j); // 0 -> fixed pt
							}	                        
						}
						x++;
						if(x == generatorPMUs)
							break; 
					}											
				}
			}	
			df = df->dnext;	
		}
		temp_cfg = temp_cfg->cfgnext;
	} // Main loop ends

	free(localAreaTheta);	
	free(fp_right);
	free(fp_left);
	free(fx_right);
	free(fx_left);

}

/* ----------------------------------------------------------------------------		*/
/* FUNCTION  randomGenerator():                           	     					*/
/* num random numbers array generated between min to max 							*/
/* ----------------------------------------------------------------------------		*/

int * randomGenerator(int max,int min,int num) {

	int c =0,n,*coeff;
	coeff = malloc((num+1)*sizeof(int));

	while(1) {

    	n = rand()%max + min;

    	if(n>=min && n<= max) {
    		coeff[c] = n;    		
    		c++;

    		if(c == num)
    		break;
    	}   
  	}	
 	return coeff; 	  
}

/* ----------------------------------------------------------------------------		*/
/* FUNCTION  getPhasorAngle():                           	     					*/
/* get the phase angle 																*/
/* ----------------------------------------------------------------------------		*/

float getPhasorAngle(int format,char left[],char right[],unsigned char *d,int polar,struct cfg_frame *temp_cfg,i,j) {

	float fp_r,fp_i,fp_angle;
	unsigned int f_r,f_l;

	
	if(format == 1) { // format indicates fixed or floating point

		memset(left,'\0',5);
		memset(right,'\0',5);
		//d = df->dpmu[j]->phasors[i];						

		copy_cbyc (left,d,4);
		left[4] = '\0';
		d += 4;

		copy_cbyc(right,d,4);
		right[4] = '\0';   
		d += 4;

		fp_r = decode_ieee_single(left);
		fp_i = decode_ieee_single(right);

		if(polar == 1) { // POLAR
					
			fp_angle = fp_i;
		} else { // rectangular

			fp_angle = atan2f(fp_i, fp_r);
		}

	} else {


        memset(left,'\0',3);
        memset(right,'\0',3);

		copy_cbyc (left,d,2);
        left[2] = '\0';
        d += 2;

        copy_cbyc(right,d,2);
        right[2] = '\0';   
        d += 2;

        f_r = to_intconvertor(left);
        f_i = to_intconvertor(right);

        if(polar == 1) { // POLAR
              
            fp_angle = f_i*1e-4; // Angle is in 10^4 radians
        } else // RECTANGULAR 
        {
        	unsigned char s = (unsigned char *)cfg->pmu[j]->phunit[i];
        	s++;

        	unsigned char *buf = malloc(4*sizeof(unsigned char));        	
        	copy_cbyc (buf,(unsigned char *)s,3);
			buf[3] = '\0';
			unsigned long int l_phunit;
			l_phunit = to_long_int_convertor1(buf);
			l_phunit = l_phunit * 1e-5;

         	fp_r = l_phunit *f_r;
            fp_i = l_phunit *f_i;
                            
            fp_angle = atan2f(fp_i, fp_r);
            free(buf);
       	}

	}	
	return fp_angle;
}