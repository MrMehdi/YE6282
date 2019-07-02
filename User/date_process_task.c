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
#include "app.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "croutine.h"
#include "semphr.h"
#include "event_groups.h"  

int16_t emu_data[2][8][51200] __attribute__((at(0xC0000000)));  //˫���棬8ͨ��51200������ 0x19 0000
uint16_t TXD_BUFFER_NET[5000][1000] __attribute__((at(0xC0200000))); //0xa0 0000
/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
void AD7606_TASK(void *pvParameters)
{
   
    uint32_t i,j;
		int16_t *p;  //????????16???
		int32_t y;
	  float volatile vy=0;//�ٶ��м����
	//	uint32_t si=0;
	  uint32_t StoreDateIndex=0;
	  uint32_t FreFactor=1;
		volatile float yy=0;
	  halfref=32768;
		
  	for(i=0;i<ADCHS;i++){
		  AD_ZERO[i]=(uint64_t)32768*16384;  // tao 10s 
			AD_ZEROlowpass[i]=0u;
			AD_INTER[i]=0;
			Parameter.vs[i]=0;
			lastdata[i]=0;
			filtercounter[i]=0;
		}

		while(1)
		{		
			while(!(xSemaphoreTake(AD7606_ready, portMAX_DELAY) == pdTRUE))
			{};//
			p=(int16_t *)&Ad7606_Data[CurrentAD7606DataCounter];
			
			for(j=0;j<AD7606SAMPLEPOINTS;j++){
//				if((ActualIndex==0)&&(!isCollectingOverInParameterMode()))
//						EnableProcessInParameterMode(); //���������0,�ٶȻ��ֵ�
				for(i=0;i<AD7606_ADCHS;i++){
					y=(int32_t)((*(int16_t*)p));//-(int32_t)((*(uint16_t*)(p+2)));  // 
					if(config.interface_type[i]==TYPE_IEPE){	
		
						   //ֱ��ʹ���ϴεĵ�ֵͨ�ȿ���					
						 AD_ZERO[i]=y+(((int64_t)AD_ZERO[i]*332767)>>15);
						 y=y-(int32_t)(AD_ZERO[i]>>15); //AD_ZEROlowpass[i]

						 if(config.ADfrequence==51200)
						 {
							 FreFactor=0;
/************************��׼������16384��ʵ�ʳ���16384����ͨ8K*************/			
						 // AD_ZEROlowpass[i]=((int64_t)y+lastdata[i])*38505-AD_ZEROlowpass[i]*11475; //5K
							// AD_ZEROlowpass[i]=((int64_t)y+lastdata[i])*45292-AD_ZEROlowpass[i]*25049; //6K
							//  AD_ZEROlowpass[i]=((int64_t)y+lastdata[i])*46754-AD_ZEROlowpass[i]*27970; //6.2K
						  AD_ZEROlowpass[i]=((int64_t)y+lastdata[i])*49041-AD_ZEROlowpass[i]*32547; //6.2K
							AD_ZEROlowpass[i]=AD_ZEROlowpass[i]>>16;			
							lastdata[i]=y;
							y=AD_ZEROlowpass[i];

						 }else if(config.ADfrequence==8192)
						 {
							 FreFactor=1;
/************************��׼������16384��ʵ�ʳ���8192����ͨ2.5K**********/			
						
							AD_ZEROlowpass[i]=((int64_t)y+lastdata[i])*22451+AD_ZEROlowpass[i]*20706;
							AD_ZEROlowpass[i]=AD_ZEROlowpass[i]>>16;			
							lastdata[i]=y;
							y=AD_ZEROlowpass[i];
						 }else if(config.ADfrequence==4096)
						 {
							 FreFactor=2;
/***********************��׼������16384��ʵ�ʳ���4096����ͨ1.25K*************/			  													
							AD_ZEROlowpass[i]=((int64_t)y+lastdata[i])*12870+AD_ZEROlowpass[i]*39795;
							AD_ZEROlowpass[i]=AD_ZEROlowpass[i]>>16;	
							lastdata[i]=y;
							y=AD_ZEROlowpass[i];
						 }
						
					 
						 /**************************��������************************/
			
						filtercounter[i]++;
						if((filtercounter[i]*config.ADfrequence)>=16384) 
						{
						filtercounter[i]=0;		
						StoreDateIndex=ActualIndex>>FreFactor;
						UpdateAccelerationData(y,i,StoreDateIndex);
						}
					}else if(config.interface_type[i]==TYPE_NONE)
					{
						halfref=y;
					}
				 
				
      	p++;
       
				}
       	ActualIndex++;   //ԭ����Ӧ�üӸ��ж����ģ��ر��ǵ�����ֵģʽ�£�д����±����Ϊ0ʱ����������һ������				

		
			
				if(ActualIndex>51200){ //config.ADfrequence,���һ�����ݣ������˲�����
						
				 GPIO_PortToggle(GPIOB, 1u << 0);  //�͹��ļǵ�ȡ��	
				 ActualIndex=0;
         currentSAMPLEblock=0;


				 xSemaphoreGive(SAMPLEDATA_ready);
				}
				}

	
			}
		
}
