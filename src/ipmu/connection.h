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


#define BACKLOG 10            /* How many pending connections queue will hold */

/* ---------------------------------------------------------------- */
/*                         global variables                         */
/* ---------------------------------------------------------------- */

int udp_send_status;	// 0-ON, 1-OFF
int tcp_send_status;	// 0-ON, 1-OFF
 
struct sockaddr_in UDP_my_addr,TCP_my_addr; /* my address information */
struct sockaddr_in UDP_addr,TCP_addr; /* connector’s address information */
struct sigaction sa;

int UDP_sockfd,TCP_sockfd; /* socket descriptors */
int UDP_addr_len,TCP_sin_size, numbytes;

pthread_t UDP_thread,TCP_thread,DATA_thread;

pthread_mutex_t mutex_data;  /* To manage data ON/OFF */

/* ------------------------------------------------------------------ */
/*                       Global Datastructure                         */
/* ------------------------------------------------------------------ */

struct status 
{
	int udp_send_status; 
	int tcp_send_status; 
	int tcp_new_fd; 

}*status_info;

/* ------------------------------------------------------------------ */
/*                       Function prototypes                          */
/* ------------------------------------------------------------------ */

void server(int id, int uport, int tport);

void* SEND_DATA();

void* UDP_PMU();

void* TCP_PMU();

/**************************************** End of File *******************************************************/
