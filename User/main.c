/*
*********************************************************************************************************
*
*	ģ������ : ������ģ��
*	�ļ����� : main.c
*	��    �� : V1.1
*	˵    �� : 
*
*	�޸ļ�¼ :
*		�汾��   ����         ����        ˵��
*		V1.0    2018-12-12   Eric2013     1. CMSIS����汾 V5.4.0
*                                     2. HAL��汾 V1.3.0
*
*   V1.1    2019-04-01   suozhang     1. add FreeRTOS V10.20
*
*	Copyright (C), 2018-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			/* �ײ�Ӳ������ */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "croutine.h"
#include "semphr.h"
#include "event_groups.h"

/**
 * Log default configuration for EasyLogger.
 * NOTE: Must defined before including the <elog.h>
 */
#if !defined(LOG_TAG)
#define LOG_TAG                    "main_test_tag:"
#endif
#undef LOG_LVL
#if defined(XX_LOG_LVL)
    #define LOG_LVL                    XX_LOG_LVL
#endif

#include "elog.h"

#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "netif_port.h"

#include "tcp_client.h"
#include "app.h"

static void vTaskLED (void *pvParameters);
static void vTaskLwip(void *pvParameters);

 TaskHandle_t xHandleTaskLED  = NULL;
 TaskHandle_t xHandleTaskLwip = NULL;

struct  CONFIG  config={0xAA55, //	uint16_t vaildsign;
	1,//uint8_t baundrate;    
	1,//uint8_t addr; 
	0x000000000000145,//0x6275110032120001,//0x6275110032120003,//0x5955125011120002, 03 yec-test 101
	0, //uint8_t parity;		// =0 : n,8,1   =1: o,8,1  =2: e,8,1	 ���ݸ�ʽ
	{0.2f,0.2f,0.2f,1,1,1,1,1,1,1,1,1},
	0, //uint8_t DisplayMode;  // ��ʾģʽ��=0���̶���=1 ѭ��
	{TYPE_IEPE,TYPE_PT,TYPE_IEPE,TYPE_IEPE,TYPE_IEPE,TYPE_IEPE,TYPE_IEPE,TYPE_IEPE,TYPE_IEPE,1,1,1}, //uint8_t interface_type[12]; // ��������
	{UNIT_M_S2,UNIT_TEMP,UNIT_M_S2,UNIT_M_S2,UNIT_M_S2,UNIT_M_S2,1,1,1,1,1,1},//uint8_t unit[12];  // ��λ
	{250,250,250,50,1000,1000,1000,1000,10000,10000,10000,10000},//uint32_t scale[12]; // ת��ϵ��
	{0.0f,0.0f,0.0f,1250.f,1250.f,0,0,0,8192,8192,8192,8192},//uint32_t adjust[12]; // ����ϵ��
//	{0,1,2,~0,~0,~0,~0,~0,3,4,5,6},//uint16_t interface_addr[12]; // modbus ��ַ �ϴ�
	{100,100,100,100,100,100},//	float alarmgate[12]; // float����ֵ
	{1.004f,1.004f,1.004f,1,1,1},
	0,//uint8_t means	;// ��ֵ����
	1,//uint16_t means_times; // ��ֵ��������
	20000,//uint16_t freq;  // ����Ƶ�� Hz
	4096,//uint16_t avr_count;
	2, // uint8_t reflash; // ˢ��ʱ�� 
	~0, //	uint16_t din_addr;  //  ����������Ĵ�����ַ
	~0, // uint16_t dout_addr; //  ����������Ĵ�����ַ
	300, 30, // uint32_t force_gate,force_backlash,   // Ӧ��������ֵ�� �ز
	~0,~0, //	uint16_t max_addr0,max_addr1; ���ֵ��ַ
	300,
	300,
	300,
	0x4a,                          //PGA
	60,                          //��������
	0x0100,                         //�Ŵ���
	0x11,                      //  ���书������
	0x01,                    //AD����ʱ�䣬����������һ������
	8192,                     //AD����Ƶ��
	0x58B6A4C3,           //8��00�֣�aabb�ĸ�ʽ��aa����ʱ�䣬bb������ӣ���Ĭ��Ϊ0
	0,                //flash��ʼ��ַ
	PARAMETERMODE,
	0x1F,     //ѡ�ļ���ͨ������
	1, //DHCP
	"wifi-sensor",//"TP-LINK-SCZZB",//"yec-test",//"wifi-sensor",//"TP-LINK-sczzb2",//"hold-704",//"wifi-test1",//"yec-test",//"wifi-test",//"yec-test",//"zl_sensor",/////"yec-test",//"test3",//"qiangang2", //"qiangang1", //"qiangang1", /////
  "wifi-sensor",//"china-yec",//"",//"wifi-sensor",//"18051061462",//"wifi-test",//"zl_sensor",///"china-yec",//"",////"",//"zl_sensor",/"lft13852578307",//"",//"",//"123456789",//"china-yec.com",// //
  "192.168.99.3",// "192.168.0.112",//�������˵�IP��ַ  "192.168.0.18", //M
  "8712", //�˿ں�
  "192.168.99.45",  //LocalIP
  "192.168.99.1",  //LocalGATEWAY
  "255.255.255.0",	//LocalMASK
	1,
	1,
	1,
	0x1f,
	1,
	0,  //�Ƿ���������������
	0,
	0,
};


