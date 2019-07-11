/*
*********************************************************************************************************
*
*	ģ������ : tcp_client
*	�ļ����� : tcp_client.c
*	��    �� : V1.0
*	˵    �� : TCP �ͻ��� �������Ӻ�̨������
*
*	�޸ļ�¼ :
*						�汾��    ����        ����       ˵��
*						V1.0  2019��04��04��  suozhang   �״η���
*
*********************************************************************************************************
*/


#include "tcp_client.h"


/**
 * Log default configuration for EasyLogger.
 * NOTE: Must defined before including the <elog.h>
 */
#if !defined(LOG_TAG)
#define LOG_TAG                    "tcp_client_tag:"
#endif
#undef LOG_LVL
#if defined(XX_LOG_LVL)
    #define LOG_LVL                    XX_LOG_LVL
#endif

#include "elog.h"

#if LWIP_NETCONN

#include "lwip/sys.h"
#include "lwip/api.h"

#include "lwip/tcp.h"
#include "lwip/ip.h"
#include "app.h"

extern TaskHandle_t xHandleTaskLED;

#define TCP_SERVER_IP   "192.168.0.22"
#define TCP_SERVER_PORT 9527

/* ������ͨ��ʹ��Ȩ�ź��� */
SemaphoreHandle_t xServerCommunicationLockSemaphore = NULL;

uint8_t NetReceiveBuf[NetReceiveBufSize];
uint32_t NetReceiveBufHeadIndex=0;
uint32_t NetReceiveBufTailIndex=0;

uint8_t    RxdTelBuf[RX_BUFFER_SIZE];
uint8_t    RXDCmdBuf[RX_BUFFER_SIZE];
uint8_t    CmdBuf[TX_BUFFER_SIZE];

volatile uint16_t RxdBytes=0;
uint16_t CmdBufLength=0;

struct netconn *tcp_client_server_conn;

void RxdByte(uint8_t c);
	
void tcp_client_conn_server_task( void )
{
  struct netbuf *buf;
  err_t err;
	struct pbuf *q;
	uint8_t *NetPbuf;
	ip_addr_t server_ip;

	
	u16_t server_port = TCP_SERVER_PORT;				     // �������˿ںų�ʼ��

	ip4addr_aton( TCP_SERVER_IP, &server_ip ); 			 // ������IP��ַ��ʼ��

	xServerCommunicationLockSemaphore = xSemaphoreCreateBinary();

	if( NULL == xServerCommunicationLockSemaphore )
	{
			log_e("err:xServerCommunicationLockSemaphore == NULL,while(1).");
			while(1);
	}
	
	for( ;; )
	{
				
		log_i("tcp server connecting %s:%d......", ipaddr_ntoa(&server_ip), server_port );
		
		
		xTaskNotify( xHandleTaskLED, 200, eSetValueWithOverwrite );/* �������Ͽ�����״̬��LED��˸Ϊ200mSһ��. */
		
		/* Create a new connection identifier. */
		tcp_client_server_conn = netconn_new( NETCONN_TCP );
				
		if( tcp_client_server_conn != NULL )
		{		
			//��TCP �ı���� ���ͻ��˲�Ĭ�ϴ򿪣���2018��12��6��10:00:41��SuoZhang
			tcp_client_server_conn->pcb.tcp->so_options |= SOF_KEEPALIVE;

			err = netconn_connect( tcp_client_server_conn, &server_ip, server_port );
					
			if(err == ERR_OK)
			{
				log_i("TCP Server %s:%d connected sucess.", ipaddr_ntoa(&server_ip), server_port );
						
				xSemaphoreGive( xServerCommunicationLockSemaphore ); /* �ͷŷ�����ͨ��ʹ��Ȩ */

				xTaskNotify( xHandleTaskLED, 1000, eSetValueWithOverwrite );/* ����������״̬��LED��˸Ϊ1000mSһ��. */
				
				for( ;; )
				{
					/* receive data until the other host closes the connection */
					if((err = netconn_recv(tcp_client_server_conn, &buf)) == ERR_OK) 
					{
														 //��ȡһ��ָ��netbuf �ṹ�е����ݵ�ָ��
						for(q=buf->p;q!=NULL;q=q->next)  //����������pbuf����
						{
							 __disable_irq(); //���ж�
								//�ж�Ҫ������UDP_DEMO_RX_BUFSIZE�е������Ƿ����UDP_DEMO_RX_BUFSIZE��ʣ��ռ䣬�������
								//�Ļ���ֻ����UDP_DEMO_RX_BUFSIZE��ʣ�೤�ȵ����ݣ�����Ļ��Ϳ������е�����
							NetPbuf=q->payload;
							for(uint32_t i=0;i<q->len;i++)
							{
								if(!isNetReceiveBufFull())
								{
									NetReceiveBuf[NetReceiveBufHeadIndex]=*NetPbuf;
									NetPbuf++;
									
									IncreaseNetReceiveBuf(NetReceiveBufHeadIndex);
								}
							}	
							 __enable_irq();  //���ж� 
						}	
						netbuf_delete(buf);						
					}
					else//if((err = netconn_recv(conn, &buf)) == ERR_OK)
					{
						log_e("err:netconn_recv(conn, &buf):%d.",err);
						netbuf_delete(buf);	
						break; //���ӷ��������˳��������ݵ�ѭ�������½�������
					}
				 }
			 }
			
			 log_e("err:TCP Server %s:%d connect fail,err:%d.", ipaddr_ntoa(&server_ip), server_port, err );
			 netconn_close  ( tcp_client_server_conn );
			 netconn_delete ( tcp_client_server_conn );		
		   vTaskDelay(1000);
		}
		else//(conn!=NULL)
		{
			log_e("err:Can not create tcp_client_server_conn.");
			vTaskDelay(1000);
		}
	}
}

