/*
 * 创建日期：2009-8-25 8:23:09
 *
 * 创建所有：江苏天泽
 *          硬件开发部
 * 原始文件：
 * 
 * 文件版本：v1.00
 * 
 * 内容描述：Q24应用程序的入口点
 * 
 * [修改日志]
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
 * 功能描述：
 * 引用参数：
 * 
 * 返回值  ：
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
 * 功能描述：
 * 引用参数：
 * 
 * 返回值  ：
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
 * 功能描述：
 * 引用参数：
 * 
 * 返回值  ：
 * 
 */
extern void wip_POP3QueryMailServerAddress ( const char *hostname )
{
  wip_DNSQueryHostIPAddress ( hostname, ( dns_procDataHdlr_f )wip_POP3DnsProcHandler );
}






