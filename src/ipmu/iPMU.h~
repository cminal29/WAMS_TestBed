/* ----------------------------------------------------------------------------- 
 * ServerFunction.h
 * 
 * PMU Simulator - Phasor Measurement Unit Simulator
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


#define MAX_STRING_SIZE 5000
#define TB 1000000 //6777216 // roundf(powf(2,24) - 1);


/* ---------------------------------------------------------------- */
/*                         global variables                         */
/* ---------------------------------------------------------------- */

unsigned char cfg2_frm[MAX_STRING_SIZE];
unsigned char data_frm[MAX_STRING_SIZE];

int data_mode; // 0: auto, 1: csv
long int soc, fracsec, fsecNum;
int data_frm_size, cfg2_frm_size;

uint16_t chk;	// For checksum calculation
FILE *fp_csv;
struct timeval tim;

FILE *logFile;  ///KK
char logfilename[10]; //KK

/* ------------------------------------------------------------------ */
/*                       Global Datastructure                         */
/* ------------------------------------------------------------------ */

struct ConfigurationFrame 
{
	int  cfg_pmuID; 
	int  cfg_fdf; 
	int  cfg_af; 
	int  cfg_pf; 
	int  cfg_pn; 
	int  cfg_phnmr_val; 
	int  cfg_annmr_val; 
	int  cfg_dgnmr_val; 
	int  cfg_fnom; 
	int  cfg_dataRate; 
	char *cfg_STNname; 
	char *cfg_phasor_channels; 
	char *cfg_analog_channels; 
	char *cfg_digital_channels; 
	unsigned char *cfg_phasor_phunit; 
	unsigned char *cfg_analog_anunit; 
	
}*cfg_info;

/* ------------------------------------------------------------------ */
/*                       Function prototypes                          */
/* ------------------------------------------------------------------ */

void auto_create_cfg(int pmuID, int Frequency);

int csv_create_cfg(int pmuID, char *filePath);

void generate_data_frame();

/**************************************** End of File *******************************************************/