void receive_server_data_task( void )
{
	for(;;)
	{
		if(!isNetReceiveBufEmpty()) //
		{
			RxdByte(NetReceiveBuf[NetReceiveBufTailIndex]);
			IncreaseNetReceiveBuf(NetReceiveBufTailIndex);
		} else
		{
		vTaskDelay(2);

		}
	}
}

uint8_t command_start()
{ 
	//EnableIEPEPower();
//	 for(uint32_t i=0;i<add_data;i++)
//		xx[i]+=i;
	 {
		for(uint8_t i=0;i<CmdBufLength;i++)
		CmdBuf[i]=RXDCmdBuf[i];
	  CmdBufLength=CmdBufLength;
	 }
	 
	 return 1;
}


uint8_t AnalyCmd(uint16_t length)
{ 

	switch( RXDCmdBuf[1] ){     //��������
//		case COMMAND_ID:     //0x02
//			   command_id();
//			break;
//		case COMMAND_CHANNELKIND:     //0x03 ͨ����������
//			   command_channelkind();
//			break;
//		case COMMAND_REPLYIP:     //0x03 ͨ����������
//			   command_replyip();
//			break;
//		case COMMAND_STOP:
//			   command_stop();
//			break;
//		case COMMAND_RECEIVE_BEACON:
//				command_receive_beacon();
//			break;
//		case COMMAND_SETIP:   //����IP��ַ
//			   command_setip();
//			break;
//		case COMMAND_SET_RUNMODE:   //����IP��ַ
//			   command_setrunmode();
//			break;
		case COMMAND_START:
			   command_start();
			break;
//		case COMMAND_REQUIRE_PERIODWAVE:
//			   command_require_periodwave();
//			break;
//		case COMMAND_SET_AP:
//			   command_set_ap();
//			break;
//		case COMMAND_RECEIVE_ACTIVE_BEACON:
//					command_receive_active_beacon();
//			break;
//		case COMMAND_SET_FACTORY_PARAMETER:
//					command_set_factory_parameter();
//			break;
//		case COMMAND_SET_CHANNEL_CONDITION:
//			   command_set_channel_condition();
//			break;
//		case COMMAND_REPLY_CHANNEL_CONDITION:
//			   command_reply_channel_condition();
//			break;
//		case COMMAND_REPLY_RATE:    //���ò�������
//			   command_reply_SampleParameter();
//		  break;
//		case COMMAND_SAMPLE_RATE:    //���ò�������
//			   command_set_SampleParameter();
//			break;
//		case COMMAND_SET_TCPSERVE:   //����tcp server��Ŀ���ַ����
//			   command_set_tcpserve();
//		  break;
//		case COMMAND_REPLYTCPSERVE:   //����tcp server��Ŀ���ַ����
//			   command_replytcpserve();
//			break; 
//		case COMMAND_SET_TIME:           //����apֵ
//			   command_set_time();
//      break;		
//		case COMMAND_REPLYAP:
//			   command_replyap();  //����apֵ
//			break;
//    case COMMAND_APPLYNETSET:
//			   command_applynetset();  //Ӧ����������
//			break;
//		 case 0x40:
//			   command_counter();  //Ӧ����������
//			break;
//		case COMMAND_REPLY_RUNMODE:
//			   command_reply_runmode();
//		  break;
//		case COMMAND_ADJUST_TEMP:
//			   command_adjust_temp();
//		  break;
//		case COMMAND_ADJUST_ADC:
//			   command_adjust_adc();
//		  break;
//		case COMMAND_SET_SCALE:
//			   command_set_scale();
//			break;
//		case COMMAND_REPLY_SCALE:
//			   command_reply_scale();
//			break;
//    case COMMAND_SET_SNNUMBER:
//			   command_set_snnumber();
//			break;
//		case COMMAND_IAP_STATUE:
//			   command_iap_statue();
//			break;
//		case COMMAND_REPLY_SW_VERSION:
//			   command_reply_sw_version();
//			break;
//		case COMMAND_IAP_DATA:
//			   command_iap_data();
//			break;
//		case COMMAND_RESET_SYSTEM:
//			   command_reset_system();
			break;
		default:
			return 1;
	}
 return 1;
	
}


