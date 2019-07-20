    /*
    *********************************************************************************************************
    *        �� �� ��: MDMA_SpeedTest
    *        ����˵��: MDMA���ܲ���
    *        ��    ��: ��
    *        �� �� ֵ: ��
    *********************************************************************************************************
    */
		
#include "bsp.h"			/* �ײ�Ӳ������ */
#include "app.h"			/* �ײ�Ӳ������ */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "croutine.h"
#include "semphr.h"
#include "event_groups.h"


#include "lwip/sys.h"
#include "lwip/api.h"

#include "lwip/tcp.h"
#include "lwip/ip.h"

MDMA_HandleTypeDef MDMA_Handle;

int16_t emu_data[2][8][51200] __attribute__((at(0xC0000000)));  //˫���棬8ͨ��51200������ 0x19 0000
uint8_t TXD_BUFFER_NET[5000][2000] __attribute__((at(0xC0200000))); //0xa0 0000
uint32_t TXD_BUFFER_NET_Length[5000];
uint16_t volatile TransferCompleteDetected=0;
uint16_t testtt[1024]={0x1111,0x2222,0x3333,0x5445,0x6666,0x1111};
volatile uint32_t TxdBufHeadIndex;
volatile uint32_t TxdBufTailIndex;

void MDMA_IRQHandler(void)
{
	HAL_MDMA_IRQHandler(&MDMA_Handle);
}
static void MDMA_TransferCompleteCallback(MDMA_HandleTypeDef *hmdma)
{
	TransferCompleteDetected = 1;
}
void MDMA_init(void)
{
				/* MDMA���� **********************************************************************/
	__HAL_RCC_MDMA_CLK_ENABLE();  

	MDMA_Handle.Instance = MDMA_Channel0;  

	MDMA_Handle.Init.Request              = MDMA_REQUEST_SW;         /* ������� */
	MDMA_Handle.Init.TransferTriggerMode  = MDMA_BLOCK_TRANSFER;     /* �鴫�� */
	MDMA_Handle.Init.Priority             = MDMA_PRIORITY_HIGH;      /* ���ȼ���*/
	MDMA_Handle.Init.Endianness           = MDMA_LITTLE_ENDIANNESS_PRESERVE; /* С�� */
	MDMA_Handle.Init.SourceInc            = MDMA_SRC_INC_BYTE;         /* Դ��ַ������˫�֣���8�ֽ� */
	MDMA_Handle.Init.DestinationInc       = MDMA_DEST_INC_BYTE;        /* Ŀ�ĵ�ַ������˫�֣���8�ֽ� */
	MDMA_Handle.Init.SourceDataSize       = MDMA_SRC_DATASIZE_BYTE;    /* Դ��ַ���ݿ��˫�֣���8�ֽ� */
	MDMA_Handle.Init.DestDataSize         = MDMA_DEST_DATASIZE_BYTE;   /* Ŀ�ĵ�ַ���ݿ��˫�֣���8�ֽ� */
	MDMA_Handle.Init.DataAlignment        = MDMA_DATAALIGN_PACKENABLE;       /* С�ˣ��Ҷ��� */                    
	MDMA_Handle.Init.SourceBurst          = MDMA_SOURCE_BURST_128BEATS;      /* Դ����ͻ�����䣬128�� */
	MDMA_Handle.Init.DestBurst            = MDMA_DEST_BURST_128BEATS;        /* Դ����ͻ�����䣬128�� */
	
	MDMA_Handle.Init.BufferTransferLength = 128;    /* ÿ�δ���128���ֽ� */

	MDMA_Handle.Init.SourceBlockAddressOffset  = 0; /* ����block���䣬��ַƫ��0 */
	MDMA_Handle.Init.DestBlockAddressOffset    = 0; /* ����block���䣬��ַƫ��0 */

	/* ��ʼ��MDMA */
	if(HAL_MDMA_Init(&MDMA_Handle) != HAL_OK)
	{
					 Error_Handler(__FILE__, __LINE__);
	}
	
	/* ���ô�����ɻص����жϼ������ȼ����� */
	HAL_MDMA_RegisterCallback(&MDMA_Handle, HAL_MDMA_XFER_CPLT_CB_ID, MDMA_TransferCompleteCallback);
	HAL_NVIC_SetPriority(MDMA_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(MDMA_IRQn);  

	HAL_MDMA_Start_IT(&MDMA_Handle,
																(uint32_t)TXD_BUFFER_NET[1],
																(uint32_t)TXD_BUFFER_NET[0],
																10,
																1);  //Ҫ�ȿմ�һ�Σ���Ȼ��һ������0�����		 

	
}

extern SemaphoreHandle_t WRITE_ready;

uint8_t WriteDataToTXDBUF(uint8_t * source,uint32_t length)
{ 
	uint32_t i=0;
	bsp_LedToggle(1);
	if(xSemaphoreTake(WRITE_ready, portMAX_DELAY) != pdTRUE);//  ���ź�����ûë��
	__disable_irq();	   //��Ӧ�����������жϣ�������
	if(isTxdBufFull()) 
	{
	xSemaphoreGive(WRITE_ready);//�ͷ��ź������
	__enable_irq();

	return 0;
	} //�������˾Ͳ���
	__enable_irq();
	
	for(i=0;i<length;i++)
		TXD_BUFFER_NET[TxdBufHeadIndex][i]=source[i];
		/* AXI SRAM��SDRAM��64KB���ݴ������ ***********************************************/
//	TransferCompleteDetected = 0;
//	HAL_MDMA_Start_IT(&MDMA_Handle,
//																(uint32_t)source,
//																(uint32_t)TXD_BUFFER_NET[TxdBufHeadIndex],
//																length,
//																1);
//	
//	while (TransferCompleteDetected == 0) 
//  {
//    /* wait until MDMA transfer complete or transfer error */
//  }

	TXD_BUFFER_NET_Length[TxdBufHeadIndex]=length;
	Increase(TxdBufHeadIndex);  
	xSemaphoreGive(WRITE_ready);//�ͷ��ź������
	return 1;
}

extern struct netconn *tcp_client_server_conn;
void send_buffer_task( void )
{
	MDMA_init();
	for(uint16_t i=0;i<100;i++)
	{
//		WriteDataToTXDBUF(testtt,1000);
	}
//	vTaskDelay(20000);
	for(;;)
	{
		if(!isTxdBufEmpty()) //
		{
			if( tcp_client_server_conn )
			{
				SCB_InvalidateDCache_by_Addr ((uint32_t *)TXD_BUFFER_NET[TxdBufTailIndex], TXD_BUFFER_NET_Length[TxdBufTailIndex]);
				if( netconn_write( tcp_client_server_conn, TXD_BUFFER_NET[TxdBufTailIndex], TXD_BUFFER_NET_Length[TxdBufTailIndex], NETCONN_COPY)==ERR_OK)
				{
					IncreaseNetReceiveBuf(TxdBufTailIndex);
				}
				else
				{
					
				}
			}
		
		} else
		{
		vTaskDelay(2);

		}
	}
}
