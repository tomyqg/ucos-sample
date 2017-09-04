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
 *
 * \brief NMEA parser implementation.
 *
 * NMEA 0183 is acronym of National Marine Electronics Association that
 * combined electrical and data specification for communication between marine
 * electronic devices such as echo sounder, sonars, anemometer (wind speed and direction),
 * gyrocompass, autopilot, GPS receivers and many other types of instruments.
 * It has been defined by, and is controlled by, the U.S.-based National Marine
 * Electronics Association.
 * The NMEA 0183 standard uses a simple ASCII, serial communications protocol
 * that defines how data is transmitted in a "sentence" from one "talker"
 * to multiple "listeners" at a time.
 * At the application layer, the standard also defines the contents of each sentence
 * (message) type so that all listeners can parse messages accurately.
 *
 */






#include <ctype.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include "nmea.h"

#include "tiza_include.h"
/**
 * \name Integer round macros.
 *
 * Round \a x to a multiple of \a base.
 * \note If \a x is signed these macros generate a lot of code.
 * \{
 */
#define ROUND_DOWN(x, base)    ( (x) - ((x) % (base)) )
#define ROUND_UP(x, base)      ( ((x) + (base) - 1) - (((x) + (base) - 1) % (base)) )
#define ROUND_NEAREST(x, base) ( ((x) + (base) / 2) - (((x) + (base) / 2) % (base)) )
/* \} */





/******************************************/
/*              内部函数[声明]            */
/******************************************/

static u32 tokenToInt ( const char *s, int precision );
static s32 nmea_altitude ( const char *palt, const char *punits );
static udegree_t nmea_latitude ( const char *plat, const char *phem );
static udegree_t nmea_longitude ( const char *plot, const char *phem );
static udegree_t convertToDegree ( const char *str );
static time_t timestampToSec ( u32 time_stamp, u32 date_stamp );









/*
 * Make conversion from one string to int.
 *
 * You can specify the precision if the string is a float
 * number. The result is an int multiplied to 10^precision.
 */
static u32 tokenToInt ( const char *s, int precision )
{
	u32 num = 0;
	bool sep_found = FALSE;
	int i;

	if ( !s )
	{
    return 0;
  }

	for ( i = 0; i < NMEAP_MAX_SENTENCE_LENGTH; i++ )
	{
		char c = *s++;

		if ( c == '.' )
		{
			sep_found = TRUE;
			continue;
		}

		if ( c == '\0' || !isdigit( c ) || ( precision == 0 && sep_found ) )
			break;

		if ( sep_found )
			precision--;

		num *= 10;
		num += c - '0';
	}

	while ( precision-- )
		num *= 10;

	return num;
}



/*
 * Convert a string to micro degree.
 */
static udegree_t convertToDegree ( const char *str )
{
	u32 dec;
	u32 deg;
	u32 min;

	if ( *str == 0 )
  {
    return 0;
  }		

	dec = tokenToInt ( str, 4 );
	deg = dec / 1000000;
	min = dec - deg * 1000000;
	dec = deg * 1000000 + ((min * 5) + 1) / 3;

	return dec;
}






/*
 * Retun latitude in micro degree from a string.
 */
static udegree_t nmea_latitude ( const char *plat, const char *phem )
{
	int ns;

	if ( *phem == 0 )
  {
    return 0;
  }		

	/* north lat is +, south lat is - */
	ns = ( *phem == 'N' ) ? 1 : -1;


	return ns * convertToDegree ( plat );
}






/*
 * Retun longitude in micro degree from a string.
 */
static udegree_t nmea_longitude ( const char *plot, const char *phem )
{
	int ew;

	if (*phem == 0)
		return 0;

	/* west long is negative, east long is positive */
	ew = ( *phem == 'E' ) ? 1 : -1;

	return ew * convertToDegree ( plot );
}







/*
 * Return altitude in meter from a string.
 *
 */
static s32 nmea_altitude ( const char *palt, const char *punits )
{
	s32 alt;

	if (*palt == 0)
		return 0;

	alt = atoi ( palt );

	if ( *punits == 'F' )
	{
		/* convert to feet */
		/* alt = alt * 3.2808399 */
		alt = alt * 3 +  /* 3.0 */
			(alt >> 2) + /* 0.25 */
			(alt >> 6) + /* 0.015625 */
			(alt >> 7) + /* 0.0078125 */
			(alt >> 8); /* 0,00390625 */
	}

	return alt;
}








