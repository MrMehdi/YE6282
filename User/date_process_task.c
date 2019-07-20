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


/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
volatile uint32_t wp[16];
volatile uint8_t jiaoyansum[16];
volatile uint16_t wave_packege_flag=0;
uint8_t WaveToSend[AD7606_ADCHS][1024];
volatile uint32_t currentSAMPLEblock=0;
void UpdateWave(float yy,uint8_t i) //????,??yy??????i???
{   
	 int16_t senddata=(int16_t)(yy*Parameter.ReciprocalofRange[i]);
		if(((config.DataToSendChannel>>i)&0x01)) 
		{
			WaveToSend[i][wp[i]+14]=senddata;//*floatdata;  //�޸�sendkk;//
			WaveToSend[i][wp[i]+15]=senddata>>8;
			jiaoyansum[i]+=(WaveToSend[i][wp[i]+14]+WaveToSend[i][wp[i]+15]);
			wp[i]=wp[i]+2;			
		}
	 if(wp[i]>(BOARDPOINTS-1)) {	

		  
		  uint16_t buflength=BOARDPOINTS+7+2+1;
			WaveToSend[i][0]=0x7e;//TELid;
			WaveToSend[i][1]=0x40;//TELid>>8;	
			WaveToSend[i][2]=(uint8_t)buflength;//TELid>>16;
			WaveToSend[i][3]=buflength>>8;// TELid>>24; //2,3????????482??
		  WaveToSend[i][4]=g_tRTC.Year;
		  WaveToSend[i][5]=g_tRTC.Year>>8;
		  WaveToSend[i][6]=g_tRTC.Mon;
		  WaveToSend[i][7]=g_tRTC.Day; //ʱ����32λ��ʾ
		  WaveToSend[i][8]=g_tRTC.Hour;
		  WaveToSend[i][9]=g_tRTC.Min;
		  WaveToSend[i][10]=g_tRTC.Sec; //ʱ����32λ��ʾ
		  WaveToSend[i][11]=(uint8_t)wave_packege_flag; //ʱ����32λ��ʾ
		  WaveToSend[i][12]=wave_packege_flag>>8; //ʱ����32λ��ʾ
			WaveToSend[i][13]=i;
		  for(uint32_t ii=1;ii<14;ii++)
		  jiaoyansum[i]+=WaveToSend[i][ii];  //adch�Ǵ�0��ʼ��
			WaveToSend[i][14+BOARDPOINTS]=jiaoyansum[i];  //2,3????????482??
		 	WaveToSend[i][15+BOARDPOINTS]=0x7e; 
		  WriteDataToTXDBUF(WaveToSend[i],BOARDPOINTS+16);
			
		
		  wave_packege_flag++;
		  jiaoyansum[i]=0;			
			wp[i]=0;
		}
	
}


void UpdateAccelerationData(float yyy,uint8_t i,uint32_t DataIndex)
{
	if(config.DataToBoardMode==PARAMETERMODE)  //������ֵģʽ�£������ڲɼ�
		{
		 emu_data[currentSAMPLEblock][i][DataIndex]=yyy;//yyy;
		}else if(config.DataToBoardMode==WAVEMODE)
		{  
		emu_data[currentSAMPLEblock][i][DataIndex]=yyy;
		 UpdateWave(yyy,i); 
		}	
	
}

