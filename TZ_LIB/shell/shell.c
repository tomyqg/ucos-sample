



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shell.h"

enum{
		
	FALSE = 0,
	TRUE = !FALSE
};




/* ����������ʱ�����С */
#define SHELL_TMP_STRING_BUFFER_SIZE    128    /* ��λ��bytes */

/* �������󳤶� */
#define SHELL_CMD_STRING_MAX_LENGTH     16    /* ��λ��bytes */

/* ֧�ֵĴ���ָ������ */
#define SHELL_CMD_MAX_NUMBER            22


typedef struct
{
  /* ָ��� */
  ascii cmdStr[ SHELL_CMD_STRING_MAX_LENGTH ];

  /* ����ѡ�� */
  u16 cmdOpt;

  /* ָ����� */
  shell_atCmdHandler_t cmdHdlr;
} SHELL_CmdTypedef;



/* ѡ�����ݼ�� */
#define SHELL_ARG_SEPARATOR_LENGTH		 2    /* ��λ��bytes */

/* ����ѡ����󳤶� */
#define SHELL_ARG_STRING_MAX_LENGTH    128     /* ��λ��bytes */

/* ͬʱ�����ѡ������ */
#define SHELL_ARG_MAX_NUMBER					 10    


typedef struct 
{
	/* ����ѡ��� */
	u8 argsLength;
	
	/* ����ѡ����ַ */
	ascii *argsStartAddress;
} SHELL_ArgsTypedef;





/******************************************/
/*              �ڲ�����[����]            */
/******************************************/

/* ָ����� */
static u8  shellCmdCounter = 0;

/* ״̬ */
static u32 shellCmdIsVliad = 0;   

static struct 
{
  /* ���� */
  SHELL_CmdTypedef cmd;
  
  /* ������ */
  SHELL_CmdParserTypedef parser;  
} shellCmdArray[ SHELL_CMD_MAX_NUMBER ];


static u8 shellDataBuffer[ SHELL_TMP_STRING_BUFFER_SIZE ];

/* ����ѡ��� */
static ascii shellArgsBuffer[ SHELL_ARG_STRING_MAX_LENGTH ];

/* ����ѡ����� */
static u8 shellArgsCounter = 0;

/* ����ѡ�� */
static SHELL_ArgsTypedef shellArgsArray[ SHELL_ARG_MAX_NUMBER ];





/******************************************/
/*              �ڲ�����[����]            */
/******************************************/

static void SHELL_ArgsOptionReset ( void );
static bool SHELL_ArgsOptionParser ( ascii *response );









/* Works like strncmp, but not case sensitive */
int strnicmp ( const char *cs, const char *ct, int count )  
{
	int c1, c2, res = 0;

	while ( count > 0 ) 
  {
		c1 = *cs++; c2 = *ct++;
		if (c1 >= 'A' && c1 <= 'Z') c1 += 'a' - 'A';
		if (c2 >= 'A' && c2 <= 'Z') c2 += 'a' - 'A';
		if ((res = c1 - c2) != 0 || (!*cs && !*ct))
			break;
		count--;
	}

	return res;
}






/* 
 * ��������������ע��ӿ�
 * ���ò�����
 *          
 * ����ֵ  ��
 * 
 */
extern bool SHELL_CmdSubscribe ( ascii *Cmdstr, shell_atCmdHandler_t Cmdhdlr, u16 Cmdopt )
{
  u8 i = 0;  
  
  if ( shellCmdCounter < SHELL_CMD_MAX_NUMBER && Cmdstr && Cmdhdlr && Cmdopt )
  {
    if ( strlen ( ( const char * )Cmdstr ) < SHELL_CMD_STRING_MAX_LENGTH )
    {
      for ( i = 0; i < SHELL_CMD_MAX_NUMBER; i++ )
      {
         
        if ( ( shellCmdIsVliad & ( ( u32 )0x01 << i ) ) == 0 )
        {
          shellCmdArray[ i ].cmd.cmdOpt = Cmdopt;
          shellCmdArray[ i ].cmd.cmdHdlr = Cmdhdlr;
          strcpy ( ( char * )shellCmdArray[ i ].cmd.cmdStr, ( const char* )Cmdstr );
  
          shellCmdArray[ i ].parser.type = 0;
          shellCmdArray[ i ].parser.length = 0;
          shellCmdArray[ i ].parser.rsp = NULL;
  
          shellCmdCounter++;
          shellCmdIsVliad |= ( ( u32 )0x01 << i );
          return TRUE;  
        }
      } 
    }
  }
  
  return FALSE;  
}







