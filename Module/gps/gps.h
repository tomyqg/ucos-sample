





#ifndef __GPS_H__
#define __GPS_H__



#include "nmea.h"




/******************************************/
/*               GPS参数[配置]            */
/******************************************/

#define GPS_DEBUG   0

typedef struct {

#ifndef __USE_GPGGA__
#define __USE_GPGGA__     0

#if __USE_GPGGA__ == 1 
    /* do something with the GGA data */
  nmeap_callout_t gpgga;
#endif

#endif  /* __USE_GPGGA__ */


#ifndef __USE_GPRMC__
#define __USE_GPRMC__     1

#if __USE_GPRMC__ == 1 
  /* do something with the RMC data */
  nmeap_callout_t gprmc;
#endif

#endif  /* __USE_GPRMC__ */


#ifndef __USE_GPGSV__
#define __USE_GPGSV__     0

#if __USE_GPGSV__ == 1 
  /* do something with the GSV data */
  nmeap_callout_t gpgsv;
#endif

#endif  /* __USE_GPGSV__ */


#ifndef __USE_GPVTG__
#define __USE_GPVTG__     0

#if __USE_GPVTG__ == 1 
  /* do something with the VTG data */
  nmeap_callout_t gpvtg;
#endif

#endif  /* __USE_GPVTG__ */

} GPS_DevTypedef;






/******************************************/
/*              外部函数[声明]            */
/******************************************/

extern void GPS_IfInit ( GPS_DevTypedef *gps );
extern void GPS_Polling ( ascii *channel, s32 length );
extern void GPS_ErrorPrint ( void );


#endif  /* __GPS_H__ */