uint16_t getTelLength(void)  //�����TLV��V�ĳ���
{
					
	return(((uint16_t)RxdTelBuf[3]<<8)+RxdTelBuf[2]);
  
}
uint8_t  isVaildTel(void)
{
	if(RxdBytes>=1)if(RxdTelBuf[0]!=0x7e)return(0);
	if(RxdBytes>=2)if((RxdTelBuf[1]!=COMMAND_START)&&(RxdTelBuf[1]!=COMMAND_STOP)&&(RxdTelBuf[1]!=COMMAND_ID)
		                  &&(RxdTelBuf[1]!=COMMAND_CHANNELKIND)&&(RxdTelBuf[1]!=COMMAND_REPLYIP)&&(RxdTelBuf[1]!=COMMAND_SETIP)
						  &&(RxdTelBuf[1]!=COMMAND_REPLY_RATE)&&(RxdTelBuf[1]!=COMMAND_SAMPLE_RATE)&&(RxdTelBuf[1]!=COMMAND_ADJUST_TEMP)
						  &&(RxdTelBuf[1]!=COMMAND_REPLY_SCALE)&&(RxdTelBuf[1]!=COMMAND_SET_SCALE)&&(RxdTelBuf[1]!=COMMAND_ADJUST_ADC)
						  &&(RxdTelBuf[1]!=COMMAND_SET_SNNUMBER)&&(RxdTelBuf[1]!=COMMAND_REQUIRE_PERIODWAVE)&&(RxdTelBuf[1]!=COMMAND_SET_CHANNEL_CONDITION)
							&&(RxdTelBuf[1]!=COMMAND_SET_RUNMODE)&&(RxdTelBuf[1]!=COMMAND_SET_TIME)&&(RxdTelBuf[1]!=COMMAND_SET_AP)
						  &&(RxdTelBuf[1]!=COMMAND_RECEIVE_BEACON)&&(RxdTelBuf[1]!=COMMAND_SET_TCPSERVE)&&(RxdTelBuf[1]!=COMMAND_REPLYAP)
						  &&(RxdTelBuf[1]!=COMMAND_REPLYTCPSERVE)&&(RxdTelBuf[1]!=COMMAND_APPLYNETSET)&&(RxdTelBuf[1]!=COMMAND_REPLY_RUNMODE)
							&&(RxdTelBuf[1]!=COMMAND_REPLY_CHANNEL_CONDITION)&&(RxdTelBuf[1]!=COMMAND_RECEIVE_ACTIVE_BEACON)&&(RxdTelBuf[1]!=COMMAND_SET_FACTORY_PARAMETER)
							&&(RxdTelBuf[1]!=COMMAND_IAP_DATA)&&(RxdTelBuf[1]!=COMMAND_IAP_STATUE)&&(RxdTelBuf[1]!=COMMAND_REPLY_SW_VERSION)&&(RxdTelBuf[1]!=COMMAND_RESET_SYSTEM))
				return(0);  //���������ID
	
  if(RxdBytes>=4) { 
		uint16_t length=getTelLength();
		if((length>200)) return(0);  //��������Ϊ1000
	}
	return(1);				 // �Ϸ���
}
uint8_t sumofRxdBuf(uint16_t l)  //��͵ı��ģ���������ʼ��ʶ,	l	�ĳ��Ȱ�����ʼ��ʶ
{ 
	uint8_t sum=0;
	if(l<2) return (0);
	for(uint16_t i=1;i<l-2;i++)
	 sum=sum+RxdTelBuf[i];
	return (sum);
}
uint8_t isTelComplete(void)	   // =0 ������  =1 sum Error =2 ��ȷ
{
	uint32_t  temp8;
	uint32_t   dat_len;

	if(RxdBytes<4)return(0);
  ////////////////
	dat_len=getTelLength()+6;	//
	if(dat_len==0)return(0);
	if(RxdBytes<(dat_len))return(0);

	temp8=sumofRxdBuf(dat_len);
 
  if (RxdTelBuf[dat_len-1]==0x7e)
		return(2); 
	if (RxdTelBuf[dat_len-2]==temp8)
		return(2); 
	else{
		return(1);
	}	
}						 