/*
*********************************************************************************************************
*	�� �� ��: main
*	����˵��: c�������
*	��    ��: ��
*	�� �� ֵ: �������(���账��)
*********************************************************************************************************
*/
int main(void)
{

	bsp_Init();		/* Ӳ����ʼ�� */
	
	/* initialize EasyLogger */
	if (elog_init() == ELOG_NO_ERR)
	{
			/* set enabled format */
			elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL & ~ELOG_FMT_P_INFO);
			elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_ALL );
			elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
			elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_TAG | ELOG_FMT_TIME);
			elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~(ELOG_FMT_FUNC | ELOG_FMT_P_INFO));
			elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~(ELOG_FMT_FUNC | ELOG_FMT_P_INFO));
		
			elog_set_text_color_enabled( true );
		
			elog_buf_enabled( false );
		
			/* start EasyLogger */
			elog_start();
	}
	
	xTaskCreate( vTaskLED, "vTaskLED", 512, NULL, 3, &xHandleTaskLED );
	xTaskCreate( vTaskLwip,"Lwip"     ,512, NULL, 2, &xHandleTaskLwip );
	
	/* �������ȣ���ʼִ������ */
	vTaskStartScheduler();
}


struct netif gnetif; /* network interface structure */

static void netif_config(void)
{
  ip_addr_t ipaddr;
  ip_addr_t netmask;
  ip_addr_t gw;

  IP_ADDR4(&ipaddr,192,168,0,11);
  IP_ADDR4(&netmask,255,255,255,0);
  IP_ADDR4(&gw,192,168,0,1);

  /* add the network interface */ 
  netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);
  
  /*  Registers the default network interface. */
  netif_set_default(&gnetif);
	
	/* Set the link callback function, this function is called on change of link status*/
  netif_set_link_callback(&gnetif, eth_link_callback);

}

/*
*********************************************************************************************************
*	�� �� ��: vTaskLwip
*	����˵��: ��ʼ�� ETH,MAC,DMA,GPIO,LWIP,�������߳����ڴ�����̫����Ϣ
*	��    ��: pvParameters ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
* �� �� ��: 2  
*********************************************************************************************************
*/
static void vTaskLwip(void *pvParameters)
{

  /* Create tcp_ip stack thread */
  tcpip_init(NULL, NULL);

  /* Initilaize the netif */
  netif_config();

	for(;;)
	{
		
		tcp_client_conn_server_task();
		
		vTaskDelay( 2000 / portTICK_PERIOD_MS );
	}
}

/*
*********************************************************************************************************
*	�� �� ��: vTaskLED
*	����˵��: KED��˸	
*	��    ��: pvParameters ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
* �� �� ��: 2  
*********************************************************************************************************
*/
static void vTaskLED(void *pvParameters)
{
	
	uint32_t ulNotifiedValue     = 0;
	uint32_t ledToggleIntervalMs = 1000;

	for(;;)
	{
		
		/*
			* ���� 0x00      ��ʾʹ��֪ͨǰ����������ֵ֪ͨλ��
			* ���� ULONG_MAX ��ʾ����xTaskNotifyWait()�˳�ǰ������ֵ֪ͨ����Ϊ0
			*/
	 if( pdPASS == xTaskNotifyWait( 0x00, 0xffffffffUL, &ulNotifiedValue, ledToggleIntervalMs ) )
	 {
		 if( ulNotifiedValue < 2000 )
			ledToggleIntervalMs  = ulNotifiedValue;
		 else
			 ledToggleIntervalMs = 1000 / portTICK_PERIOD_MS;
	 }

		bsp_LedToggle(1);

//		log_i( "SystemCoreClock:%u,system heap:%u.", SystemCoreClock,xPortGetFreeHeapSize() );

	}
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
