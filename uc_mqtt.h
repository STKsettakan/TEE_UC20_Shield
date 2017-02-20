#ifndef uc_mqtt_h
#define uc_mqtt_h
#include "TEE_UC20.h"

#define MQTT_MAX_PACKET_SIZE 128
class UCxMQTT
{
	private:
	uint8_t buffer[MQTT_MAX_PACKET_SIZE];
	uint8_t write_tcp(uint8_t* buf, uint8_t length); 
	int red_data_from_3G_buffermode();
	unsigned int ReadDataInBufferMode(unsigned int buf_len);
	void check_rx_sub();
	
	public:
	UCxMQTT();
	bool ConnectMQTTServer(String web , String port);
	bool DisconnectMQTTServer();
	bool ConnectState();
	unsigned char Connect(String id,String user,String pass);
	String ConnectReturnCode(unsigned char input);
	void Publish(char *topic ,int lentopic, char *payload ,int lenpay);
	void Publish(String topic , String payload);
	void Subscribe(char *topic,int topiclen);
	void Subscribe(String topic);
	void clear_buffer();
	void Ping();
	void MqttLoop();
	void (*callback)(String topic ,char *playload,unsigned char length);
	
};	
#endif