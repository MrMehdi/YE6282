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

MDMA_HandleTypeDef MDMA_Handle;

uint16_t volatile TransferCompleteDetected=0;
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
	MDMA_Handle.Init.SourceInc            = MDMA_SRC_INC_DOUBLEWORD;         /* Դ��ַ������˫�֣���8�ֽ� */
	MDMA_Handle.Init.DestinationInc       = MDMA_DEST_INC_DOUBLEWORD;        /* Ŀ�ĵ�ַ������˫�֣���8�ֽ� */
	MDMA_Handle.Init.SourceDataSize       = MDMA_SRC_DATASIZE_DOUBLEWORD;    /* Դ��ַ���ݿ��˫�֣���8�ֽ� */
	MDMA_Handle.Init.DestDataSize         = MDMA_DEST_DATASIZE_DOUBLEWORD;   /* Ŀ�ĵ�ַ���ݿ��˫�֣���8�ֽ� */
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

			 

	
}

uint8_t WriteDataToTXDBUF(uint8_t * source,uint16_t length)
{ 
	
	if(xSemaphoreTake(WRITE_ready, portMAX_DELAY) != pdTRUE);//  ���ź�����ûë��
	__disable_irq();	   //��Ӧ�����������жϣ�������
	if(isTxdBufFull()) 
	{
	xSemaphoreGive(WRITE_ready);//�ͷ��ź������
	__enable_irq();

	return 0;
	} //�������˾Ͳ���
	__enable_irq();

		/* AXI SRAM��SDRAM��64KB���ݴ������ ***********************************************/
	TransferCompleteDetected = 0;
	HAL_MDMA_Start_IT(&MDMA_Handle,
																(uint32_t)source,
																(uint32_t)0xC0000000,
																length/4,
																1);

	while(TransferCompleteDetected == 0) {}
	Increase(HeadIndex);  
	xSemaphoreGive(WRITE_ready);//�ͷ��ź������
	return 1;
}
