/************************************************************
 * Copyright (C), 2008-2012,
 * FileName:		m66
 * Author:			bitter
 * Date:			// ����
 * Description:		// ģ������
 * Version:			// �汾��Ϣ
 * Function List:	// ��Ҫ�������书��
 *     1. -------
 * History:			// ��ʷ�޸ļ�¼
 *     <author>  <time>   <version >   <desc>
 *     David    96/10/12     1.0     build this moudle
 ***********************************************************/

#include <stdio.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "stm32f4xx.h"

#include <finsh.h>

#include <gsm.h>

#include "jt808.h"

#ifdef M66

#define GSM_GPIO                        GPIOC
#define GSM_TX_PIN                      GPIO_Pin_10
#define GSM_TX_PIN_SOURCE       GPIO_PinSource10

#define GSM_RX_PIN                      GPIO_Pin_11
#define GSM_RX_PIN_SOURCE       GPIO_PinSource11

typedef void ( *URC_CB )( char *s, uint16_t len );
typedef rt_err_t ( *RESP_FUNC )( char *s, uint16_t len );

#define GSM_PWR_PORT    GPIOD
#define GSM_PWR_PIN     GPIO_Pin_13

#define GSM_TERMON_PORT GPIOD
#define GSM_TERMON_PIN  GPIO_Pin_12

#define GSM_RST_PORT    GPIOD
#define GSM_RST_PIN     GPIO_Pin_11

/*����һ��gsm�豸*/
static struct rt_device dev_gsm;


/*����һ��uart�豸ָ��,ͬgsmģ�����ӵĴ���
   ָ��һ���Ѿ��򿪵Ĵ���
 */
static struct rt_mailbox mb_gsmrx;
#define MB_GSMRX_POOL_SIZE 32
static uint8_t mb_gsmrx_pool[MB_GSMRX_POOL_SIZE];

#define GSM_RX_SIZE 2048
static uint8_t gsm_rx[GSM_RX_SIZE];
static uint16_t gsm_rx_wr = 0;

static T_GSM_STATE gsm_state               = GSM_IDLE;
static T_GSM_STATE socket_state    = SOCKET_IDLE;

static rt_thread_t tid_gsm_subthread = RT_NULL;

/*���ڽ��ջ���������*/
#define UART4_RX_SIZE 128
static uint8_t uart4_rxbuf[UART4_RX_SIZE];
struct rt_ringbuffer rb_uart4_rx;

static uint8_t fgsm_rawdata_out = 1;

/*���һ���յ��������ݵ�ʱ��,��ʹ��sem��Ϊ��ʱ�ж�*/
static uint32_t last_tick;

struct _gsm_param
{
	char imei[16];
	char imsi[16];
	uint8_t csq;
} gsm_param;

/*���֧��4������*/
#define MAX_SOCKET_NUM 4
struct
{
	char connect_state;                 /*����״̬0:������ 1:���þ����� 3:�ȴ�DNS�� 4:������*/
	char domain_name[32];               /*����*/
	char ip[16];                        /*dns���IP*/
	uint16_t port;                      /**/
} gsm_socket[MAX_SOCKET_NUM];