/*
 * Convert time and date stamp string to unix time.
 */
static time_t timestampToSec ( u32 time_stamp, u32 date_stamp )
{
	struct tm t;
  u16 i;
	u16 msec;
	u16 tmr[ 3 ];
	u16 date[ 3 ];
  u32 res;
  u32 all;

	memset ( &t, 0, sizeof( t ) );
	memset ( &tmr, 0, sizeof( tmr ) );
	memset ( &date, 0, sizeof( date ) );

	res = time_stamp / 1000;
	all = time_stamp;
	msec = all - res * 1000;

	for ( i = 0; i < 3; i++ )
	{
		all = res;
		res = all / 100;
		tmr[ i ]  = all - res * 100;
	}

	t.tm_sec = tmr[ 0 ] + ( ROUND_UP( msec, 1000 ) / 1000 );
	t.tm_min = tmr[ 1 ];
	t.tm_hour = tmr[ 2 ];

	/* If we do not have refence data, we set 1/1/1970 as default */
	t.tm_mday = 1;
	t.tm_mon = 0;
	t.tm_year = 70;

	if ( date_stamp )
	{
		res = all = date_stamp;
		for ( i = 0; i < 3; i++ )
		{
			all = res;
			res = all / 100;
			date[ i ]  = all - res * 100;
		}

		t.tm_mday = date[2];
		t.tm_mon = date[1] - 1; /* time struct count month from 0 to 11; */ 
		/* we should specify the number of years from 1900, but the year field
     * is only two digits, so we add 100 (2000 - 1900)..
     */
		t.tm_year = date[ 0 ] + 100;
	}

	return  mktime( &t );
}







/**
 *  Callout example for GGA data
 */
void gpgga_callout(nmeap_context_t *context, void *data, void *user_data)
{
	(void)context;
	(void)user_data;
	(void)data;

//	NmeaGga *gga = (NmeaGga *)data;
}






/**
 * Callout example for RMC
 */
void gprmc_callout(nmeap_context_t *context, void *data, void *user_data)
{
	(void)context;
	(void)user_data;
	(void)data;


//		NmeaRmc *rmc = (NmeaRmc *)data;
}






/**
 * Callout example for GSV data
 */
void gpgsv_callout(nmeap_context_t *context, void *data, void *user_data)
{
	(void)context;
	(void)user_data;
	(void)data;

//		NmeaGsv *gsv = (NmeaGsv *)data;
}






/**
 * Callout example for VTG data
 */
void gpvtg_callout(nmeap_context_t *context, void *data, void *user_data)
{
	(void)context;
	(void)user_data;
	(void)data;

//		NmeaVtg *vtg = (NmeaVtg *)data;
}



/**
 * standard GPGGA sentence parser
 */
extern int nmea_gpgga ( nmeap_context_t *context, nmeap_sentence_t *sentence )
{
	/*
	 * get pointer to sentence data
	 */
	NmeaGga *gga = (NmeaGga *)sentence->data;	
	
	//printf("\r\n****************nmea_gpgga*********************\r\n");
	gga->latitude   = nmea_latitude  ( context->token[ 2 ], context->token[ 3 ] );
	gga->longitude  = nmea_longitude ( context->token[ 4 ], context->token[ 5 ] );
	gga->altitude   = nmea_altitude  ( context->token[ 9 ], context->token[ 10] );
	gga->time       = timestampToSec ( tokenToInt ( context->token[ 1 ], 3 ), 0 );
	gga->satellites = atoi ( context->token[ 7 ] );
	gga->quality    = atoi ( context->token[ 6 ] );
	gga->hdop       = tokenToInt ( context->token[8], 1 );
	gga->geoid      = nmea_altitude ( context->token[ 11 ], context->token[ 12 ] );

	/*
	 * if the sentence has a callout, call it
	 */
	if ( sentence->callout != 0 )
	{
    ( *sentence->callout )( context, gga, context->user_data );
  }

	return NMEA_GPGGA;
}






/**
 * standard GPRMC sentence parser
 */
