




#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "modem.h"





/******************************************/
/*              内部函数[声明]            */
/******************************************/

extern AtCmdHandler_Status MDM_Utils_Handler                    ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status VDR_Handler_SwitchOffMS              ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status VDR_Handler_SetAudioHardware         ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status VDR_Handler_GetAudioHardware         ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status VDR_Handler_SetLoudspeakerVolume     ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );
static AtCmdHandler_Status VDR_Handler_GetLoudspeakerVolume     ( MDM_CmdTypedef *at, MDM_RspTypedef *atrsp );







/******************************************/
/*              内部变量[定义]            */
/******************************************/

extern MDM_DevTypedef *modem;

static MDM_CmdTypedef vendor_commands[] 
= {

  /* Control a Status LED */
  { USING_SYNC_PIN, "AT^SSYNC=1\r\n", 
  	100, 2, 
  	MDM_Utils_Handler, SIM_UNKNOWN, FALSE },

   /*STAT_LED GPIO Setting*/
   { STAT_LED_GPIO_SET, "AT#SLED=%s\r\n", 
	 100, 2,
	 MDM_Utils_Handler, SIM_READY, FALSE },

  /* Switch off mobile station */
  { SWITCH_OFF_MS, "AT^SMSO\r\n", 
  	400, 1,
  	VDR_Handler_SwitchOffMS, SIM_UNKNOWN, FALSE },

  /* Select audio hardware set */
  { SET_AUDIO_HARDWARE, "AT^SNFS=%s\r\n", 
  	20, 1,
  	VDR_Handler_SetAudioHardware, SIM_UNKNOWN, FALSE },

  /* Select audio hardware set */
  { GET_AUDIO_HARDWARE, "AT^SNFS?\r\n", 
  	20, 1,
  	VDR_Handler_GetAudioHardware, SIM_UNKNOWN, FALSE },

  /* Set loudspeaker volume */
  { SET_LOUDSPEAKER_VOLUME, "AT^SNFV=%s\r\n", 
  	20, 1, 
  	VDR_Handler_SetLoudspeakerVolume, SIM_UNKNOWN, FALSE },

  /* Get loudspeaker volume */
  { GET_LOUDSPEAKER_VOLUME, "AT^SNFV?\r\n", 
  	20, 1, 
  	VDR_Handler_GetLoudspeakerVolume, SIM_UNKNOWN, FALSE },

  { 0, NULL, 0, 0, NULL, SIM_UNKNOWN, FALSE }
}, *vendor_command_p = vendor_commands;



enum
{
  VDR_SMSO=1,
  VDR_SSYNC,
  VDR_SNFS,
  VDR_SNFV,
  VDR_MAX
} ;
 


static SymbolTable vdr_symbols[] 
= {
  { "SMSO",       VDR_SMSO  }, 
  { "VDR_SSYNC",  VDR_SSYNC }, 
  { "SNFS",       VDR_SNFS  }, 
  { "SNFV",       VDR_SNFV  }, 
  { NULL, 0                 }
}, *vdr_symbol_p = vdr_symbols;








/* 
 * 功能描述: 配置LED闪烁模式 
 * 引用参数: 
 *          
 * 返回值  :
 * 
 */
extern void VDR_Command_UseSyncLED ( void* ipc_data )
{
  MDM_PostAtCmdByID ( vendor_command_p, 
                      USING_SYNC_PIN, 
                      NULL );
}


extern void GL868_Command_UseSyncLED ( void* ipc_data, u8 mode, u8 on_duration, u8 off_duration )
{
    u8 params[ 15 ] = { 0 };
    sprintf ( ( char * )params, "%d,%d,%d", mode, on_duration, off_duration);
    MDM_PostAtCmdByID ( vendor_command_p, 
                      STAT_LED_GPIO_SET, 
                      params );
}


extern void GL868_Command_LED_Ctl ( void* ipc_data, u8 mode )
{
    u8 params[ 15 ] = { 0 };
    sprintf ( ( char * )params, "%d", mode);
    MDM_PostAtCmdByID ( vendor_command_p, 
                      STAT_LED_GPIO_SET, 
                      params );
}






/****************************************************
 *               S M S O toolbox           
 * AT^SMSO initiates the power-off procedure. Low level of the module's VDD pin and the URC "^SHUTDOWN" notify
 * that the procedure has completed and the module has entered the POWER DOWN mode. Therefore, be sure
 * not to disconnect the operating voltage until VDD is low or until the URC "^SHUTDOWN" is displayed. Otherwise,
 * you run the risk of losing data. For further details on how to turn off the module see the [2]. 
 * 
 * ^SHUTDOWN
 * Indicates that the power-off procedure is finished and the module will be switched off in less than 1 second. 
 ****************************************************/


