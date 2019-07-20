#ifndef _APP_H_
#define _APP_H_
/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

#define      RX_BUFFER_SIZE                              1024
#define      TX_BUFFER_SIZE                              1024

//extern uint8_t TXDBUF[TxdBufLine][BOARDPOINTS+20];
extern volatile uint32_t TxdBufHeadIndex;
extern volatile uint32_t TxdBufTailIndex;

extern void DelayMS ( uint16_t t);
extern volatile uint16_t RxdBufHeadIndex;
extern volatile uint16_t RxdBufTailIndex;


extern uint8_t CmdBuf[TX_BUFFER_SIZE];
extern uint16_t CmdBufLength;

#define AD7606_ADCHS 8
#define AD7606SAMPLEPOINTS 200
#define AD7606_SAMPLE_COUNT AD7606_ADCHS*2*AD7606SAMPLEPOINTS

#define BOARDPOINTS AD7606SAMPLEPOINTS*sizeof(int16_t)*2 //(4*ADCHS*SAMPLEPOINTS)
#define PERIODBOARDPOINTS AD7606SAMPLEPOINTS*sizeof(int16_t) //(4*ADCHS*SAMPLEPOINTS)
#define ADC16_ByDMA_SAMPLE_COUNT (2*ADCHS*SAMPLEPOINTS) /* The ADC16 sample count. */


/*���ڷ������������*/
#define TxdBufLine  256
#define Increase(x)   {x=(x+1)%TxdBufLine;}
#define isTxdBufFull() ((TxdBufHeadIndex+1)%TxdBufLine==TxdBufTailIndex)
#define isTxdBufEmpty() (TxdBufHeadIndex==TxdBufTailIndex)

extern uint16_t TXDBUFLength[TxdBufLine];

/*���ڽ������������*/
extern uint32_t NetReceiveBufHeadIndex;
extern uint32_t NetReceiveBufTailIndex;
#define NetReceiveBufSize  2048
#define IncreaseNetReceiveBuf(x)   {x=(x+1)%NetReceiveBufSize;}
#define isNetReceiveBufFull() ((NetReceiveBufHeadIndex+1)%NetReceiveBufSize==NetReceiveBufTailIndex)
#define isNetReceiveBufEmpty() (NetReceiveBufHeadIndex==NetReceiveBufTailIndex)



extern float emu_data[2][8][51200] __attribute__((at(0xC0000000)));  //˫���棬8ͨ��51200������ 0x19 0000
extern uint8_t TXD_BUFFER_NET[5000][2000] __attribute__((at(0xC0400000))); //0xa0 0000

extern uint8_t WriteDataToTXDBUF(uint8_t * source,uint32_t length);
extern void AD7606_StartRecord(uint32_t _ulFreq);
extern void AD7606_StopRecord(void);

#define RxdBufLine  256
#define EMUPONITS  16390*2  //���ȡ2S������

#define IncreaseRxdBufNum(x)   {x=(x+1)%RxdBufLine;}
#define isRxdBufFull() ((RxdBufHeadIndex+1)%RxdBufLine==RxdBufTailIndex)
#define isRxdBufEmpty() (RxdBufHeadIndex==RxdBufTailIndex)
//������
#define COMMAND_STOP 0x00
#define COMMAND_START 0x01
#define COMMAND_ID 0x02
#define COMMAND_CHANNELKIND 0x03
#define COMMAND_REPLYIP 0x04
#define COMMAND_SETIP 0x05  //����IP��ַ
#define COMMAND_REPLY_RATE 0x06   //���ò�������
#define COMMAND_SAMPLE_RATE 0x07   //���ò�������
#define COMMAND_REPLY_SCALE 0x08
#define COMMAND_SET_SCALE 0x09
#define COMMAND_ADJUST_TEMP 0x69
#define COMMAND_ADJUST_ADC 0x68
#define COMMAND_SET_SNNUMBER 0x88

#define COMMAND_RECEIVE_BEACON 0x11
#define COMMAND_RECEIVE_ACTIVE_BEACON 0x12
#define COMMAND_SET_FACTORY_PARAMETER 0x13

#define COMMAND_REQUIRE_PERIODWAVE 0x30 //����һ�ε��β���
#define COMMAND_SET_CHANNEL_CONDITION 0x0a  //����ͨ����
#define COMMAND_SET_RUNMODE 0x0B  //����ģʽ������ֵģʽ�����ǲ���ģʽ�����߱���ģʽɶ��
#define COMMAND_SET_TIME 0x50  //����ʱ��
#define COMMAND_SET_AP 0x51  //����AP��ַ
#define COMMAND_SET_TCPSERVE 0x52  //����server�˵ĵ�ַ
#define COMMAND_REPLYAP 0x53
#define COMMAND_REPLYTCPSERVE 0x54
#define COMMAND_APPLYNETSET 0x55
#define COMMAND_REPLY_RUNMODE 0x56  //����ģʽ������ֵģʽ�����ǲ���ģʽ�����߱���ģʽɶ��
#define COMMAND_REPLY_CHANNEL_CONDITION 0x57  //����ͨ����

#define COMMAND_REPLY_SW_VERSION 0x80  //�汾��
#define COMMAND_SET_SW_VERSION 0x81  //�汾��
#define COMMAND_IAP_DATA 0x82  //IAP���ݰ�
#define COMMAND_IAP_STATUE 0x83  //IAP״̬��
#define COMMAND_RESET_SYSTEM 0x84  //�������

#define IAP_ADDRESS 0x00100000





typedef struct  CONFIG				 // ������Ϣ
{
	uint16_t vaildsign;

	uint8_t baundrate;    /* =0:600    =1:1200 		=2:2400 		=3:4800 		=4:9600 */
	uint8_t addr; 
	uint64_t SNnumber; 
	uint8_t parity;		// =0 : n,8,1   =1: o,8,1  =2: e,8,1	 ���ݸ�ʽ
	float floatscale[12];
	//float floatrange[12];
	uint8_t DisplayMode;  // ��ʾģʽ��=0���̶���=1 ѭ��
	uint8_t interface_type[12]; // ��������
	uint8_t unit[12];  // ��λ
	float floatrange[12]; // ת��ϵ��
	float floatadjust[12]; // ����ֵ
	//uint16_t interface_addr[12]; // modbus ��ַ �ϴ�
	float alarmgate[6]; // ����ֵ
	float floatadc[6]; // ����ֵ
	uint8_t means	;// ��ֵ����
	uint16_t means_times; // ��ֵ��������
	uint16_t freq;  // ����Ƶ�� Hz
	uint16_t avr_count;
	uint8_t reflash; // ˢ��ʱ�� 
	uint16_t din_addr;  //  ����������Ĵ�����ַ
	uint16_t dout_addr; //  ����������Ĵ�����ַ
	uint32_t force_gate,force_backlash;  // Ӧ��������ֵ�� �ز
	uint16_t max_addr0,max_addr1;        //
	uint16_t vlpsseconds;            //  
	uint16_t vlprseconds;           //
	uint16_t runseconds;           //
	uint16_t pga;                 //
	uint16_t workcycleseconds;   //  ��������
	uint16_t fangda;            //  �Ŵ���
	uint16_t boardset;         // ���书������
	uint16_t ADtime;          //AD���β���ʱ��
	uint16_t ADfrequence;    //AD����Ƶ��
	  
	uint64_t alarmminutetime;  //��ʼ����ʱ��
	uint32_t FLASH_WRADDR;    //flash��ʼ��ַ
	uint8_t  DataToBoardMode; 
	uint16_t  DataToSendChannel;
	uint8_t  DHCP;
	char APssid[20];
	char APpassword[20];
	char TcpServer_IP[20];
	char TcpServer_Port[10];
	char LocalIP[20];
	char LocalGATEWAY[20];
	char LocalMASK[20];
	uint32_t PeriodTransimissonCounter;
	uint8_t PeriodTransimissonStatus;
	uint8_t ParameterTransimissonStatus;
	uint8_t RequirePeriodChannel; //������Щ���β��ε�ͨ��
	uint8_t RESETTIME;
	uint8_t Enable_active_beacon;
	uint8_t Iap_flag;
	uint32_t Iap_datalength;
	uint32_t channel_freq[12];
}CONFIG;

extern  struct CONFIG  config;  //������Ϣ



typedef enum {No_OS_Smapling = 0x000, OS_2_Smapling, OS_4_Smapling, OS_8_Smapling, OS_16_Smapling, OS_32_Smapling, OS_64_Smapling, }  Average_level;


