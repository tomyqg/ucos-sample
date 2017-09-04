/*****************************************************************************
* pppdebug.h - System debugging utilities.
*
* Copyright (c) 2003 by Marc Boucher, Services Informatiques (MBSI) inc.
* portions Copyright (c) 1998 Global Election Systems Inc.
* portions Copyright (c) 2001 by Cognizant Pty Ltd.
*
* The authors hereby grant permission to use, copy, modify, distribute,
* and license this software and its documentation for any purpose, provided
* that existing copyright notices are retained in all copies and that this
* notice and the following disclaimer are included verbatim in any 
* distributions. No written agreement, license, or royalty fee is required
* for any of the authorized uses.
*
* THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS *AS IS* AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
* IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************
* REVISION HISTORY (please don't use tabs!)
*
* 03-01-01 Marc Boucher <marc@mbsi.ca>
*   Ported to lwIP.
* 98-07-29 Guy Lancaster <lancasterg@acm.org>, Global Election Systems Inc.
*   Original.
*
*****************************************************************************
*/




#include <stdio.h>
#include <stdarg.h>

#include "stm32f10x_type.h"

#include "pppdebug.h"



/***********************
*** PUBLIC FUNCTIONS ***
***********************/
/*
 * ppp_trace - a form of printf to send tracing information to stderr
 */
void ppp_trace(int level, const char *format,...)
{
  ascii output[ 256 ];
  va_list va_args;

  va_start ( va_args, format );
  vsprintf ( ( char * )output, format, va_args );
  va_end ( va_args );

	switch ( level )
	{
		case LOG_CRITICAL:
			printf ( "\r\nLOG_CRITICAL\t" );
			break;
		case LOG_ERR:
			printf ( "\r\nLOG_ERR\t" );		
			break;
		case LOG_NOTICE:
			printf ( "\r\nLOG_NOTICE\t" );
			break;
		case LOG_WARNING:
			printf ( "\r\nLOG_WARNING\t" );
			break;
		case LOG_INFO:
			printf ( "\r\nLOG_INFO\t" );
			break;
		case LOG_DETAIL:
			printf ( "\r\nLOG_DETAIL\t" );
			break;
		case LOG_DEBUG:
			printf ( "\r\nLOG_DEBUG\t" );
			break;
			
		default:
			break;		
	}
	
  printf ( ( char * )output );	
}



