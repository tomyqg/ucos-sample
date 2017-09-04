/**
 * \file
 * <!--
 * This file is part of BeRTOS.
 *
 * Bertos is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 *
 * Copyright 2009 Develer S.r.l. (http://www.develer.com/)
 *
 * -->
 * \brief NMEA Parser.
 *
 * \author Daniele Basile <asterix@develer.com>
 *
 * $WIZ$ module_name = "nmea"
 */





#ifndef __NMEA_H__
#define __NMEA_H__


#include "stm32f4xx.h"



#include <time.h>
#include <nmeap.h>



/*
 * Implemented NMEA parser strings.
 */
#define NMEA_GPGGA 1   /* GGA MESSAGE ID */ 
#define NMEA_GPRMC 2   /* RMC MESSAGE ID */ 
#define NMEA_GPVTG 3   /* VTG MESSAGE ID */ 
#define NMEA_GPGSV 4   /* GSV MESSAGE ID */ 




/* Standart type to rappresent fiels. */ 
typedef s32 udegree_t;    /* Micro degrees */ 
typedef s32 mdegree_t;    /* Milli degrees */ 
typedef s16 degree_t;     /* Degrees */ 




/**
 * Global Positioning System Fix Data.
 * Extracted data from a GGA message
 *
 * Note: time member contains the seconds elapsed from 00:00:00 1/1/1970,
 * because from nmea sentence we read only the time of UTC position, we
 * have not any reference of date (day, month and year) so time is referred to
 * the start of unix time.
 */
typedef struct 
{
	udegree_t     latitude;   /* Latitude (micro degree) */
	udegree_t     longitude;  /* Longitude (micro degree) */
	s32           altitude;   /* Altitude (Meter) */
	time_t        time;       /* UTC of position  (Unix time) */
	u16           satellites; /* Satellites are in view */
	u16           quality;    /* Fix Quality: 0 = Invalid; 1 = GPS fix; 2 = DGPS fix; */
	u16           hdop;       /* Relative accuracy of horizontal position (hdop * 10) */
	s16           geoid;      /* Height of geoid above WGS84 ellipsoid (Meter) */
} NmeaGga;



/**
 * Recommended minimum specific GPS/Transit data.
 * Extracted data from an RMC message
 *
 * Note: RMC sentences contain also date stamp so, time contains real seconds elapsed
 * from 0:00:00 1/1/1970.
 */
typedef struct 
{
	time_t        time;       /* UTC of position  (Unix time) */
	char          warn;       /* Navigation receiver warning A = OK, V = warning */
	udegree_t     latitude;   /* Latitude (micro degree) */
	udegree_t     longitude;  /* Longitude (micro degree) */
	udegree_t     latitude0;   /* Latitude */
	udegree_t     longitude0;  /* Longitude */

  float         fLat;
  float         fLon;
	//u16           speed;      /* Speed over ground (knots) */
	double				speed;      /* Speed over ground (knots) */
	degree_t      course;     /* Track made good in degrees True (degree) */
	degree_t      mag_var;    /* Magnetic variation degrees (degree) */
} NmeaRmc;



/**
 * Extracted data from an vtg message
 */
typedef struct 
{
	degree_t     track_good;  /* True track made good (degree) */
	u16          knot_speed;  /* Speed over ground (knots) */
	u16          km_speed;    /* Speed over ground in kilometers/hour */
} NmeaVtg;



/**
 * Extracted data from an gsv message
 */
struct SvInfo 
{
	u16         sv_prn;       /* SV PRN number */
	degree_t    elevation;    /* Elevation in degrees, 90 maximum */
	degree_t    azimut;       /* Azimuth, degrees from true north, 000 to 359 */
	u16         snr;          /* SNR, 00-99 dB (null when not tracking) */
} ;



typedef struct 
{
	u16    tot_message;  /* Total number of messages of this type in this cycle */
	u16    message_num;  /* Message number */
	u16    tot_svv;      /* Total number of SVs in view */
	struct SvInfo info[ 4 ];    /* Stanrd gsv nmea report up to 4 sv info */
} NmeaGsv;







/******************************************/
/*              外部函数[声明]            */
/******************************************/

extern void nmea_poll ( nmeap_context_t *context, ascii *channel, s32 length );
extern int nmea_gpgsv ( nmeap_context_t *context, nmeap_sentence_t *sentence );
extern int nmea_gpvtg ( nmeap_context_t *context, nmeap_sentence_t *sentence );
extern int nmea_gprmc ( nmeap_context_t *context, nmeap_sentence_t *sentence );
extern int nmea_gpgga ( nmeap_context_t *context, nmeap_sentence_t *sentence );



#endif /* __NMEA_H__ */