typedef enum {NoBrainTransmission=0,BrainTransmission=1,} Esp32TransmissionMode;
typedef enum {EnableDHCP=1,DisableDHCP=0,} DHCPmode;
typedef enum {PARAMETERMODE=1,WAVEMODE=2,FFTWAVEMODE=3,FFTPARAMETERMODE=4,IDLEMODE=5,LITEWAVEMODE=6} DataToBoard_TYPE;
/* TYPE_HBR ȫ�� */
typedef enum {TYPE_NONE=0,TYPE_MA=1,TYPE_V=2, TYPE_IEPE=3,TYPE_PT=4,TYPE_HBR=5,TYPE_AC_V=6,} INTERFACE_TYPE;
typedef enum {UNIT_NONE=0,UNIT_V=1,UNIT_A=2,UNIT_KV=3,UNIT_TEMP=4,UNIT_M=5,UNIT_M_S=6,UNIT_M_S2=7,UNIT_G=8,UNIT_MA=9,UNIT_DB=10,UNIT_MM_S=11,} INTERFACE_UNIT;
typedef enum {MEANS_NONE=0,
							MEANS_MEANS, // ��ֵ
							MEANS_RMS, // ������ֵ
							MEANS_P,  // ��ֵ
							MEANS_PP, // ���ֵ
} MEANS_TYPE;  
typedef enum {BAUND2400=0,BAUND4800,BAUND9600,BAUND19200,BAUND38400,} BAUND_RATE;
//typedef enum {BAM4E33=0,BAM4E31=1, BAM4I33=2,BAM4I31=3,BAM4U33=4,BAM4U31=5,BAM4H33=6,BAM4H31=7,BAM4P33=8,BAM4P31=9,BAM4Q33=10,BAM4Q31=11}MODEL ;
extern uint8_t 	brightness; 
extern	uint8_t FirstBlood; //����¶��ϴ���ͻ
typedef  struct PARAMETER				 // ���в���
{
	uint16_t vaildsign;
	int32_t int_v[12];
	int32_t int_av[12];
	int64_t s[12]; // �ۻ�
	int64_t as[12]; // �ۻ�
	uint32_t sparse_index[12];
	float v[12];
	float fv[12];
	float vs[12];  //�ٶ��ۼӺͣ�������
	float av[12];
	float adate;                //���ٶ�
	float vdate;                 //�ٶ�
	float xdate;                  //λ��
	float pdate;                   //�¶�
	float scale_v[12];
	int32_t gate[12];
	int32_t backlash[12];
	uint8_t alarm[12]; // ������־
	uint16_t din;  // ��8λ��Ӧ 8��ͨ������
	uint16_t dout; // ��4λ��Ӧ 4�����ͨ�� 
	uint32_t status;
	int32_t ch1_int_max,ch2_int_max;
	int32_t ch1_int_max1,ch2_int_max2;
	float ch1_max,ch2_max;
	int32_t vibrate_gate1,vibrate_gate2,vibrate_backlash1,vibrate_backlash2;
	int32_t force_gate,force_backlash;
	int16_t selfpgaallow;
	uint16_t daytime;
	uint16_t minutetime;
	uint16_t selffangda;
  uint16_t alarmminutetime;
	uint8_t Esp32TransmissionMode;
	uint32_t sleeptime;
	uint8_t wakeupsourec;
	float Abs_average[12];
	float ReciprocalofADfrequence;
	float Inter_Skew[12]; //���
	float InterIIMarginIndex[12];//ԣ��ָ���м����
	float InterMAX[12];
	float InterMIN[12];
	float S_sum[12];
	float SS_sum[12];
	float SSS_sum[12];
	float SSSS_sum[12];
	float Abs_S_average[12];
	float PeakValue[12];  //���ֵ
	float EffectiveValue[12]; //��Чֵ
	float Skew[12]; //���
	float MaxValue[12]; //��ֵ
	float Kurtosis[12]; //�Ͷ�
	float Mean[12]; //��ֵ
	float WaveformIndex[12]; //����ָ��
	float PeakIndex[12];//��ֵָ��
	float PulseIndex[12];//����ָ��
	float MarginIndex[12];//ԣ��ָ��
	float KurtosisIndex[12];//�Ͷ�ָ��
	float Inter_MarginIndex[12]; //�Ͷ�
	float S_average[12]; //ƽ��ֵ��ֵ
	float average[12]; //ƽ��ֵ
	float ReciprocalofEMUnumber; //����Ƶ�ʵĵ�������������
	float ReciprocalofRange[12];//���̵ĵ���
	//
	float F_sum[12];     //s(k)֮��
	float FS_sum[12];   //f(k)s(k)�ĺ�
	float FFS_sum[12];  //F(K)F(K)S(K)
	float FFFFS_sum[12];  //F(K)F(K)S(K)
	float F2_sum[12];     //feature2���м��
	float F3_sum[12];     //feature3���м��
	float F4_sum[12];     //feature4���м��
	float F6_sum[12];     //feature6���м��
	float F11_sum[12];     //feature11���м��
	float F12_sum[12];     //feature12���м��
	float F13_sum[12];     //feature12���м��
	float F_feature1[12];
	float F_feature2[12];
	float F_feature3[12];
	float F_feature4[12];
	float F_feature5[12];
	float F_feature6[12];
	float F_feature7[12];
	float F_feature8[12];
	float F_feature9[12];
	float F_feature10[12];
	float F_feature11[12];
	float F_feature12[12];
	float F_feature13[12];
	uint32_t IapDataLength;
}PARAMETER;

