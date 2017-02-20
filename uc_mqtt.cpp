#include "uc_mqtt.h"
#include "tcp.h"
TCP tcp_mqtt;
bool connected = false;
unsigned int len_buffer_in_module = 0;
void func_null(String topic ,char *playload,unsigned char length){}

const long interval_ping = 10000; 
unsigned long previousMillis_ping; 
unsigned long currentMillis_ping;  
	
UCxMQTT::UCxMQTT()
{
	//void (*callback)(String topic ,char *playload,unsigned char length);
	callback = func_null;
	
}
bool UCxMQTT::ConnectMQTTServer(String web , String port)
{
	//connected = tcp_mqtt.Open(1,0,"TCP",web,port,"0",1); //direct mode
	connected = tcp_mqtt.Open(1,0,"TCP",web,port,"0",0);//buffer mode	
	return (connected); 
}

bool UCxMQTT::DisconnectMQTTServer()
{
	connected = false;
	return (tcp_mqtt.Close()); 
}

unsigned char UCxMQTT::Connect(String id,String user,String pass) 
{
		    connected = false;
		    // Leave room in the buffer for header and variable length field
            uint16_t length = 0;
            unsigned int j;
			unsigned char ctrl_flag = 0x02;
			buffer[0]=0x10;    // Control packet type     (Fix=10)
			buffer[1]=0x00;    // Remain Length
			buffer[2]=0x00;    // Protocol name Lengh MSB (Fix=0)
			buffer[3]=0x04;    // Protocol name Lengh LSB (Fix=4)
			buffer[4]='M';     // Protocol name   (Fix=M)
			buffer[5]='Q';     // Protocol name   (Fix=Q)
			buffer[6]='T';     // Protocol name   (Fix=T)
			buffer[7]='T';     // Protocol name   (Fix=T)
			buffer[8]=0x04;    // Protocol Level  (Fix=4)
			//buffer[9]=0xC2;    // Control Flag (Bit7 : user name flag),(Bit6 : password flag) , (Bit5 : will retrain) , (Bit4-3 : will Qos) , (Bit2 : will flag) , (Bit1 : clear section) , (Bit0 : fix0)
			//buffer[9]=0x02;
			buffer[10]=0x00;   // keep alive MSB
			buffer[11]=0x0F;   // keep alive LSB time ping 16 bit (seconds)

			
			unsigned char  i;
			unsigned char len = id.length();
			buffer[12] = (len>>8)&0x00FF;   // id Lengh MSB
			buffer[13] = len&0x00FF;        // id Lengh LSB
			
			length = 14;
			for(i=0;i<len;i++)
			{
				buffer[length] = id[i];
				length++;
			}

			len = user.length();
			if(len>0)
				ctrl_flag |= 1<<7;
			buffer[length] = (len>>8)&0x00FF;   // id Lengh MSB
			length++;
			buffer[length] = len&0x00FF;        // id Lengh LSB
			length++;
			for(i=0;i<len;i++)
			{
				buffer[length] = user[i];
				length++;
			}

			len = pass.length();
			if(len>0)
				ctrl_flag |= 1<<6;
			buffer[length] = (len>>8)&0x00FF;   // id Lengh MSB
			length++;
			buffer[length] = len&0x00FF;        // id Lengh LSB
			length++;
			for(i=0;i<len;i++)
			{
				buffer[length] = pass[i];
				length++;
			}
			buffer[1]= length-2;
			buffer[9]=ctrl_flag;
			
		/*	for(i=0;i<length;i++)
			{
				 Serial.print(" ");
				Serial.print(buffer[i],16);
			}
	*/
			
			
	 write_tcp(buffer,length);	
		//write(MQTTCONNECT,buffer,length-5);
		//int ret = red_data_from_3G();
		
		
		
		
		
		int ret = red_data_from_3G_buffermode();
		
		
		if(ret==-1)
			connected = false;
		else if(buffer[ret]==0)
			connected = true;
		else
			connected = false;
		//Serial.println(ret);
		return(buffer[ret]);
}
/*unsigned char UCxMQTT::Connect(const char *id, const char *user, const char *pass) 
{
	return(Connect(id,user,pass,0,0,0,0));
}
*/
int UCxMQTT::red_data_from_3G_buffermode()
{
	int ret = 99;
	unsigned long pv_timeout   = millis();;
	const long interval_timeout = 5000;
	bool state_data_in=false;
	//Serial.println();
	//Serial.println("testbuf");
	//Serial.println();
	
	while(1)
	{
		unsigned long current_timeout = millis();
		ret=0;
		if(tcp_mqtt.ReceiveAvailable())
		{
			int len = tcp_mqtt.ReadBuffer(10);
			//Serial.println("len = ");
			//Serial.println(len);
			while(len)
			{
				if(gsm.available())
				{
					char c = gsm.read();
					buffer[ret] = c;
					ret++;
					//Serial.print(c,HEX);
					len--;
				}
			}
			if(buffer[0]=0x20)
			{
				ret-=1;
				//Serial.println(ret);
				return(ret);
			}
		}
		if(current_timeout - pv_timeout >= interval_timeout)
		{
			//Serial.println("timeout");
			return(-1);
		}
	}
	
}