int32_t halfref=0;
uint32_t ActualIndex=0;
int32_t AD_ZERO[AD7606_ADCHS],AD_ZEROlowpass[AD7606_ADCHS],AD_INTER[AD7606_ADCHS],lastdata[AD7606_ADCHS],filtercounter[AD7606_ADCHS];
extern  SemaphoreHandle_t SAMPLEDATA_ready;
extern __attribute__((section (".RAM_D1"))) int16_t Ad7606_Data[AD7606SAMPLEPOINTS*AD7606_ADCHS*2];
extern volatile uint32_t CurrentAD7606DataCounter;
extern  SemaphoreHandle_t AD7606_ready;
uint32_t sprase_counter[12],StoreDateIndex[12];
void emu_sprase_index()
{
	uint32_t i=0;
	for(i=0;i<AD7606_ADCHS;i++){  //ĳ���������˻�׼������Ϊ51200����Ϊ8ͨ������ͬ������������8��ͨ��ֻ��һ����׼������
			Parameter.sparse_index[i]=config.ADfrequence/config.channel_freq[i];
		}
}
void AD7606_TASK(void)
{
   
    uint32_t i,j;
		int16_t *p;  //????????16???
		int32_t y;
	  float volatile vy=0;//�ٶ��м����
	//	uint32_t si=0;
	  uint32_t FreFactor=1;
		volatile float yy=0;
	  halfref=32768;
		
  	for(i=0;i<AD7606_ADCHS;i++){
		  AD_ZERO[i]=(uint64_t)32768*16384;  // tao 10s 
			AD_ZEROlowpass[i]=0u;
			AD_INTER[i]=0;
			Parameter.vs[i]=0;
			lastdata[i]=0;
			wp[i]=0;
			jiaoyansum[i]=0;		
			filtercounter[i]=0;
			sprase_counter[i]=0;
			StoreDateIndex[i]=0;
			Parameter.ReciprocalofRange[i]=32768/config.floatrange[i];
			Parameter.sparse_index[i]=config.ADfrequence/config.channel_freq[i];
		}

		while(1)
		{		
			while(!(xSemaphoreTake(AD7606_ready, portMAX_DELAY) == pdTRUE))
			{};//
//			SCB_InvalidateDCache_by_Addr ((uint32_t *)&Ad7606_Data[CurrentAD7606DataCounter], AD7606SAMPLEPOINTS*AD7606_ADCHS/2);
	
			p=(int16_t *)&Ad7606_Data[CurrentAD7606DataCounter];
			
			for(j=0;j<AD7606SAMPLEPOINTS;j++){
				for(i=0;i<AD7606_ADCHS;i++){
					y=(int32_t)((*(int16_t*)p));//-(int32_t)((*(uint16_t*)(p+2)));  // 
					if(config.interface_type[i]==TYPE_IEPE){	
//								y=ActualIndex*0.1f;
						   //ֱ��ʹ���ϴεĵ�ֵͨ�ȿ���					
//						 AD_ZERO[i]=y+(((int64_t)AD_ZERO[i]*32767)>>15);
//						 y=y-(int32_t)(AD_ZERO[i]>>15); //AD_ZEROlowpass[i]

						 if(config.ADfrequence==51200)
						 {
/************************��׼������16384��ʵ�ʳ���16384����ͨ8K*************/			
//						  AD_ZEROlowpass[i]=((int64_t)y+lastdata[i])*49041-AD_ZEROlowpass[i]*32547; //6.2K
//							AD_ZEROlowpass[i]=AD_ZEROlowpass[i]>>16;			
//							lastdata[i]=y;
//							y=AD_ZEROlowpass[i];

						 }else if(config.ADfrequence==25600)
						 {
/************************��׼������16384��ʵ�ʳ���8192����ͨ2.5K**********/			
						
							AD_ZEROlowpass[i]=((int64_t)y+lastdata[i])*22451+AD_ZEROlowpass[i]*20706;
							AD_ZEROlowpass[i]=AD_ZEROlowpass[i]>>16;			
							lastdata[i]=y;
							y=AD_ZEROlowpass[i];
						 }else if(config.ADfrequence==12800)
						 {
/***********************��׼������16384��ʵ�ʳ���4096����ͨ1.25K*************/			  													
							AD_ZEROlowpass[i]=((int64_t)y+lastdata[i])*12870+AD_ZEROlowpass[i]*39795;
							AD_ZEROlowpass[i]=AD_ZEROlowpass[i]>>16;	
							lastdata[i]=y;
							y=AD_ZEROlowpass[i];
						 }else if(config.ADfrequence==6400)
						 {
/***********************��׼������16384��ʵ�ʳ���4096����ͨ1.25K*************/			  													
							AD_ZEROlowpass[i]=((int64_t)y+lastdata[i])*12870+AD_ZEROlowpass[i]*39795;
							AD_ZEROlowpass[i]=AD_ZEROlowpass[i]>>16;	
							lastdata[i]=y;
							y=AD_ZEROlowpass[i];
						 }else if(config.ADfrequence==3200)
						 {
/***********************��׼������16384��ʵ�ʳ���4096����ͨ1.25K*************/			  													
							AD_ZEROlowpass[i]=((int64_t)y+lastdata[i])*12870+AD_ZEROlowpass[i]*39795;
							AD_ZEROlowpass[i]=AD_ZEROlowpass[i]>>16;	
							lastdata[i]=y;
							y=AD_ZEROlowpass[i];
						 }
						
					 
						 /**************************��������************************/
			
						sprase_counter[i]++;
						if(sprase_counter[i]>=Parameter.sparse_index[i]) 
						{
						bsp_LedToggle(2);
						sprase_counter[i]=0;
							//������¼�������ݵ��±�
//						if(StoreDateIndex[i]>=config.channel_freq[i])
//							StoreDateIndex[i]=0;
//						y=y*0.30517578f;
					
						UpdateAccelerationData(y,i,StoreDateIndex[i]);
						StoreDateIndex[i]++;
						}
					}else if(config.interface_type[i]==TYPE_NONE)
					{
						halfref=y;
					}
				 
				
      	p++;
       
				}
       	ActualIndex++;   //ԭ����Ӧ�üӸ��ж����ģ��ر��ǵ�����ֵģʽ�£�д����±����Ϊ0ʱ����������һ������				

		
			
				if(ActualIndex>=config.ADfrequence){ //config.ADfrequence,���һ�����ݣ������˲�����
				
				RTC_ReadClock();	/* ��ʱ�ӣ������ g_tRTC */
				for(i=0;i<AD7606_ADCHS;i++){
					StoreDateIndex[i]=0;
					sprase_counter[i]=0; //ȫ����0
				}
				 ActualIndex=0;
         currentSAMPLEblock=(currentSAMPLEblock+1)%2;
				 xSemaphoreGive(SAMPLEDATA_ready);
				}
				}

	
			}
		
}
