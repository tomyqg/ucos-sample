#include "tiza_utility.h"




#define OK        0
#ifndef NULL
#define NULL      ( ( void* ) 0 )
#endif


/******************************************/
/*          辅助函数[定义]                */
/******************************************/

/* 不是数字 */ 
#define	 NUM_ERROR	100		    

/* 是整数 */ 
#define	 NUM_INT		101		

/* 是浮点数 */ 
#define	 NUM_FLOAT	102		    


/* 
 * 功能描述：十六进制(字符)转化十进制(数值)
 * 引用参数：
 *          (1)十六进制字符串；
 *          (2)该字符串长度；
 * 
 * 返回值  ：十进制数值
 * 
 */

extern u32 HexToAscii(u8 *pDst,u8 *pSrc,u32 len,bool newline)   
{     
    u8 tab[]="0123456789ABCDEF";       
    u32 i;
	
    for(i=0;i<len;i++)   
    {    
        *pDst++ = tab[(*pSrc) >> 4];      
        *pDst++ = tab[(*pSrc) & 0x0f];   
        pSrc++;   
    } 
		if(newline)
		{
			*pDst++=0x0D;
			*pDst++=0x0A;
			return ((len << 1)+2);
		}    
    return ((len << 1));   
}




/* 
 * 功能描述：十六进制(字符)转化十进制(数值)
 * 引用参数：
 *          (1)十六进制字符串；
 *          (2)该字符串长度；
 * 
 * 返回值  ：十进制数值
 * 
 */

extern u32 HexToInt ( ascii *str, u16 len )    
{
  u32 i, temp = 0, val = 0;

  for ( i = 0; i < len; i++ )
  {
    if ( str[ i ] >= 0x30 &&  str[ i ] <= 0x39 )
    {
        val = str[ i ] - 0x30;
    }
    else if ( str[ i ] >= 0x41 && str[ i ] <= 0x46 )    
    {
        val = str[ i ] - 0x37;
    }
    else if ( str[ i ] >= 0x61 && str[ i ] <= 0x66 )    
    {
        val = str[ i ] - 0x57;
    }
    temp = temp*16 + val;
  }
  return temp;
}








/* 
 * 功能描述：十进制(字符)转化十进制(数值)
 * 引用参数：
 *          (1)十进制字符串；
 *          (2)该字符串长度；
 * 
 * 返回值  ：十进制数值
 * 
 * 
 */
extern u32 DecToInt ( ascii *str, u16 len )    
{
  u32 i, temp = 0, val = 0;

  for ( i = 0; i < len; i++ )
  {
    if ( str[ i ] >= 0x30 && str[ i ] <= 0x39 )
    {
      val  = str[ i ] - 0x30;
      temp = temp*10 + val;
    }
    else	
    {
      return temp;        
    }
  }
  return temp;
}







/* 
 * 功能描述：十进制(字符)转化十进制(数值)
 * 引用参数：
 *          (1)十进制字符串；
 *          (2)该字符串长度；
 * 返回值  ：十进制数值
 * 
 * 
 */
extern unsigned long long DecToLongInt ( ascii *str, u16 len )    
{
  u32 i = 0; 
  unsigned long long  val = 0;
  unsigned long long temp = 0;
  unsigned long long base = 10;
 
  for ( i = 0; i < len; i++ )
  {
    if ( str[ i ] >= 0x30 && str[ i ] <= 0x39 )
    {
      val  = str[ i ] - 0x30;
      temp = temp*base + val;
    }
    else    
    {
      return temp;        
    }
  }
  return temp;
}







/* 
 * 功能描述：数字字符串判断
 * 引用参数：
 *          (1)字符串；
 * 返回值  ：
 *          (1)NUM_ERROR    错误
 *          (2)NUM_FLOAT    浮点数
 *          (3)NUM_INT      整数
 * 
 */
extern s32 IsNumber ( const ascii* cNum )
{
  s32 dot = 0;

  while ( cNum != NULL )
  {
    if ( ( *cNum < 48 || *cNum > 57 ) && *cNum != 46 )
  	{
  		return NUM_ERROR;
  	}
        
    if ( *cNum == 46 )
  	{
  		dot++;    
  	}
        
    if ( dot > 1 )
  	{ 
  		return NUM_ERROR; 
  	}
    cNum++;
  }

  if ( dot > 0 )           
  {
	  return NUM_FLOAT;
  }
  return NUM_INT;
}





/* 
 * 功能描述：将字符(串)转换为整数值
 * 引用参数：
 *          (1)字符串；
 * 返回值  ：整数值
 * 
 */
extern s32 ToInt ( const ascii* cNum )
{
  s32 iNum = 0;       /* 返回值 */

  while ( cNum != NULL )
	{ 
		iNum = iNum*10 + ( s32 )( *cNum - 48 ); 
    cNum++;   
	}
  return iNum;
}