uint8_t UCxMQTT::write_tcp(uint8_t* buf, uint8_t length) 
{
	uint8_t len_let=0;
	if(tcp_mqtt.StartSend(0,length))
	{
		for(int itcp=0;itcp<length;itcp++)
	   {
			tcp_mqtt.write(buf[itcp]);
			len_let++;
	   }
	   if(!tcp_mqtt.WaitSendFinish())
	   {
		   Serial.println("false unfinish");
		   connected = false;
	   }
	}
	else
	{
		connected = false;
	}
   return(len_let);
}

String UCxMQTT:: ConnectReturnCode(unsigned char input)
{
	switch(input)
	{
		case 0:
			return(F("Connection Accepted"));
		break;
		case 1:
			return(F("Connection Refused, unacceptable protocol version"));
		break;
		case 2:
			return(F("Connection Refused, identifier rejected"));
		break;
		case 3:
			return(F("Connection Refused, Server unavailable"));
		break;
		case 4:
			return(F("Connection Refused, bad user name or password"));
		break;
		case 5:
			return(F("Connection Refused, not authorized"));
		break;
		default:
			return(F("Unknow!!!"));
		break;
	}
}
void UCxMQTT::Publish(char *topic ,int lentopic, char *payload ,int lenpay)
{
	 
	 clear_buffer();

	 buffer[0]=0x30;    // Control packet type (Fix=3x) ,x bit3 = DUP , xbit2-1 = Qos level , xbit0 = Retain
	 buffer[1]=0x00;     // remaining length

	 int i=0;
	 int len = lentopic;
	 buffer[2]=(len>>8)&0x00FF;    //  topic Lengh MSB
	 buffer[3]=len&0x00FF;    //  topic Lengh LSB

	 int all_len = 4;
	 for(i=0;i<len;i++)
	 {
		 buffer[all_len] = topic[i];
	     all_len++;
	 }

	 len = lenpay;
	 for(i=0;i<len;i++)
	 {
		 buffer[all_len] = payload[i];
	     all_len++;
	 }

	 buffer[1]=all_len-2;
	//Serial.println();
	/*for(i=0;i<all_len;i++)
	{
		Serial.print(" ");
		Serial.print(buffer[i],16);
		//Ql_Debug_Trace("%02x",data[i]);
	}
	*/
	//Serial.println();
//	Serial.println(all_len);
	
	write_tcp(buffer,all_len);
}
void UCxMQTT::Publish(String topic , String payload)
{
	
	char chartopic[topic.length()+2];
	char charpay[payload.length()+2];
	unsigned char i=0;
	for(i=0;i<topic.length();i++)
	{
		chartopic[i] = topic[i];
	}
	chartopic[i]=0;
	
	for(i=0;i<payload.length();i++)
	{
		charpay[i] = payload[i];
	}
	charpay[i]=0;
	
	Publish(chartopic,topic.length(),charpay,payload.length());
}

