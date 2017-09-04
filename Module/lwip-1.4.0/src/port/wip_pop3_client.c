/*
 * �������ڣ�2009-8-25 8:23:09
 *
 * �������У���������
 *          Ӳ��������
 * ԭʼ�ļ���
 * 
 * �ļ��汾��v1.00
 * 
 * ����������Q24Ӧ�ó������ڵ�
 * 
 * [�޸���־]
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 */



/* 
 * ����������
 * ���ò�����
 * 
 * ����ֵ  ��
 * 
 */
static bool wip_POP3RecvDataHdlr ( u16 DataSize, u8 *Data ) 
{
  
//////////////////////////////////////////////////
  if ( sysDebugCtroller.debugIsValid == TRUE )
  {
    USART_WriteData ( sysDebugCtroller.debugPort, Data, DataSize );
  }
//////////////////////////////////////////////////



  if ( DataSize > 2 )
	{	
		if ( strstr ( ( const char* )Data, "220" ) )
		{
      command ( "HELO",  );  
    }

		if ( strstr ( ( const char* )Data, "250" ) )
		{
      command ( "VRFY",  );
      command ( "MAIL FROM",  );
      command ( "RCPT TO",  ); 
      command ( "DATA",  );
      command ( ".", NULL );
      command ( "QUIT", NULL );
    }

		if ( strstr ( ( const char* )Data, "354" ) )
		{
      command ( "<HEADERS>", ftpclient.userPass ); 
    }

		if ( strstr ( ( const char* )Data, "550" ) )
		{
      printf ( "\r\n***550" );
    }
	}  
  return TRUE;
}





/* 
 * ����������
 * ���ò�����
 * 
 * ����ֵ  ��
 * 
 */
static bool wip_POP3DnsProcHandler ( const char *name, u32 *ipaddr )
{
  if ( name && ipaddr )
  {
    smtpMailServerAddr = *ipaddr;
    smtpSessionSock = wip_TCPClientCreate ( smtpMailServerAddr, 25, ( tcp_procDataHdlr_f )wip_SMTPRecvDataHdlr );
  }
}






/* 
 * ����������
 * ���ò�����
 * 
 * ����ֵ  ��
 * 
 */
extern void wip_POP3QueryMailServerAddress ( const char *hostname )
{
  wip_DNSQueryHostIPAddress ( hostname, ( dns_procDataHdlr_f )wip_POP3DnsProcHandler );
}