extern s32 stringToInt ( const ascii* cNum,u16 len )
{
  s32 iNum = 0;       /* 返回值 */

  while (len--)
	{ 
		iNum = iNum*10 + ( s32 )( *cNum - 48 ); 
    cNum++;   
	}
  return iNum;
}





/* 
 * 功能描述：将字符(串)转换为浮点数值
 * 引用参数：
 *          (1)字符串；
 * 返回值  ：浮点数值
 * 
 */
extern float ToFloat ( const ascii *cNum )
{
  s32   iNum = 0;         /* 返回值整数部分 */ 
  float fNum = 0.0;       /* 返回值小数部分 */ 
  float Num  = 0.0;       /* 返回的最终数据 */ 
  float t;

  /* 整数部分 */ 
  while ( cNum != NULL && *cNum != 46 )
  {
    iNum = iNum*10 + (s32)( *cNum - 48 );
    cNum++;
  }
  
  /* 小数部分 */ 
  t = 1.0;
  while ( cNum != '\0' )
  {
    fNum = fNum + (s32)( *cNum - 48 )/( t = t*10 );
    cNum++;
  }
  Num = iNum + fNum;
  return Num;
}








/* 
 * 功能描述：字符(串)查找
 * 引用参数：
 *          (1)源字符串；
 *          (2)查找字符串；
 *          (3)源字符串长度；
 *          (4)查找字符串长度；
 * 
 * 返回值  ：查找字符串在源字符串中的首地址。
 * 
 */
extern ascii* strstr_bin ( ascii * str, ascii * part, s32 str_len, s32 part_len )
{
  s32 k = 0;
  
  while ( k < str_len )
  {
    s32 i = k;
    s32 j = 0;

    while ( ( i < str_len )&&( j < part_len ) )
    {
      if ( str[ i ] != part[ j ] )
      {
          break;
      }

      i++;
      j++;

      if ( j == part_len )
      {
          return str+i-part_len;
      }
    }
    k++;
  }
  return NULL;
}








/* 
 * 功能描述：四字节字节倒序
 * 引用参数：
 *          整数；
 * 返回值  ：字节倒序后的整数。
 * 
 */
extern u32 reverseU32 ( u32 i )
{
    u32 j;

	j = ( i & 0xFF ) << 24
    | (( i >> 8  ) & 0xFF ) << 16
    | (( i >> 16 ) & 0xFF ) << 8
    | (( i >> 24 ) & 0xFF );
	return j;
}





/* 
 * 功能描述：两字节字节倒序
 * 引用参数：
 *          整数；
 * 返回值  ：字节倒序后的整数。
 * 
 */
extern u16 reverseU16 ( u16 i )
{
	u16 j = 0;
    
	j = ( i & 0xFF ) << 8 | ( ( i >>8 ) & 0xFF );
	return j;
}











/*!
 * \brief Convert decimal dotted ASCII representation into numeric IP address.
 *
 * \param str String containing the ASCII representation.
 *
 * \return IP address in network byte order.
 */
extern u32 inet_addr ( const char * str )
{
  u16 num;
  u32 addr = 0;
  u8 parts = 0;
  u8 *ap;

  ap = (u8 *) & addr;
  while (parts < 4) 
  {
      if (*str < '0' || *str > '9')
          break;
      for (num = 0; num <= 255;) 
      {
        num = (num * 10) + (*str - '0');
        if (*++str < '0' || *str > '9')
            break;
      }
      if (num > 255)
          break;
      parts++;
      *ap++ = (u8) num;
      if (*str != '.') 
      {
        if (parts == 4)
            return addr;
        break;
      }
      str++;
  }
  return 0;
}









/*!
 * \brief Convert numeric IP address into decimal dotted 
 *        ASCII representation.
 *
 * \note This function is not thread safe. Each subsequent
 *       call will destroy the previous result. Applications
 *       should locally store the result before calling the 
 *       function again or allowing other threads to call it.
 *
 * \param addr IP address in network byte order.
 *
 * \return Pointer to a static buffer containing the 
 *         ASCII representation.
 */
extern char *inet_ntoa ( u32 addr )
{
  static char str[ 16 ];
  char inv[ 3 ];
  char *rp;
  u8 *ap;
  u8 rem;
  u8 n;
  u8 i;

  rp = str;
  ap = (u8 *) & addr;
  for (n = 0; n < 4; n++) 
  {
    i = 0;

    do 
    {
      rem = *ap % (u8) 10;
      *ap /= (u8) 10;
      inv[i++] = '0' + rem;
    } while (*ap);

    while (i--)
        *rp++ = inv[i];
    *rp++ = '.';
    ap++;
  }
  *--rp = 0;
  return str;
}