/* 
 * 功能描述: 
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern void VDR_Command_SwitchOffMobileStation ( void* ipc_data )
{
  MDM_PostAtCmdByID ( vendor_command_p, 
                      SWITCH_OFF_MS, 
                      NULL );
}






/* 
 * 功能描述: 关闭MS设备
 * 引用参数: 
 *          
 * 返回值  :
 * 
 */
static AtCmdHandler_Status VDR_Handler_SwitchOffMS ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  if ( code == MDM_OK )
  {
    ascii* token = MDM_StrStr ( response, MDM_Sym2Str ( vdr_symbol_p, VDR_SMSO ) );

    if ( token )
    {
      if ( modem && modem->vendor && modem->vendor->switch_off_ms_reply )
      {
        modem->vendor->switch_off_ms_reply ( NULL );
      }
    }
    return AT_HANDLER_DONE;
  }
  else  /* Modem didn't reply or replied with an error */
  if ( ( code == MDM_TIME_OUT )||( code == MDM_ERROR ) )
  {
    if ( modem && modem->vendor && modem->vendor->switch_off_ms_error )
    {
      modem->vendor->switch_off_ms_error ( NULL );
    }
    return AT_HANDLER_DONE_ERROR;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}









/****************************************************
 *               S N F S toolbox           
 * The AT^SNFS write command serves to set the audio mode required for the connected equipment.
 * 0        Audio mode 0: Test mode with activated audio loop. It is not possible to switch
 *          to this mode using  AT^SNFS. Use  AT^SCFG="Audio/Loop" instead (see
 *          description for AT^SCFG parameter <al> for details). 
 * [1]      Audio mode 1: Standard mode optimized for the reference handset, that can
 *          be connected to the analog interface 1 (see "MC37i Hardware Interface
 *          Description" for information on this handset.) To adjust the volume use the
 *          knob of the reference handset. In audio mode 4, this handset can be used with
 *          user defined parameters.
 * Note: The default parameters are determined for type approval and are not
 * adjustable with AT commands. 
 * AT^SNFD restores <audMode> 1. 
 * 2        Audio mode 2: Customer specific mode for a basic handsfree (speakerphone)
 *          device (Siemens Car Kit Portable).
 *          Analog interface 2 is assumed as default.
 * 3        Audio mode 3: Customer specific mode for a mono-headset.
 *          Analog interface 2 is assumed as default. 
 * 4        Audio mode 4: Customer specific mode for a user handset.
 *          Analog interface 1 is assumed as default. 
 * 5        Audio mode 5: Customer specific mode. 
 *          Analog interface 1 is assumed as default. 
 * 6        Audio mode 6: Customer specific mode.
 *          Analog interface 2 is assumed as default. 
 ****************************************************/

/* 
 * 功能描述: 
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern void VDR_Command_SetAudioHardware ( void* ipc_data, u8 audMode )
{
  u8 audMsg[ 4 ] = { 0 };

  sprintf ( ( char* )audMsg, "%u", audMode );
  MDM_PostAtCmdByID ( vendor_command_p, 
                      SET_AUDIO_HARDWARE, 
                      audMsg );
}



/* 
 * 功能描述: 
 * 引用参数: 
 *          
 * 返回值  :
 * 
 */
static AtCmdHandler_Status VDR_Handler_SetAudioHardware ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  if ( code == MDM_OK )
  {
    ascii* token = MDM_StrStr ( response, MDM_Sym2Str ( vdr_symbol_p, VDR_SNFS ) );

    if ( token )
    {
      if ( modem && modem->vendor && modem->vendor->set_audio_mode_reply )
      {
        modem->vendor->set_audio_mode_reply ( NULL );
      }
    }
    return AT_HANDLER_DONE;
  }
  else  /* Modem didn't reply or replied with an error */
  if ( ( code == MDM_TIME_OUT )||( code == MDM_ERROR ) )
  {
    if ( modem && modem->vendor && modem->vendor->set_audio_mode_error )
    {
      modem->vendor->set_audio_mode_error ( NULL );
    }
    return AT_HANDLER_DONE_ERROR;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}





/* 
 * 功能描述: 
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern void VDR_Command_GetAudioHardware ( void* ipc_data )
{
  MDM_PostAtCmdByID ( vendor_command_p, 
                      GET_AUDIO_HARDWARE, 
                      NULL );
}



/* 
 * 功能描述: 
 * 引用参数: 
 *          
 * 返回值  :
 * 
 */
