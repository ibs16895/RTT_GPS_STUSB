#ifndef _JT808_GPS_H_
#define _JT808_GPS_H_

#include <rtthread.h>
#include "stm32f4xx.h"


#define BIT_STATUS_ACC	0x01
#define BIT_STATUS_GPS	0x02
#define BIT_STATUS_NS	0x04
#define BIT_STATUS_EW	0x08



#define BIT_ALARM_EMG	0x01



/*基本位置信息,因为字节对齐的方式，还是使用数组方便*/
typedef __packed struct _gps_baseinfo
{
	uint32_t alarm;
	uint32_t status;
	uint32_t latitude; /*纬度*/
	uint32_t longitude;/*精度*/
	uint16_t altitude;
	uint16_t spd_10x;		/*对地速度 0.1KMH*/
	uint16_t cog;			/*对地角度*/
	uint8_t datetime[6];
}GPS_BASEINFO;


enum BDGPS_MODE 
{
	MODE_GET=0,	/*查询*/
	MODE_BD=1,
	MODE_GPS,
	MODE_BDGPS,
};


typedef  struct  _gps_status
{
   enum BDGPS_MODE   Position_Moule_Status;  /* 1: BD   2:  GPS   3: BD+GPS    定位模块的状态*/
   uint8_t  Antenna_Flag;//显示提示开路 
   uint8_t  Raw_Output;   //  原始数据输出  
   uint8_t	NoSV;
}GPS_STATUS;

extern GPS_STATUS	gps_status;

extern GPS_BASEINFO gps_baseinfo;

/*告警和状态信息*/
extern uint32_t		jt808_alarm;
extern uint32_t		jt808_status;

void gps_rx( uint8_t * pinfo, uint16_t length );
void jt808_gps_init( void );




#endif
