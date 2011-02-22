/*************************************************************************************
 * File   : idct.c, file for JPEG-JFIF Multi-thread decoder    
 *
 * Copyright (C) 2007 TIMA Laboratory
 * Author(s) :      Alexandre Chagoya-Garzon 
 * Bug Fixer(s) :   
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 *************************************************************************************/
#include <stdio.h>
#include <string.h> 
#include <malloc.h>

#include <dpn.h>

#include "compute.h"
#include "mjpeg.h"
#include "utils.h"

uint8_t * c_Idct_YCbCr;
int32_t * c_block_YCbCr;
uint32_t c_flit_size = 0;

Channel ** c_idct;

int idct_init (Channel ** _c) {  
  printf("%s\n", __FUNCTION__);
  c_idct = _c;

	channelRead (c_idct[0], (uint8_t *) & c_flit_size, sizeof (uint32_t));
	VPRINTF("Flit size = %lu\r\n", c_flit_size);

	c_Idct_YCbCr = (uint8_t *) malloc (c_flit_size * 64 * sizeof (uint8_t));
	if (c_Idct_YCbCr == NULL) printf ("%s,%d: malloc failed\n", __FILE__, __LINE__);

	c_block_YCbCr = (int32_t *) malloc (c_flit_size * 64 * sizeof (int32_t));
	if (c_block_YCbCr == NULL) printf ("%s,%d: malloc failed\n", __FILE__, __LINE__);
  return 0;
  
}

int idct_process () {
  printf("%s\n", __FUNCTION__);
		channelRead (c_idct[0], (unsigned char *) c_block_YCbCr, c_flit_size * 64 * sizeof (int32_t));

		for (uint32_t i = 0; i < c_flit_size; i++) IDCT(& c_block_YCbCr[i * 64], & c_Idct_YCbCr[i * 64]);

		channelWrite (c_idct[1], (unsigned char *) c_Idct_YCbCr, c_flit_size * 64 * sizeof (uint8_t));

	return 0;
}
