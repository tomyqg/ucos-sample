



#include "stdio.h"
#include "gps.h"
#include "tiza_gps_port.h"


/******************************************/
/*               内部宏[定义]             */
/******************************************/

#define GPS_STATUS_IS_UNINIT        0x00
#define GPS_STATUS_IS_INIT_DONE     0x01





/******************************************/
/*              内部变量[定义]            */
/******************************************/

static u8 gpsIsValid = GPS_STATUS_IS_UNINIT;
static nmeap_context_t nmea;	   /* parser context */

#if __USE_GPRMC__ == 1
static NmeaRmc rmc;
#endif  /* __USE_GPRMC__ */

#if __USE_GPGGA__ == 1  
static NmeaGga gga;
#endif  /* __USE_GPGGA__ */

#if __USE_GPGSV__ == 1 
static NmeaGsv gsv;
#endif  /* __USE_GPGSV__ */

#if __USE_GPVTG__ == 1 
static NmeaVtg vtg;
#endif  /* __USE_GPVTG__ */











/* 
 * 功能描述：
 * 引用参数：无
 *          
 * 返回值  ：无
 * ps:兼容北斗系统
 */
 
 
extern void GPS_IfInit ( GPS_DevTypedef *gps )
{
	if ( gps )
  {
    nmeap_init ( ( nmeap_context_t* )&nmea, NULL );
    gpsIsValid = GPS_STATUS_IS_INIT_DONE;
		
#if __USE_GPGGA__ == 1  
    if ( gps->gpgga )
  	{
      nmeap_addParser ( ( nmeap_context_t* )&nmea, "GPGGA", nmea_gpgga, gps->gpgga, &gga );
    }
#endif  /* __USE_GPGGA__ */

#if __USE_GPRMC__ == 1  
    if ( gps->gprmc )
    {	
			nmeap_addParser ( ( nmeap_context_t* )&nmea, "GNRMC", nmea_gprmc, gps->gprmc, &rmc );
    }
#endif  /* __USE_GPRMC__ */

#if __USE_GPGSV__ == 1   	
    if ( gps->gpgsv )
    {		
      nmeap_addParser ( ( nmeap_context_t* )&nmea, "GPGSV", nmea_gpgsv, gps->gpgsv, &gsv );
    }
#endif  /* __USE_GPGSV__ */

#if __USE_GPVTG__ == 1 
    if ( gps->gpvtg )
    {
      nmeap_addParser ( ( nmeap_context_t* )&nmea, "GPVTG", nmea_gpvtg, gps->gpvtg, &vtg );	
    }
#endif  /* __USE_GPVTG__ */
  
  }
}









/* 
 * 功能描述：
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
extern void GPS_Polling ( ascii *channel, s32 length )
{
  if ( gpsIsValid == GPS_STATUS_IS_INIT_DONE && channel && length > 0 )
  {
		
    nmea_poll ( ( nmeap_context_t* )&nmea, channel, length );
  }
}







/* 
 * 功能描述：
 * 引用参数：无
 *          
 * 返回值  ：无
 * 
 */
extern void GPS_ErrorPrint ( void )
{
  printf ( "\r\ngps msgs %6u err_hdr %6u err_ovr %6u err_unk %6u err_id %6u err_cks %6u err_crl %6u\r\n",
           nmea.msgs, 
           nmea.err_hdr, 
           nmea.err_ovr, 
           nmea.err_unk, 
           nmea.err_id, 
           nmea.err_cks, 
           nmea.err_crl );
}