void UCxMQTT::Subscribe(char *topic,int topiclen)
{
	clear_buffer();   
    buffer[0]=0x82;
    buffer[1]=0x00; //Remaining Length
    buffer[2]=0x00;    // Packet Identifier MSB
    buffer[3]=0x02;    // Packet Identifier LSB

    int i=0;
    int len = topiclen;
    buffer[4]=(len>>8)&0x00FF;;    //  topic Lengh MSB
    buffer[5]=len&0x00FF;    //  topic Lengh LSB

    int all_len = 6;
     for(i=0;i<len;i++)
    {
        buffer[all_len] = topic[i];
         all_len++;
    }
    buffer[all_len] = 0;
    buffer[1] = all_len-1;

    //send_tcp_data(buffer,all_len+1);
	write_tcp(buffer,all_len+1);
    //socket->write(data,all_len+1);
	//Serial.println();
    /* for(i=0;i<all_len+1;i++)
     {
         Serial.print(" ");
		Serial.print(buffer[i],16);
     }
*/

}
void UCxMQTT::Subscribe(String topic)
{
	char chartopic[topic.length()+2];
	unsigned char i=0;
	for(i=0;i<topic.length();i++)
	{
		chartopic[i] = topic[i];
	}
	chartopic[i]=0;
	
	Subscribe(chartopic,topic.length());
}
void UCxMQTT:: clear_buffer()
{
	for(int i=0;i<MQTT_MAX_PACKET_SIZE;i++)
	{
		buffer[i]=0;
	}
	gsm.flush();
}
void UCxMQTT::Ping()
{
	buffer[0] = 0xC0;
	buffer[1] = 0x00;
	write_tcp(buffer,2);
	//return(connected);
}
void UCxMQTT::MqttLoop()
{
	unsigned char ret;
	currentMillis_ping = millis();
	static bool ping_flag=true;
	
	if(gsm.available())
	{
			String req = gsm.readStringUntil('\r');
			//Serial.println(req);
			if(req.indexOf(F("+QIURC: \"closed\""))!= -1)
			{
				connected = false;
				clear_buffer(); 
				return;
			}

	}
				unsigned int buf_cnt=0;
				while(1)
				{
					unsigned int len_in_buffer = ReadDataInBufferMode(1);
					if(len_in_buffer==0)
					{
						clear_buffer();
						return;						
					}
					//Serial.println(buffer[0],HEX);
					switch (buffer[0])
					{
						case 0x30: // rx_sub
						Serial.println("rx sub");
							check_rx_sub();
							return;
						break;
						case 0xD0: //ping
							ReadDataInBufferMode(1);
							if(buffer[0]==0x00)
							{
								connected = true;
								ping_flag = true;
								Serial.println("ping ok");
								return;
							}
							else
							{
								ping_flag = false;
								connected = false;
								Serial.println("ping fail");
								clear_buffer();
								return;	
							}
							
						break;
						default:
							Serial.println("tout");
							clear_buffer();
							return;							
						break;
					}
					
					currentMillis_ping = millis();
					if(currentMillis_ping - previousMillis_ping >= interval_ping) 
					{
						Ping();
						previousMillis_ping = currentMillis_ping ;
					}					
				}
				
	
	
}

void UCxMQTT::check_rx_sub()
{
	unsigned int all_byte;
	unsigned char topic_len;
	unsigned char topic_cnt=0;
	unsigned char playload_cnt=0;
	unsigned int i;
	unsigned int len_in_buffer = ReadDataInBufferMode(1);
	all_byte = buffer[0];
	len_in_buffer = ReadDataInBufferMode(2);
	topic_len = buffer[1];
	len_in_buffer = ReadDataInBufferMode(all_byte-2);
	char topic[topic_len+1];
	char playload[len_in_buffer+1];
	//char topic[100];
	//char playload[100];
	
	for(i=0;i<len_in_buffer;i++)
	{
		if(i<topic_len)
		{
			topic[topic_cnt]=buffer[i];
			topic_cnt++;
			if(topic_cnt>=100)
				break;
			
		}
		else
		{
			playload[playload_cnt] = buffer[i];
			playload_cnt++;
			if(playload_cnt>100)
				break;
		}
	}
	topic[topic_cnt]=0;
		String str_topic(topic);
		(*callback)(str_topic,playload,playload_cnt);
}

unsigned int UCxMQTT::ReadDataInBufferMode(unsigned int buf_len)
{
	unsigned int len = tcp_mqtt.ReadBuffer(buf_len);
	unsigned int re_turn = len; 
	unsigned int ret=0;
	
	//Serial.println("");
	//Serial.print("len = ");
	//Serial.println(len);
	while(len)
	{
		if(gsm.available())
		{
			char c = gsm.read();
			//Serial.print(c,HEX);
			buffer[ret] = c;
			ret++;
			//Serial.print(c,HEX);
			len--;
			if(ret > MQTT_MAX_PACKET_SIZE)
				return(ret);
		}
	}
	gsm.flush();
		return(re_turn);
}
bool UCxMQTT::ConnectState()
{
	return(connected);
}