static AtCmdHandler_Status VDR_Handler_GetAudioHardware ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  if ( code == MDM_OK )
  {
    ascii* token = MDM_StrStr ( response, MDM_Sym2Str ( vdr_symbol_p, VDR_SNFS ) );

    if ( token )
    {
      token = ( ascii* )strstr ( ( const char* )token, ": " );
      if ( token )
      {
        u8 audMode = ( u8 )atoi ( ( const char* )( token + 2 ) );
        if ( modem && modem->vendor && modem->vendor->get_audio_mode_reply )
        {
          modem->vendor->get_audio_mode_reply ( NULL, audMode );
        }
        return AT_HANDLER_DONE;
      }
    }
    return AT_HANDLER_RETRY;
  }
  else  /* Modem didn't reply or replied with an error */
  if ( ( code == MDM_TIME_OUT )||( code == MDM_ERROR ) )
  {
    if ( modem && modem->vendor && modem->vendor->get_audio_mode_error )
    {
      modem->vendor->get_audio_mode_error ( NULL );
    }
    return AT_HANDLER_DONE_ERROR;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}







/****************************************************
 *               S N F V toolbox           
 * AT^SNFV canbeusedtosetthevolumeoftheloudspeakertothevalue <outCalibrate> addressed by
 * <outStep>. The read and write commands refer to the active audio mode. The write command works only in
 * audio modes 2 to 6.  
 * Any change to <outStep> takes effect in audio modes 2 to 6. That is, when you change <outStep> and
 * then select another mode with AT^SNFS, the same step will be applied. Nevertheless, the actual volume can
 * be quite different, depending on the values of <outCalibrate> set in each mode. The only exception is
 * audio mode 1 which is fixed to <outStep>=4. 
 * <outStep> is stored non-volatile when the ME is powered down with AT^SMSO or reset with AT+CFUN=1,1.
 * <outStep> is not stored by AT^SNFW. 
 * <outStep> can also be changed by AT^SNFO (Section 16.13) and AT+CLVL (Section 16.4). 
 ****************************************************/

/* 
 * 功能描述: 
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern void VDR_Command_SetLoudspeakerVolume ( void* ipc_data, u8 outStep )
{
  u8 outstepMsg[ 4 ] = { 0 };

  sprintf ( ( char* )outstepMsg, "%u", outStep );
  MDM_PostAtCmdByID ( vendor_command_p, 
                      SET_LOUDSPEAKER_VOLUME, 
                      outstepMsg );
}





/* 
 * 功能描述: 
 * 引用参数: 
 *          
 * 返回值  :
 * 
 */
static AtCmdHandler_Status VDR_Handler_SetLoudspeakerVolume ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  if ( code == MDM_OK )
  {
    ascii* token = MDM_StrStr ( response, MDM_Sym2Str ( vdr_symbol_p, VDR_SNFV ) );

    if ( token )
    {
      if ( modem && modem->vendor && modem->vendor->set_loudspeaker_volume_reply )
      {
        modem->vendor->set_loudspeaker_volume_reply ( NULL );
      }
    }
    return AT_HANDLER_DONE;
  }
  else  /* Modem didn't reply or replied with an error */
  if ( ( code == MDM_TIME_OUT )||( code == MDM_ERROR ) )
  {
    if ( modem && modem->vendor && modem->vendor->set_loudspeaker_volume_error )
    {
      modem->vendor->set_loudspeaker_volume_error ( NULL );
    }
    return AT_HANDLER_DONE_ERROR;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}




/* 
 * 功能描述: 
 * 引用参数:
 *          
 * 返回值  :
 * 
 */
extern void VDR_Command_GetLoudspeakerVolume ( void* ipc_data )
{
  MDM_PostAtCmdByID ( vendor_command_p, 
                      GET_LOUDSPEAKER_VOLUME, 
                      NULL );
}




/* 
 * 功能描述: 
 * 引用参数: 
 *          
 * 返回值  :
 * 
 */
static AtCmdHandler_Status VDR_Handler_GetLoudspeakerVolume ( MDM_CmdTypedef *at, MDM_RspTypedef *response )
{
  MDM_RSP_ID code = MDM_GetResultCode ( response );

  if ( code == MDM_OK )
  {
    ascii* token = MDM_StrStr ( response, MDM_Sym2Str ( vdr_symbol_p, VDR_SNFV ) );

    if ( token )
    {
      token = ( ascii* )strstr ( ( const char* )token, ": " );
      if ( token )
      {
        u8 volume = ( u8 )atoi ( ( const char* )( token + 2 ) );
        if ( modem && modem->vendor && modem->vendor->get_loudspeaker_volume_reply )
        {
          modem->vendor->get_loudspeaker_volume_reply ( NULL, volume );
        }
        return AT_HANDLER_DONE;
      }
    }
    return AT_HANDLER_RETRY;
  }
  else  /* Modem didn't reply or replied with an error */
  if ( ( code == MDM_TIME_OUT )||( code == MDM_ERROR ) )
  {
    if ( modem && modem->vendor && modem->vendor->get_loudspeaker_volume_error )
    {
      modem->vendor->get_loudspeaker_volume_error ( NULL );
    }
    return AT_HANDLER_DONE_ERROR;
  }

  return AT_HANDLER_DONT_UNDERSTAND;
}