/* 
 * ��������������ע���ӿ�
 * ���ò�����
 *          
 * ����ֵ  ��
 * 
 */
extern bool SHELL_CmdUnSubscribe ( ascii *Cmdstr, shell_atCmdHandler_t Cmdhdlr )
{
  u8 i = 0;  
  
  if ( shellCmdCounter > 0 && Cmdstr && Cmdhdlr )
  {
    for ( i = 0; i < SHELL_CMD_MAX_NUMBER; i++ )
    {
      if ( ( shellCmdIsVliad & ( ( u32 )0x01 << i ) ) 
        && ( shellCmdArray[ i ].cmd.cmdHdlr == Cmdhdlr )
        && strcmp ( ( const char* )shellCmdArray[ i ].cmd.cmdStr, ( const char* )Cmdstr ) == 0 ) 
      {
        shellCmdArray[ i ].cmd.cmdOpt = 0;
        shellCmdArray[ i ].cmd.cmdHdlr = 0;
        strcpy ( ( char * )shellCmdArray[ i ].cmd.cmdStr, "\0" );

        shellCmdArray[ i ].parser.type = 0;
        shellCmdArray[ i ].parser.rsp = NULL;
        shellCmdArray[ i ].parser.length = 0;

        shellCmdCounter--;
        shellCmdIsVliad &= ( ~( ( u32 )0x01 << i ) );
        return TRUE;
      } 
    }
  }

  return FALSE;
}






/* 
 * ����������
 * ���ò�����
 *          
 * ����ֵ  ��
 * 
 */
static void SHELL_ArgsOptionReset ( void )
{
  u8 i = 0;
  	
  shellArgsCounter = 0;
	memset ( shellArgsBuffer, 0, SHELL_ARG_STRING_MAX_LENGTH );
	
	for ( i = 0; i < SHELL_ARG_MAX_NUMBER; i++ )
	{
		shellArgsArray[ i ].argsStartAddress = shellArgsBuffer;
		shellArgsArray[ i ].argsLength = 0;
	}
}






/* 
 * ����������
 * ���ò�����
 *          
 * ����ֵ  ��
 * 
 */
static bool SHELL_ArgsOptionParser ( ascii *response )
{
  s32 totalLeftLength = SHELL_ARG_STRING_MAX_LENGTH;
  bool finish = FALSE;

  if ( response )
  {
    ascii *token = response;
    ascii *found = NULL;
    ascii *startline = NULL;
    ascii *endline = NULL;
   	u8 i = 0;

    for ( i = 0; i < SHELL_ARG_MAX_NUMBER; i++ )
    {
    	startline = token;
      endline = ( ascii* ) strchr ( ( const char* )token, ',' );

      if ( endline != NULL )
      {
        token = endline;
        token++;
      }
	  else
	  {
		endline = token + ( strlen ( ( const char * )token ) - 2 );
		finish = TRUE;
	  }

      found = NULL;
			found = ( ascii* ) strchr ( ( const char* )startline, '\"' );
      if ( found != NULL && found < endline )
      {
        startline = found;
        startline++;
        found = NULL;
        found = ( ascii* ) strchr ( ( const char* )startline, '\"' );
        if ( found != NULL )
        {
        	endline = found;
        }
        else
        {
        	return FALSE;	/* ���ݲ�������������ѡ�� */
        }
      }
			
			shellArgsArray[ shellArgsCounter ].argsLength = ( u8 )( endline - startline + 1 );
			if ( totalLeftLength < shellArgsArray[ shellArgsCounter ].argsLength )
			{
				shellArgsArray[ shellArgsCounter ].argsLength = 0;
				return FALSE;
			}
			
      snprintf ( ( char * )shellArgsArray[ shellArgsCounter ].argsStartAddress, 
                 shellArgsArray[ shellArgsCounter ].argsLength, "%s", startline );

#if 0
      printf ( "\r\nARGS option %u, %s, len %u bytes\r\n", 
               shellArgsCounter, 
               shellArgsArray[ shellArgsCounter ].argsStartAddress, 
               shellArgsArray[ shellArgsCounter ].argsLength );
#endif

      shellArgsArray[ shellArgsCounter ].argsLength += SHELL_ARG_SEPARATOR_LENGTH;
      totalLeftLength -= shellArgsArray[ shellArgsCounter ].argsLength;
      
      if ( finish == TRUE || totalLeftLength <= 0 )
      {
      	return TRUE;	
      }
      else
      {
        shellArgsArray[ shellArgsCounter + 1 ].argsStartAddress 
          = shellArgsArray[ shellArgsCounter ].argsStartAddress + shellArgsArray[ shellArgsCounter ].argsLength;        
        shellArgsCounter++;
      }
    }
  }

  return FALSE;
}