/***********************************************************
* Function:
* Description: uart4���жϷ�����
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void UART4_IRQHandler( void )
{
	rt_interrupt_enter( );
	if( USART_GetITStatus( UART4, USART_IT_RXNE ) != RESET )
	{
		rt_ringbuffer_putchar( &rb_uart4_rx, USART_ReceiveData( UART4 ) );
		USART_ClearITPendingBit( UART4, USART_IT_RXNE );
		//rt_sem_release( &sem_uart );
		last_tick = rt_tick_get( );
	}


/*
   if (USART_GetITStatus(UART4, USART_IT_TC) != RESET)
   {
   USART_ClearITPendingBit(UART4, USART_IT_TC);
   }
 */
	rt_interrupt_leave( );
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static void urc_cb_default( char *s, uint16_t len )
{
	rt_kprintf( "\rrx>%s", s );
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static void urc_cb_ciev( char *s, uint16_t len )
{
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static void urc_cb_ring( char *s, uint16_t len )
{
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static void urc_cb_cring( char *s, uint16_t len )
{
}

/*
   urc: unsolicited result code
 */
struct
{
	char    *code;
	URC_CB pfunc;
} urc[] =
{
	//{ "^SYSSTART", urc_cb_sysstart },
	//{ "^SHUTDOWN", urc_cb_shutdown },
	{ "+CIEV:",        urc_cb_ciev    },
	{ "RING",          urc_cb_ring    },
	{ "+CRING:",   urc_cb_cring       },
	{ "+CREG:",        urc_cb_default },
	{ "^SIS:",         urc_cb_default },
	{ "+CGEV:",        urc_cb_default },
	{ "+CGREG:",   urc_cb_default },
	{ "+CMT:",         urc_cb_default },
	{ "+CBM:",         urc_cb_default },
	{ "+CDS:",         urc_cb_default },
	{ "+CALA:",        urc_cb_default },
	{ "CME ERROR", urc_cb_default },
	{ "CMS ERROR", urc_cb_default },
	{ "",              NULL                   }
};


/***********************************************************
* Function:		sys_default_cb
* Description:	ȱʡ��ϵͳ�ص���������
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/

static void sys_default_cb( char *pInfo, uint16_t len )
{
	int i;
	char match = 0;

	rt_kprintf( "\rrx>%s", pInfo );
/*�п����������Ҷϣ�Ҳ�п����������ӶϿ����Ƿ�Ҫ֪ͨ? ����Ҫ*/
	if( strncmp( pInfo, "%IPCLOSE", 7 ) == 0 )
	{
	}

//�жϷ���������-�����ϱ�����
	for( i = 0;; i++ )
	{
		if( urc[i].pfunc == NULL )
		{
			break;
		}
		if( strncmp( pInfo, urc[i].code, strlen( urc[i].code ) == 0 ) )
		{
			( urc[i].pfunc )( pInfo, len );
			match = 1; //�Ѵ���
			break;
		}
	}
	if( match )
	{
		return;
	}

//AT����Ľ���������������������������APP������
}

/***********************************************************
* Function:
* Description: ��С��0x20���ַ����Ե������ڽ�β����0��תΪ
   �ɼ����ַ�����
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static uint16_t strip_string( char *str )
{
	char            *psrc, *pdst;
	uint16_t len = 0;
	psrc    = str;
	pdst    = str;
	while( *psrc )
	{
		if( *psrc > 0x20 )
		{
			*pdst++ = toupper( *psrc );
			len++;
		}
		psrc++;
	}
	*pdst = 0;
	return len;
}

/***********************************************************
* Function:
* Description: ��С��0x20���ַ����Ե���ֻ�������ֲ��֡�
               ���ڽ�β����0��תΪ�ɼ����ַ�����
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static uint16_t strip_numstring( char *str )
{
	char            *psrc, *pdst;
	uint16_t len = 0;
	psrc    = str;
	pdst    = str;
	while( *psrc )
	{
		if( ( *psrc >= '0' ) && ( *psrc <= '9' ) )
		{
			*pdst++ = *psrc;
			len++;
		}
		else
		{
			if( len )
			{
				break;
			}
		}
		psrc++;
	}
	*pdst = 0;
	return len;
}

/***********************************************************
* Function:
* Description: ���ÿص�ܽţ����ö�Ӧ�Ĵ����豸uart4
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static rt_err_t m66_init( rt_device_t dev )
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD, ENABLE );
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_UART4, ENABLE );

	GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;

	GPIO_InitStructure.GPIO_Pin = GSM_PWR_PIN;
	GPIO_Init( GSM_PWR_PORT, &GPIO_InitStructure );
	GPIO_ResetBits( GSM_PWR_PORT, GSM_PWR_PIN );

	GPIO_InitStructure.GPIO_Pin = GSM_TERMON_PIN;
	GPIO_Init( GSM_TERMON_PORT, &GPIO_InitStructure );
	GPIO_ResetBits( GSM_TERMON_PORT, GSM_TERMON_PIN );


/*
   RESET�ڿ������̲���Ҫ���κ�ʱ����ϣ���ͨ��CPU �� reset��ͬ����
   ����ùܽŽ�OC�����GPIO������ʱ OC ������衣
 */
	GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType   = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;

	GPIO_InitStructure.GPIO_Pin = GSM_RST_PIN;
	GPIO_Init( GSM_RST_PORT, &GPIO_InitStructure );
	GPIO_SetBits( GSM_RST_PORT, GSM_RST_PIN );

/*uart4 �ܽ�����*/

	/* Configure USART Tx as alternate function  */
	GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin             = GSM_TX_PIN | GSM_RX_PIN;
	GPIO_Init( GSM_GPIO, &GPIO_InitStructure );

	GPIO_PinAFConfig( GSM_GPIO, GSM_TX_PIN_SOURCE, GPIO_AF_UART4 );
	GPIO_PinAFConfig( GSM_GPIO, GSM_RX_PIN_SOURCE, GPIO_AF_UART4 );

	NVIC_InitStructure.NVIC_IRQChannel                                              = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority    = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority                   = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd                                   = ENABLE;
	NVIC_Init( &NVIC_InitStructure );

	USART_InitStructure.USART_BaudRate                              = 57600;
	USART_InitStructure.USART_WordLength                    = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits                              = USART_StopBits_1;
	USART_InitStructure.USART_Parity                                = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl   = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode                                  = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init( UART4, &USART_InitStructure );
	/* Enable USART */
	USART_Cmd( UART4, ENABLE );
	USART_ITConfig( UART4, USART_IT_RXNE, ENABLE );

	return RT_EOK;
}

/***********************************************************
* Function:	�ṩ������thread���ã����豸����ʱ�ж�
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static rt_err_t m66_open( rt_device_t dev, rt_uint16_t oflag )
{
	if( gsm_state == GSM_IDLE )
	{
		gsm_state = GSM_POWERON; //��λ�ϵ������
	}
	return RT_EOK;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static rt_err_t m66_close( rt_device_t dev )
{
	gsm_state = GSM_POWEROFF; //��λ�ϵ������
	return RT_EOK;
}

/***********************************************************
* Function:m66_read
* Description:����ģʽ�¶�ȡ����
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static rt_size_t m66_read( rt_device_t dev, rt_off_t pos, void* buff, rt_size_t count )
{
	return RT_EOK;
}

/* write one character to serial, must not trigger interrupt */
static void uart4_putc( const char c )
{
	USART_SendData( UART4, c );
	while( !( UART4->SR & USART_FLAG_TXE ) )
	{
		;
	}
	UART4->DR = ( c & 0x1FF );
}

/***********************************************************
* Function:		m66_write
* Description:	����ģʽ�·������ݣ�Ҫ�����ݽ��з�װ
* Input:		const void* buff	Ҫ���͵�ԭʼ����
       rt_size_t count	Ҫ�������ݵĳ���
       rt_off_t pos		ʹ�õ�socket���
* Output:
* Return:
* Others:
***********************************************************/

static rt_size_t m66_write( rt_device_t dev, rt_off_t pos, const void* buff, rt_size_t count )
{
	rt_size_t len = count;
	uint8_t         *p      = (uint8_t*)buff;

	while( len )
	{
		USART_SendData( UART4, *p++ );
		while( USART_GetFlagStatus( UART4, USART_FLAG_TC ) == RESET )
		{
		}
		len--;
	}
	return RT_EOK;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static rt_err_t m66_control( rt_device_t dev, rt_uint8_t cmd, void *arg )
{
	switch( cmd )
	{
	case CTL_STATUS:
		break;
	case CTL_AT_CMD:         //����at����,���Ҫ����
		break;
	case CTL_PPP:
		break;
	case CTL_SOCKET:
		break;
	}
	return RT_EOK;
}

/*
   +CREG:0,1
   +CGREG:0,5
 */
rt_err_t RespFunc_CGREG( char *p, uint16_t len )
{
	uint32_t i, n, code;

	i = sscanf( p, "%*[^:]:%d,%d", &n, &code );
	if( i != 2 )
	{
		return RT_ERROR;
	}
	if( ( code != 1 ) && ( code != 5 ) )
	{
		return RT_ERROR;
	}
	return RT_EOK;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
rt_err_t RespFunc_COPS( char *p, uint16_t len )
{
/*
   char *oper=(char*)0;
   oper=strstr(p,"CHN-CUGSM");
   if(oper){
   strcpy((char*)sys_param.apn,"UNINET");
   return AT_RESP_OK;
   }
   oper=strstr(p,"CHINAMOBILE");
   if(oper){
   strcpy((char*)sys_param.apn,"CMNET");
   return AT_RESP_OK;
   }
 */
	return RT_EOK;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
rt_err_t RespFunc_CIFSR( char *p, uint16_t len )
{
	uint8_t i       = 0;
	rt_err_t res = RT_ERROR;
	int32_t PARAM_LocalIP[4];
	i = sscanf( p, "%u.%d.%d.%d",
	            &( PARAM_LocalIP[0] ),
	            &( PARAM_LocalIP[1] ),
	            &( PARAM_LocalIP[2] ),
	            &( PARAM_LocalIP[3] ) );
	if( i == 4 )
	{
		res = RT_EOK;
	}
	return res;
}

/*
   SIM����IMSI����Ϊ4600 00783208249��
      460 00 18 23 20 86 42

   �ӿں������ֶε�����ΪIMSI����ĺ�12λ
   ��6���ֽڵ�����Ϊ 0x00 0x07 0x83 0x20 0x82 0x49

 */
rt_err_t RespFunc_CIMI( char *p, uint16_t len )
{
	char *pimsi, i;
	rt_kprintf( "cimi len=%d\n", len );
	if( len < 15 )
	{
		return RT_ERROR;
	}
	strip_numstring( p );
	strcpy( gsm_param.imsi, p );
	return RT_EOK;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
rt_err_t RespFunc_CGSN( char *p, uint16_t len )
{
	char *pimsi, i;
	if( len < 15 )
	{
		return RT_ERROR;
	}
	strip_numstring( p );
	strcpy( gsm_param.imsi, p );
	return RT_EOK;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
rt_err_t RespFunc_CPIN( char *p, uint16_t len )
{
	char *pstr;
	pstr = strstr( p, "+CPIN: READY" );
	if( pstr )
	{
		return RT_EOK; /*�ҵ���*/
	}
	return RT_ERROR;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:		+CSQ: 31, 99
* Return:
* Others:
***********************************************************/
rt_err_t RespFunc_CSQ( char *p, uint16_t len )
{
	rt_err_t res = RT_ERROR;
	uint32_t i, n, code;

	i = sscanf( p, "+CSQ%*[^:]:%d,%d", &n, &code );
	if( i != 2 )
	{
		return RT_ERROR;
	}
	gsm_param.csq = n;
	return RT_EOK;
}

/*�ȴ��̶���Ϣ�ķ���*/
rt_err_t WaitResp( char *resp, rt_tick_t ticks )
{
	rt_tick_t start           = rt_tick_get( );
	rt_tick_t localticks      = ticks;
	char            *pstr, *p;
	rt_err_t ret;

/*�ȴ��յ���Ϣ*/
lbl_waitresp_again:
	ret = rt_mb_recv( &mb_gsmrx, (rt_uint32_t*)&pstr, localticks );
	if( ret == -RT_ETIMEOUT )
	{
		return ret;                                 //��ʱ�˳���û�����ݻ�����������
	}
	strip_string( pstr );
	p = strstr( pstr, resp );
	if( p )
	{
		rt_free( pstr );
		return RT_EOK;                              /*�ҵ���*/
	}
	/*û�ҵ��������ȣ������Ϣ��δ���?���������ŵ�֪ͨ��URC*/
	localticks = start + ticks - rt_tick_get( );    /*����ʣ���ʱ��*/
	if( localticks > 0 )
	{
		goto lbl_waitresp_again;
	}
	return RT_ETIMEOUT;
}

/*
   ���ָ��ʱ���ڵķ������
   ������ר�ŵĺ����������ؽ��
 */
int8_t CheckResp( uint32_t ticks, RESP_FUNC resp_func )
{
	rt_tick_t start           = rt_tick_get( );
	rt_tick_t localticks      = ticks;
	rt_err_t ret                     = RT_EOK;
	char            * pstr;
/*�ȴ��յ���Ϣ*/
lbl_checkresp_again:
	ret = rt_mb_recv( &mb_gsmrx, (rt_uint32_t*)&pstr, localticks );
	if( ret == -RT_ETIMEOUT )
	{
		//rt_free(pstr);		/*û���յ����ݣ������ͷ�*/
		return ret; //��ʱ�˳���û�����ݻ�����������
	}
	/*�յ���Ϣ��Ҫ�ж��ǲ�����Ҫ��*/
	ret = resp_func( pstr, strlen( pstr ) );
	if( ret == RT_EOK )
	{
		rt_free( pstr );
		return RT_EOK;
	}
	localticks = start + ticks - rt_tick_get( ); /*����ʣ���ʱ��*/
	if( localticks > 0 )
	{
		goto lbl_checkresp_again;
	}
	return RT_ETIMEOUT;
}

/***********************************************************
* Function:		SendATCmdWaitRespStr
* Description:	��������ȴ�����ָ�����ַ���
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
int8_t SendATCmdWaitRespStr( char *atcmd,
                             uint32_t ticks,
                             char * respstring,
                             uint8_t no_of_attempts )
{
	rt_err_t ret_val = RT_ERROR;
	uint8_t i;

	for( i = 0; i < no_of_attempts; i++ )
	{
		rt_kprintf( "gsm_send>%s", atcmd );
		m66_write( &dev_gsm, 0, atcmd, strlen( atcmd ) );
		ret_val = WaitResp( respstring, ticks );
		if( ret_val == RT_EOK )
		{
			break;
		}
	}
	return ( ret_val );
}

/***********************************************************
* Function:		SendATCmdWaitRespFunc
* Description:	��������������ؽ������ָ���Ĵ�������
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
int8_t SendATCmdWaitRespFunc( char *atcmd,
                              uint32_t ticks,
                              RESP_FUNC respfunc,
                              uint8_t no_of_attempts )
{
	int8_t ret_val = RT_ERROR;
	uint8_t i;

	for( i = 0; i < no_of_attempts; i++ )
	{
		rt_kprintf( "gsm_send>%s", atcmd );
		m66_write( &dev_gsm, 0, atcmd, strlen( atcmd ) );
		ret_val = CheckResp( ticks, respfunc );
		if( ret_val == RT_EOK )
		{
			break;
		}
	}
	return ( ret_val );
}

/*�߳��˳���cleanup����*/
void cleanup( struct rt_thread *tid )
{
	tid_gsm_subthread = RT_NULL;
}

/***********************************************************
* Function:
* Description:	ģ���ϵ粢���AT�����ʼ��
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static void gsm_poweron( void* parameter )
{
	rt_err_t res;

/*�˴���ͨ��rt_thread_self��ȡ�߳���Ϣ*/
lbl_start_pwr_on:

	GPIO_SetBits( GSM_PWR_PORT, GSM_PWR_PIN );
	GPIO_SetBits( GSM_TERMON_PORT, GSM_TERMON_PIN );
	rt_thread_delay( RT_TICK_PER_SECOND / 10 );
	GPIO_ResetBits( GSM_TERMON_PORT, GSM_TERMON_PIN );
	rt_thread_delay( RT_TICK_PER_SECOND );
	GPIO_SetBits( GSM_TERMON_PORT, GSM_TERMON_PIN );
/*
	res = WaitResp( "AT-Command Interpreter ready", RT_TICK_PER_SECOND * 5 );
	if( res != RT_EOK )
	{
		rt_kprintf( "\r\n%d>re gsm pwron\r\n", rt_tick_get( ) );
		GPIO_SetBits( GSM_TERMON_PORT, GSM_TERMON_PIN );
		GPIO_SetBits( GSM_PWR_PORT, GSM_PWR_PIN );
		rt_thread_delay( RT_TICK_PER_SECOND * 3 );
		goto lbl_start_pwr_on;
	}
*/	
	res = WaitResp( "OK", RT_TICK_PER_SECOND * 5 );
	if( res != RT_EOK )
	{
		rt_kprintf( "\r\n%d>re gsm pwron\r\n", rt_tick_get( ) );
		GPIO_SetBits( GSM_TERMON_PORT, GSM_TERMON_PIN );
		GPIO_SetBits( GSM_PWR_PORT, GSM_PWR_PIN );
		rt_thread_delay( RT_TICK_PER_SECOND * 3 );
		goto lbl_start_pwr_on;
	}

	res = WaitResp( "OK", RT_TICK_PER_SECOND * 5 );
	if( res != RT_EOK )
	{
		rt_kprintf( "\r\n%d>re gsm pwron\r\n", rt_tick_get( ) );
		GPIO_SetBits( GSM_TERMON_PORT, GSM_TERMON_PIN );
		GPIO_SetBits( GSM_PWR_PORT, GSM_PWR_PIN );
		rt_thread_delay( RT_TICK_PER_SECOND * 3 );
		goto lbl_start_pwr_on;
	}


	
	res = SendATCmdWaitRespStr( "AT\r\n", 3 * RT_TICK_PER_SECOND, "OK", 3 );
	res = SendATCmdWaitRespStr( "ATE0\r\n", 3 * RT_TICK_PER_SECOND, "OK", 3 );
	res = SendATCmdWaitRespStr( "ATV1\r\n", 3 * RT_TICK_PER_SECOND, "OK", 3 );
	res = SendATCmdWaitRespFunc( "AT+CPIN?\r\n", 3 * RT_TICK_PER_SECOND, RespFunc_CPIN, 3 );
	if( res != RT_EOK )
	{
		goto lbl_start_pwr_on;
	}
	res                     = SendATCmdWaitRespFunc( "AT+COPS?\r\n", 3 * RT_TICK_PER_SECOND, RespFunc_COPS, 10 );
	res                     = SendATCmdWaitRespFunc( "AT+CREG?\r\n", 3 * RT_TICK_PER_SECOND, RespFunc_CGREG, 10 );
	res                     = SendATCmdWaitRespFunc( "AT+CIMI\r\n", 3 * RT_TICK_PER_SECOND, RespFunc_CIMI, 3 );
	res                     = SendATCmdWaitRespFunc( "AT+CGREG?\r\n", 3 * RT_TICK_PER_SECOND, RespFunc_CGREG, 10 );
	res                     = SendATCmdWaitRespStr( "AT+CGATT?\r\n", 3 * RT_TICK_PER_SECOND, "+CGATT:1", 10 );
	gsm_state       = GSM_AT; /*�л���AT״̬*/
}

/***********************************************************
* Function:	rt_thread_entry_gsm_poweroff
* Description: �Ҷϵ�ǰ���ӣ����ر�ģ��
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static void gsm_poweroff( void* parameter )
{
	rt_err_t res;

/*�˴���ͨ��rt_thread_self��ȡ�߳���Ϣ*/

/*�ر�����*/

/*ģ��ϵ�*/
lbl_start_pwr_off:

	rt_kprintf( "\r\n%ld>gsm pwr off start", rt_tick_get( ) );
	GPIO_SetBits( GSM_PWR_PORT, GSM_PWR_PIN );
	GPIO_SetBits( GSM_TERMON_PORT, GSM_TERMON_PIN );
	rt_thread_delay( RT_TICK_PER_SECOND / 10 );
	GPIO_ResetBits( GSM_TERMON_PORT, GSM_TERMON_PIN );
	rt_thread_delay( RT_TICK_PER_SECOND );
	GPIO_SetBits( GSM_TERMON_PORT, GSM_TERMON_PIN );
	//SendATCmdWaitRespStr("AT^SMSO\r\n",RT_TICK_PER_SECOND,"OK",1);
	rt_kprintf( "\r\n%ld>gsm pwr on end", rt_tick_get( ) );

	res = WaitResp( "^SHUTDOWN", RT_TICK_PER_SECOND * 10 );
	if( res != RT_EOK )
	{
		rt_kprintf( "\r\n%d>re gsm pwron\r\n", rt_tick_get( ) );
		goto lbl_start_pwr_off;
	}

	rt_kprintf( "\r\n%ld>gsm pwr off end", rt_tick_get( ) );
}

/***********************************************************
* Function:		gsmrx_cb
* Description:	gsm�յ���Ϣ�Ĵ���
* Input:			char *s     ��Ϣ
    uint16_t len ����
* Output:
* Return:
* Others:
***********************************************************/
static void gsmrx_cb( uint8_t *pInfo, uint16_t len )
{
	int i, count;
	uint8_t tbl[24]={0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,0,0xa,0xb,0xc,0xd,0xe,0xf};
	char c, *pmsg;
	uint8_t *psrc, *pdst;
	int32_t infolen, link;

/*��������Ϣ��ֱ��֪ͨ�ϲ�����*/
	if( fgsm_rawdata_out )
	{
		rt_kprintf( "\r\ngsm>%s\r\n", pInfo );
	}

	psrc    = RT_NULL;
	psrc    = strstr( pInfo, "%IPDATA:" );
	if( psrc != RT_NULL )
	{
		/*����������Ϣ*/
		i = sscanf( psrc, "%%IPDATA:%d,%d,%s", &link, &infolen, pdst );
		if(i!=3) return;
		if(infolen<11) return;
		if(*pdst!='"') return;
		pmsg=pdst++;

		while(*pmsg!='"') {
			c=tbl[*pmsg-'0']<<4;
			pmsg++;
			c|=tbl[*pmsg-'0'];
			pmsg++;
			*pdst++=c;
			count++;
			if(count>=infolen) break;
		}
		gprs_rx( pdst, infolen );
	}
	else
	{
		/*Ҫ�ж��Ƿ���gsm�����߳��ڹ�����*/
		pmsg = rt_malloc( len );
		if( pmsg != RT_NULL )
		{
			memcpy( pmsg, pInfo, len );
			rt_mb_send( &mb_gsmrx, (rt_uint32_t)pmsg );
		}
		/*�ͷ��յ�һ����Ϣ���ź�,Ҫ���Ǵ���ʱ�����������*/
		return;
	}
}


/***********************************************************
* Function:
* Description:	��������
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
int gsm_send( uint8_t *pinfo, uint16_t len )
{
	int i;
/*���ҿ��õ�socket*/
	for(i=0; i<MAX_SOCKET_NUM; i++)
	{
		if (gsm_socket[i].connect_state==4)
		{

		}


	}
}

ALIGN( RT_ALIGN_SIZE )
static char thread_socket_stack[512];
struct rt_thread thread_socket;


/***********************************************************
* Function:
* Description: socket���Ӵ���
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static void rt_thread_entry_socket( void* parameter )
{
	int i;
	rt_err_t res;
	char dialstr[128];
/*��ѯgsm״̬,�Ƿ��ʼ���ڲ�tcpip*/
	if( ( gsm_state == GSM_AT ) && ( socket_state == SOCKET_TCPIP_INIT ) )
	{
		sprintf(dialstr,"AT+CGDCONT=1,\"IP\",\"%s\"\r\n",jt808_param.apn);
		res=SendATCmdWaitRespStr(dialstr,3*RT_TICK_PER_SECOND,"OK",1);
		if(res!=RT_EOK)
		{
			gsm_state=GSM_POWERON;
		}
		res=SendATCmdWaitRespStr("AT%ETCPIP\r\n",10*RT_TICK_PER_SECOND,"OK",1);
		if(res!=RT_EOK)
		{
			gsm_state=GSM_POWERON;
		}
		res=SendATCmdWaitRespStr("AT%IOMODE=1,2,1\r\n",5*RT_TICK_PER_SECOND,"OK",1);
		if(res!=RT_EOK)
		{
			gsm_state=GSM_POWERON;
		}
		socket_state=SOCKET_READY;
	}

/*��ѯ����״̬*/
	if( socket_state == SOCKET_READY )
	{
		for( i = 0; i < MAX_SOCKET_NUM; i++ )
		{
			if( gsm_socket[i].connect_state == 1 )
			{
			}
		}
	}

/*ȷ��dns���*/
	rt_thread_delay( RT_TICK_PER_SECOND / 10 );
}

ALIGN( RT_ALIGN_SIZE )
static char thread_gsm_stack[512];
struct rt_thread thread_gsm;


/***********************************************************
* Function:       rt_thread_entry_gsm
* Description:    ���մ�����״̬ת��
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
static void rt_thread_entry_gsm( void* parameter )
{
	rt_tick_t curr_ticks;
	rt_err_t res;
	unsigned char ch;

/*gsm��״̬�л�*/
	while( 1 )
	{
		curr_ticks = rt_tick_get( );
		switch( gsm_state )
		{
		case GSM_IDLE:
			break;
		case GSM_POWERON:         /*�ϵ�����У�����IO�Ĳ������ڴ˴�����α�֤ʱ��Ŀɿ�*/
			if( tid_gsm_subthread == RT_NULL )
			{
				tid_gsm_subthread = rt_thread_create( "pwron", gsm_poweron, (void*)1, 512, 25, 5 );
				rt_kprintf( "\r\ntid_gsm_subthread=%p\r\n", tid_gsm_subthread );
				if( tid_gsm_subthread != RT_NULL )
				{
					tid_gsm_subthread->cleanup = cleanup;
					rt_thread_startup( tid_gsm_subthread );
				}
			}
			break;
		case GSM_POWEROFF:         /*�ϵ������У�����IO�Ĳ������ڴ˴�����α�֤ʱ��Ŀɿ�*/
			if( tid_gsm_subthread == RT_NULL )
			{
				tid_gsm_subthread = rt_thread_create( "pwroff", gsm_poweroff, (void*)1, 512, 25, 5 );
				if( tid_gsm_subthread != RT_NULL )
				{
					rt_thread_startup( tid_gsm_subthread );
				}
			}
			break;
		case GSM_AT:
			break;
		}

/*���ճ�ʱ�ж�*/
		while( rt_ringbuffer_getchar( &rb_uart4_rx, &ch ) == 1 ) /*������ʱ����������*/
		{
			gsm_rx[gsm_rx_wr++] = ch;
			if( gsm_rx_wr == GSM_RX_SIZE )
			{
				gsm_rx_wr = 0;
			}
			gsm_rx[gsm_rx_wr] = 0;
		}
		if( rt_tick_get( ) - last_tick > RT_TICK_PER_SECOND / 10 )  //�ȴ�100ms,ʵ���Ͼ��Ǳ䳤����ʱ,���100ms������һ�����ݰ�
		{
			if( gsm_rx_wr )
			{
				gsmrx_cb( gsm_rx, gsm_rx_wr );                      /*������Ϣ�Ĵ�������*/
				gsm_rx_wr = 0;
			}
		}
		rt_thread_delay( RT_TICK_PER_SECOND / 50 );                 /*��ʱ20ms*/
	}
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void gsm_init( void )
{
	rt_thread_t tid;

/*��ʼ�����ڽ��ջ�����*/
	rt_ringbuffer_init( &rb_uart4_rx, uart4_rxbuf, UART4_RX_SIZE );

	rt_mb_init( &mb_gsmrx, "gsm_rx", &mb_gsmrx_pool, MB_GSMRX_POOL_SIZE / 4, RT_IPC_FLAG_FIFO );

	rt_thread_init( &thread_gsm,
	                "gsm",
	                rt_thread_entry_gsm,
	                RT_NULL,
	                &thread_gsm_stack[0],
	                sizeof( thread_gsm_stack ), 7, 5 );
	rt_thread_startup( &thread_gsm );

/*����socket�����߳�*/
	rt_thread_init( &thread_socket,
	                "socket",
	                rt_thread_entry_socket,
	                RT_NULL,
	                &thread_socket_stack[0],
	                sizeof( thread_socket_stack ), 8, 5 );
	rt_thread_startup( &thread_socket );

	dev_gsm.type    = RT_Device_Class_Char;
	dev_gsm.init    = m66_init;
	dev_gsm.open    = m66_open;
	dev_gsm.close   = m66_close;
	dev_gsm.read    = m66_read;
	dev_gsm.write   = m66_write;
	dev_gsm.control = m66_control;

	rt_device_register( &dev_gsm, "gsm", RT_DEVICE_FLAG_RDWR );
	rt_device_init( &dev_gsm );
}

#ifdef TEST_GSM


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
rt_err_t gsm_open( void )
{
	return m66_open( &dev_gsm, RT_DEVICE_OFLAG_RDWR );
}

FINSH_FUNCTION_EXPORT( gsm_open, open gsm );


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
rt_err_t gsm_status( void )
{
	char *st[] = {
		"����",
		"�ϵ����",
		"�ϵ������",
		"AT����",
		"PPP����״̬",
		"����״̬",
	};
	rt_kprintf( "gsm>%s\n", st[gsm_state] );
}

FINSH_FUNCTION_EXPORT( gsm_status, gsm status );


/***********************************************************
* Function:
* Description: �ͷ����ӣ�gsm�ϵ�
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
rt_err_t gsm_close( void )
{
	return m66_close( &dev_gsm );
}

FINSH_FUNCTION_EXPORT( gsm_close, close gsm );


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
rt_size_t gsm_write( char *sinfo )
{
	return m66_write( &dev_gsm, 0, sinfo, strlen( sinfo ) );
}

FINSH_FUNCTION_EXPORT( gsm_write, write gsm );

#if 0

/*�������ӵ�socket����*/
rt_err_t sock_config( uint8_t linkno, char* apn, char *connect_str )
{
	if( linkno >= GSM_MAX_SOCK_NUM )
	{
		return RT_ERROR;
	}
	if( gsm_socket[linkno].active != 0 )
	{
		return RT_ERROR;
	}
	gsm_socket[linkno].active = 0;
	strcpy( gsm_socket[linkno].apn, apn );
	strcpy( gsm_socket[linkno].conn_str, connect_str );
	return RT_EOK;
}

FINSH_FUNCTION_EXPORT( sock_config, < linkno > < apn > < connstr > );

/*����socket����*/
rt_err_t sock_control( uint8_t linkno, uint8_t action )
{
	if( linkno >= GSM_MAX_SOCK_NUM )
	{
		return RT_ERROR;
	}
	if( gsm_socket[linkno].active ^ action ) /*��״̬�仯*/
	{
		rt_kprintf( "sock>%d\n", __LINE__ );
		if( action == 1 )
		{
			if( tid_gsm_subthread == RT_NULL )
			{
				tid_gsm_subthread = rt_thread_create( "ppp", gsm_socket_open, (void*)&linkno, 512, 25, 5 );
				if( tid_gsm_subthread != RT_NULL )
				{
					tid_gsm_subthread->cleanup = cleanup;
					rt_thread_startup( tid_gsm_subthread );
					rt_kprintf( "sock>%d\n", __LINE__ );
					return RT_EOK;
				}
				rt_kprintf( "sock>%d\n", __LINE__ );
				return RT_ERROR;
			}
			rt_kprintf( "sock>%d\n", __LINE__ );
			return RT_ERROR;
		}
		else
		{
			if( tid_gsm_subthread == RT_NULL )
			{
				tid_gsm_subthread = rt_thread_create( "ppp", gsm_socket_close, (void*)linkno, 512, 25, 5 );
				if( tid_gsm_subthread != RT_NULL )
				{
					rt_thread_startup( tid_gsm_subthread );
					rt_kprintf( "sock>%d\n", __LINE__ );
					return RT_EOK;
				}
				rt_kprintf( "sock>%d\n", __LINE__ );
				return RT_ERROR;
			}
			rt_kprintf( "sock>%d\n", __LINE__ );
			return RT_ERROR;
		}
	}
	return RT_EOK;
}

FINSH_FUNCTION_EXPORT( sock_control, < linkno > < 0 | 1 > );

#endif

#endif
#endif

/************************************** The End Of File **************************************/