extern struct PARAMETER Parameter;
typedef enum {PRE_TX=0,IN_TX=1,} TXcondition;
//typedef enum {DISABLE = 0, ENABLE = !DISABLE} FunctionalState;
//typedef enum {VLLS = 0, MANUALRESET = !DISABLE} WakeupSourec;
#define IS_FUNCTIONAL_STATE(STATE) (((STATE) == DISABLE) || ((STATE) == ENABLE))

				
struct ADJUSTDATA					 // У׼����
{	uint16_t vaildsign;
	uint32_t  UaSub,UbSub,UcSub;
	uint32_t IaSub,IbSub,IcSub;
	int32_t PhaseA,PhaseB,PhaseC;
	uint32_t  CVSub,OVSub;
};

extern  struct PARAMETER Parameter;
#define LPTMR_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_LpoClk)

#define isReceiveActiveBeaconMessage() ((Parameter.status&0x01)!=0)
#define ReceiveActiveBeaconMessage() {Parameter.status|=0x01;}
#define DeleteActiveBeaconMessage() {Parameter.status&=~0x01;}

#define isFactoryLink() ((Parameter.status&0x02)!=0)
#define setFactoryLink() {Parameter.status|=0x02;}
#define clrFactoryLink() {Parameter.status&=~0x02;}

#define isAbleTempTransmission() ((Parameter.status&0x04)!=0)
#define EnableTempTransmission() {Parameter.status|=0x04;}
#define DisableTempTransmission() {Parameter.status&=~0x04;}

#define isAblePeroidWaveTransmission() ((Parameter.status&0x08)!=0)
#define EnablePeroidWaveTransmission() {Parameter.status|=0x08;}
#define DisablePeroidWaveTransmission() {Parameter.status&=~0x08;}


#define isAbleLiteWaveTransmission() ((Parameter.status&0x10)!=0)
#define EnableLiteWaveTransmission() {Parameter.status|=0x10;}
#define DisableLiteWaveTransmission() {Parameter.status&=~0x10;}

#define isAblePeroidWaveAutoTransmission() ((Parameter.status&0x20)!=0)
#define EnablePeroidWaveAutoTransmission() {Parameter.status|=0x20;}
#define DisablePeroidWaveAutoTransmission() {Parameter.status&=~0x20;}

#define isCollectingOverInParameterMode() ((Parameter.status&0x40)!=0)  //���ڲɼ������ϴ�����
#define overCollectingInParameterMode() {Parameter.status|=0x40;}
#define BeginCollectingInParameterMode() {Parameter.status&=~0x40;}

#define isReceiveBeaconMessage() ((Parameter.status&0x80)!=0)  //���ڲɼ������ϴ�����
#define ReceiveBeaconMessage() {Parameter.status|=0x80;}
#define DeleteBeaconMessage() {Parameter.status&=~0x80;}

#define IsableProcessInParameterMode() ((Parameter.status&0x0100)!=0)  //ʹ������ֵ�µ����ݴ���
#define EnableProcessInParameterMode() {Parameter.status|=0x0100;}
#define DisableProcessInParameterMode() {Parameter.status&=~0x0100;}

#define IsNeedRestartCollect() ((Parameter.status&0x0200)!=0)  //ʹ������ֵ�µ����ݴ���
#define NeedRestartCollect() {Parameter.status|=0x0200;}
#define NotNeedRestartCollect() {Parameter.status&=~0x0200;}

#define EnableEsp32Power()  {GPIO_PortSet(GPIOC, 1u << 0);}  //�Ƿ��esp32����
#define DisableEsp32Power() {GPIO_PortClear(GPIOC, 1u << 0);}

#define EnableAV3V3Power()  {GPIO_PortSet(GPIOC, 1u << 2);}  //�Ƿ񹩵�ģ�ⲿ��
#define DisableAV3V3Power() {GPIO_PortClear(GPIOC, 1u << 2);}

#define RUN1_SET() {GPIO_PortClear(GPIOA, 1u << 10);}  //�����ȣ�GPIOA 10
#define RUN1_CLR() {GPIO_PortSet(GPIOA, 1u << 10);}

#define PSRAM_CS_HIGH() {GPIO_PortSet(GPIOD, 1u << 15);}  //�����ȣ�GPIOA 10
#define PSRAM_CS_LOW() {GPIO_PortClear(GPIOD, 1u << 15);}

#define DisableIEPEPower() {GPIO_PortClear(GPIOC, 1u << 4);}  //IEPE,PE4ʹ��
#define EnableIEPEPower() {GPIO_PortSet(GPIOC, 1u << 4);}

#define DisableBatteryChargePower() {GPIO_PortClear(GPIOD, 1u << 1);}  //IEPE,PC3ʹ��
#define EnableBatteryChargePower() {GPIO_PortSet(GPIOD, 1u << 1);}

#endif