/* 
 * ����������
 * ���ò�����
 *          
 * ����ֵ  ��
 * 
 */
extern ascii* SHELL_CmdParamsGet ( ascii *response, u32 paramsIndex )
{
  if ( response && paramsIndex <= shellArgsCounter )
  {
		return shellArgsArray[ paramsIndex ].argsStartAddress;
  }

  return NULL;
}








/* 
 * ����������
 * ���ò�����
 *          
 * ����ֵ  ��
 * 
 */
extern bool SHELL_CmdParser ( u16 DataSize, u8 *Data )
{
  u32 i = 0, length = 0;  


  if ( DataSize >= SHELL_TMP_STRING_BUFFER_SIZE )
  {
    return FALSE;
  }
  else
  {
    memset ( shellDataBuffer, 0, SHELL_TMP_STRING_BUFFER_SIZE );
    memcpy ( shellDataBuffer, Data, DataSize );
  }

  for ( i = 0; ( i < SHELL_CMD_MAX_NUMBER )&&( shellCmdCounter > 0 ); i++ )
  {
    if ( shellCmdIsVliad & ( ( u32 )0x01 << i ) )
    {
      length = ( u8 ) strlen ( ( const char * )shellCmdArray[ i ].cmd.cmdStr );

      if ( strnicmp ( ( const char * )shellCmdArray[ i ].cmd.cmdStr, 
                      ( const char * )shellDataBuffer, length ) == 0 )
      {
      	ascii *token = NULL;
      	
        if ( strstr ( ( const char * )shellDataBuffer, "\r\n" ) == NULL )
        {
          //return TRUE;
          return FALSE;/*FIX BUG*/
        }
        else
        if ( strstr ( ( const char * )shellDataBuffer, "=?" ) )
        {
          shellCmdArray[ i ].parser.type = ADL_CMD_TYPE_TEST;
        }
        else
        if ( ( token = ( ascii* ) strchr ( ( const char * )shellDataBuffer, '=' ) ) != NULL )
        {
          shellCmdArray[ i ].parser.type = ADL_CMD_TYPE_PARA;
          ++token;
          
          /* ��һ������ */
          SHELL_ArgsOptionReset ();
          SHELL_ArgsOptionParser ( token );
        }
        else
        if ( strchr ( ( const char * )shellDataBuffer, '?' ) )
        {
          shellCmdArray[ i ].parser.type = ADL_CMD_TYPE_READ;
        }
        else
        if ( length <= ( DataSize - 2 ) )
        {
          shellCmdArray[ i ].parser.type = ADL_CMD_TYPE_ACT;  
        }
        
        /* �жϸ�����ѡ���Ƿ�֧�֣� */
        if ( ( shellCmdArray[ i ].cmd.cmdOpt & shellCmdArray[ i ].parser.type ) != 0 )
        {
          if ( shellCmdArray[ i ].cmd.cmdHdlr )
          {
            shellCmdArray[ i ].parser.rsp = shellDataBuffer;
            shellCmdArray[ i ].parser.length = DataSize;
            shellCmdArray[ i ].cmd.cmdHdlr ( ( SHELL_CmdParserTypedef * )&shellCmdArray[ i ].parser );
          }        
        }
        else
        {        
          printf ( "\r\nERROR\r\n" );
        }
        shellCmdArray[ i ].parser.type = 0;
        shellCmdArray[ i ].parser.rsp = NULL;
        shellCmdArray[ i ].parser.length = 0;
        return TRUE; 
      }
    }
  }

  return FALSE;
}