uint8_t leftRxdTel(void)		//��������һλ
{
	uint32_t i;
	if(RxdBytes<1)return(0);     // �޷�����
	for	(i=1;i<RxdBytes;i++)
	{
		RxdTelBuf[i-1]=RxdTelBuf[i];		
	}
	RxdBytes--;
	return(1);					 // ����һ���ֽڳɹ�

}

 void RxdByte(uint8_t c)
{	
	uint32_t 	i;
	RxdTelBuf[RxdBytes]=c;
	RxdBytes++;

	switch(RxdBytes)
	{
		case 0:	break;
		case 3:	break;
		case 1:
		case 2:
		case 4:while(!isVaildTel())	//������Ϸ�			 
				{
					if(!leftRxdTel())break;	  // �������ֽ�
				}
				break;
			
		
		default:		
				i=isTelComplete();
				if(i==2)
				{
					//do some thing
					for(uint16_t j=0;j<RxdBytes;j++)
					RXDCmdBuf[j]=RxdTelBuf[j];
					CmdBufLength=RxdBytes;
					AnalyCmd(CmdBufLength);
					
					RxdBytes=0;
					
				}
				else if(i==1)	 // CRC error
				{
					leftRxdTel();
					while(!isVaildTel())	//������Ϸ�			 
					{
						if(!leftRxdTel())break;
					}	
				}
				else if(i==0) //û���������
				{
				
				}
				else
				{
				}
				break;
			
		}
	
}
int send_server_data( uint8_t *data, uint16_t len )
{
												 
	if( tcp_client_server_conn )
	{
		return netconn_write( tcp_client_server_conn, data, len, NETCONN_COPY);
	}
	else
		
	
		return ERR_CONN; 

}

#endif /* LWIP_NETCONN */
















