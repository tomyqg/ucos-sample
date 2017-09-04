



#ifndef __TZ_CAN_H__
#define __TZ_CAN_H__



#include "stm32f4xx.h"
#include "tiza_include.h"


/* CAN报文处理 */
typedef bool ( *can_procDataHdlr_f ) (u32 frameID, u16 DataSize, u8 *Data );



/* 接受缓冲区 */
#define MAX_CAN_BUF_LENGTH            24


/******************************************/
/*            CAN工作模式[配置]           */
/******************************************/
#define CAN_MODE_NORMAL           0x00    /* 正常模式 */
#define CAN_MODE_LISTENER         0x01    /* 侦听模式 */       
#define CAN_MODE_TEST             0x02    /* 热自测试模式 */





/******************************************/
/*            TZ-CAN参数[配置]            */
/******************************************/

#ifndef CAN_RECVED_BUFFER_LENGTH
#define CAN_RECVED_BUFFER_LENGTH      96    /* 单位：bytes */
#endif  /* CAN_RECVED_BUFFER_LENGTH */


#ifndef CAN_TOSEND_BUFFER_LENGTH
#define CAN_TOSEND_BUFFER_LENGTH      32    /* 单位：bytes */
#endif  /* CAN_TOSEND_BUFFER_LENGTH */


//#ifndef CAN_CONTROLLER_ID
//#define CAN_CONTROLLER_ID             ( 0x0000 )
//#endif  /* CAN_CONTROLLER_ID */


#ifndef CAN_LENGTH_BYTES
#define CAN_LENGTH_BYTES               1    /* 单位：bytes */
#endif  /* CAN_LENGTH_BYTES */





enum CAN_STATUS
{
	CAN_STATUS_BEGIN = 0,
	CAN_STATUS_Unknow,
	CAN_STATUS_OK,
	CAN_STATUS_Bus_Off,
	CAN_STATUS_Error_Warning,
	CAN_STATUS_Error_Passive,
	CAN_STATUS_Error,
	CAN_STATUS_Wakeup,
	CAN_STATUS_Sleep,
	CAN_STATUS_Last_Error_Code,
	CAN_STATUS_END
};  /*  CAN模块状态  */


enum  
{ 
    CAN_BUS_BDRT_10K  = 10,
	CAN_BUS_BDRT_20K  = 20,
	CAN_BUS_BDRT_40K  = 40,
	CAN_BUS_BDRT_50K  = 50,
	CAN_BUS_BDRT_80K  = 80,
	CAN_BUS_BDRT_100K = 100,
	CAN_BUS_BDRT_125K = 125,
	CAN_BUS_BDRT_200K = 200,
	CAN_BUS_BDRT_250K = 250,
	CAN_BUS_BDRT_400K = 400,
	CAN_BUS_BDRT_500K = 500,
	/*CAN_BUS_BDRT_666K = 666, 暂时处理不了*/
	CAN_BUS_BDRT_800K = 800, 
	CAN_BUS_BDRT_1M   = 1000 
};	/*  CAN模块波特率  */


/******************************************/
/*            CAN数据帧[配置]             */
/******************************************/
#define CAN_FRAME_EXT_AND_STD     0x00    /* 混合帧 */   
#define CAN_FRAME_EXT             0x01    /* 扩展帧 */         
#define CAN_FRAME_STD             0x02    /* 标准帧 */ 


typedef struct 
{
	/* 是否有效状态 */
	bool IsValid;  

	enum CAN_STATUS Status;

	/* 数据处理句柄 */
	can_procDataHdlr_f  DataHandler;

	/* 运行模式 */
	u8 CAN_RUN;

	/* 同步跳跃 */
	u8 CAN_Sjw;

	/* 时间段1 */
	u8 CAN_Bs1;

	/* 时间段2 */
	u8 CAN_Bs2;

	/* CAN BRP */
	u16 CAN_BRP;

	/* 帧类型 */
	u8 CAN_FrameType;
	
	u32 CAN_FrameID;

	u8 RxBuffer[ MAX_CAN_BUF_LENGTH ];
	u8 RxBufSiz;
} CAN_DevTypedef;  /* CAN模块初始化配置 */


/* CAN Param configure Structure definition */
typedef struct 
{
	/* CAN 波特率 */
	u32 u32Baudrate;

	can_procDataHdlr_f DataHandler;

} CAN_Paramdef;



typedef struct
{
  /* 控制器厂家编号 */
  u16 ControllerID;

  /* 长度字段长度 */
  u8  LengthOpt;

  /* 数据接收长度 */
  u16 DataSize;

  /* 数据存储地址 */
  u8* Data;
} can_info_t;



typedef struct
{
  /* 数据发送类型 */
  u32 sendType;

  /* 数据长度 */
  u8 length;

  /* 控制器数据 */
  u8 data[ CAN_TOSEND_BUFFER_LENGTH ];
} can_data_t;





/* CAN数据备份 */
extern can_data_t canBackupData;
extern  u8 * CanSendBuffer;


















/******************************************/
/*              外部函数[声明]            */
/******************************************/

/* CAN应用接口初始化 */
extern void TZ_CAN_ApplicationIfInit ( void );

/* CAN周期事务处理 */
extern void CAN_ProcDataHdlr ( void );

/* 中心设置CAN控制下发的参数 */
extern bool CAN_SetControllerParamsByServer ( u16 controllerID, u16 DataSize, u8 *IPData );

/* CAN备份域数据复位 */
extern void CAN_BackupBufferReset ( void );

/* CAN备份域数据读取 */
extern void CAN_BackupBufferRead ( void );

/* 获取CAN控制数据信息 */
extern can_info_t* CAN_GetDataInfo ( void );


uint8 NssInit(void);

#endif  /* __TZ_CAN_H__ */