extern int nmea_gprmc ( nmeap_context_t *context, nmeap_sentence_t *sentence )
{
	/*
	 * get pointer to sentence data
	 */
	NmeaRmc *rmc = (NmeaRmc *)sentence->data;
	/*
	 * extract data from the tokens
	 */
	rmc->time       = timestampToSec ( tokenToInt ( context->token[ 1 ], 3 ), tokenToInt ( context->token[ 9 ], 0 ) );
	rmc->warn       = *context->token[ 2 ];
	rmc->latitude   = nmea_latitude ( context->token[ 3 ], context->token[ 4 ] );
  rmc->longitude  = nmea_longitude( context->token[ 5 ], context->token[ 6 ] );

  {
    u32 i = 0;
    u8* data = ( u8* )context->token[ 3 ];

    rmc->latitude0 = 0;
    for ( i = 0; i < 9;  )
    {
      if ( data[ i ] != '.' )
      {
        rmc->latitude0 = rmc->latitude0 * 10 + ( data[ i ] - '0' );
      }
      i++;
    }
  }

  {
    u32 i = 0;
    u8* data = ( u8* )context->token[ 5 ];

    rmc->longitude0 = 0;
    for ( i = 0; i < 10; )
    {
      if ( data[ i ] != '.' )
      {
        rmc->longitude0 = rmc->longitude0 * 10 + ( data[ i ] - '0' );
      }
      i++;
    }
  }

//rmc->fLat       = atof ( context->token[ 3 ] );
//rmc->fLon       = atof ( context->token[ 5 ] );
	rmc->speed      = atoi ( context->token[ 7 ] );
	rmc->course     = atoi ( context->token[ 8 ] );
	rmc->mag_var    = atoi ( context->token[ 10] );

	/*
	 * if the sentence has a callout, call it
	 */
	if ( sentence->callout != 0 )
  {
    ( *sentence->callout )( context, rmc, context->user_data );
  }		

	return NMEA_GPRMC;
}






/**
 * standard GPVTG sentence parser
 */
extern int nmea_gpvtg ( nmeap_context_t *context, nmeap_sentence_t *sentence )
{
	/*
	 * get pointer to sentence data
	 */
	NmeaVtg *vtg = (NmeaVtg *)sentence->data;

	/*
	 * extract data from the tokens
	 */
	vtg->track_good  = atoi ( context->token[ 1 ] );
	vtg->knot_speed  = atoi ( context->token[ 5 ] );
	vtg->km_speed    = atoi ( context->token[ 7 ] );

	/*
	 * if the sentence has a callout, call it
	 */
	if ( sentence->callout != 0 )
	{
    ( *sentence->callout )( context, vtg, context->user_data );
  }
	
	return NMEA_GPVTG;
}






/**
 * standard GPGSV sentence parser
 */
extern int nmea_gpgsv ( nmeap_context_t *context, nmeap_sentence_t *sentence )
{
	/*
	 * get pointer to sentence data
	 */
	NmeaGsv *gsv = (NmeaGsv *)sentence->data;
	/*
	 * extract data from the tokens
	 */
	gsv->tot_message     = atoi ( context->token[ 1 ] );
	gsv->message_num     = atoi ( context->token[ 2 ] );
	gsv->tot_svv         = atoi( context->token[ 3 ] );


	/* Fill remaning member until we have token */ 
  {
  	int  i, j;


  	for ( i = 4, j = 0; i < context->tokens - 3; i += 4, j++ )
  	{  
  		gsv->info[ j ].sv_prn     = atoi ( context->token[ i + 0 ] );
  		gsv->info[ j ].elevation  = atoi ( context->token[ i + 1 ] );
  		gsv->info[ j ].azimut     = atoi ( context->token[ i + 2 ] );
  		gsv->info[ j ].snr        = atoi ( context->token[ i + 3 ] );
  	}  
  }

	/*
	 * if the sentence has a callout, call it
	 */

	if ( sentence->callout != 0 )
  {
    ( *sentence->callout )( context, gsv, context->user_data );
  }

	return NMEA_GPGSV;

}




/**
 * Parse NMEA sentence from a channel.
 */
extern void nmea_poll ( nmeap_context_t *context, ascii *channel, s32 length )
{
  if ( context && channel && length > 0 )
  {
		
    nmeap_parseBuffer ( context, ( const char* )channel, ( int* )&length );
  }
}
