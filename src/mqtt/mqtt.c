#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <memory.h>
#include <sys/time.h>
#include <stdlib.h>
#include "MQTTClient.h"
#include "mqtt.h"
#include <strings.h>
//#define __DEBUG
#include "debug.h"


extern char USE_MQTT[16];
extern char MQTT_BROKER[16];
extern char MQTT_PORT[16];
extern char MQTT_PASSWORD[16];
extern char MQTT_USERNAME[16];

volatile int toStop = 0;


struct opts_struct
{
	char *clientid;
	int nodelimiter;
	char *delimiter;
	enum QoS qos;
	char *username;
	char *password;
	char *host;
	int port;
	int showtopics;
};



void cfinish (int sig)
{
	signal (SIGINT, NULL);
	toStop = 1;
}


int hostapd_pub (char *topic, char *msg)
{
	if(atoi(USE_MQTT)==0)
		return 0;

	struct opts_struct opts;

	opts.clientid=(char*)"hostapd";
	opts.nodelimiter=0;
	opts.delimiter=(char *)"\n";
	opts.qos=QOS2;
	opts.host=(char *)MQTT_BROKER;
	opts.username=MQTT_USERNAME;
	opts.password=MQTT_PASSWORD;
	opts.port=atoi(MQTT_PORT);
	opts.showtopics=0;

	int rc = 0;
	unsigned char buf[100], readbuf[100];

	Network n;
	Client MQ_CLIENT;

	signal (SIGINT, cfinish);
	signal (SIGTERM, cfinish);

	NewNetwork (&n);
	ConnectNetwork (&n, opts.host, opts.port);
	MQTTClient (&MQ_CLIENT, &n, 1000, buf, 100, readbuf, 100);

	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;

	data.willFlag = 0;
	data.MQTTVersion = 3;
	data.clientID.cstring = opts.clientid;
	data.username.cstring = opts.username;
	data.password.cstring = opts.password;
	data.keepAliveInterval = 10;
	data.cleansession = 1;

	DEBUG ("Connecting to %s %d", opts.host, opts.port);
	rc = MQTTConnect (&MQ_CLIENT, &data);
	if(rc!=0)
	{
		printf("\r\nconnect to mqtt broker failed.\r\n");
		return 0;
	}
	DEBUG ("Connected %d", rc);

    struct MQTTMessage msgs;

    msgs.qos=QOS0;
    msgs.retained=0;
    msgs.dup=0;
    msgs.id=0;
    msgs.payload=msg;
    msgs.payloadlen=strlen(msg);
    MQTTPublish (&MQ_CLIENT, topic, &msgs);

	DEBUG("Pub...");
	MQTTDisconnect(&MQ_CLIENT);
	n.disconnect(&n);
	DEBUG("Done...");

    return 0;
}
