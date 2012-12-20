#include "Menu_Include.h"
#include "Lcd.h"
#include <string.h>
#include <stdlib.h>



unsigned char licheng_sum[18]={"�����:000000 KM"};

static void show(void)
{
	unsigned long DisKm=0;

	/*licheng_sum[7]=DisKm%1000000/100000+0x30;
	licheng_sum[8]=DisKm%100000/10000+0x30;
	licheng_sum[9]=DisKm%10000/1000+0x30;
	licheng_sum[10]=DisKm%1000/100+0x30;
	licheng_sum[11]=DisKm%100/10+0x30;
	licheng_sum[12]=DisKm%10+0x30; 
	*/
    //ultoa(DisKm,licheng_sum,10);
	

	lcd_fill(0);
	lcd_text12(0, 3,(char *)Dis_date,20,LCD_MODE_SET);
	lcd_text12(0,18,(char *)licheng_sum,16,LCD_MODE_SET);
	lcd_update_all();
}


static void keypress(unsigned int key)
{
switch(KeyValue)
	{
	case KeyValueMenu:
		pMenuItem=&Menu_1_InforCheck;
		pMenuItem->show();
		CounterBack=0;
		break;
	case KeyValueOk:
		break;
	case KeyValueUP:
		break;
	case KeyValueDown:
		break;
	}
KeyValue=0;
}

static void timetick(unsigned int systick)
{
    CounterBack++;
	if(CounterBack!=MaxBankIdleTime)  // ������ʾ��ʱʱ�˻ص���������
		return;
	
	pMenuItem=&Menu_1_Idle;
	pMenuItem->show();
	
	CounterBack=0;

}

ALIGN(RT_ALIGN_SIZE)
MENUITEM	Menu_2_3_6_Mileage=
{
	"�����Ϣ�鿴",
	12,
	&show,
	&keypress,
	&timetick,
	(void*)0
};