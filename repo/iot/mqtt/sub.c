#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <mosquitto.h>

#define MQTT_HOSTNAME   "localhost"
//#define MQTT_HOSTNAME   "192.168.6.3"
//#define MQTT_PORT       8883
#define MQTT_PORT       1883

char topic[32];
char msg[64];

void my_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	if(message == NULL)
	{
		printf("ERROR in Mosquitto subscription\n");
		return;
	}

	if(message->payloadlen)
	{
		fwrite(message->payload, 1, message->payloadlen, stdout);
		printf("\n");
	}

	sprintf(msg, "Command '%s' completed", message->payload);
	printf("publishing msg: %s\n", msg);
	mosquitto_publish(mosq, NULL, "home/ack", strlen(msg), msg, 0, false);
}

void my_connect_callback(struct mosquitto *mosq, void *obj, int result)
{
	int rc;

	rc = mosquitto_subscribe(mosq, NULL, topic, 0);
	if(rc != MOSQ_ERR_SUCCESS )
	{
		printf("Error1: %s\n",mosquitto_strerror(rc));
		return;
	}

	if(result)
	{
		printf("%s\n", mosquitto_connack_string(result));
		exit(0);
	}
	printf("subscribed to topic: %s\n", topic);
}

int main(int argc, char *argv[])
{
	int rc;
	struct mosquitto *mosq = NULL;
	if (argc != 2) {
		printf("missing topic name\n");
		return 0;
	}

	strcpy(topic, argv[1]);

	mosquitto_lib_init();
	mosq = mosquitto_new (NULL, true, NULL);
	if (!mosq) {
		printf("Can't initialize Mosquitto library\n");
		return -1;
	}

	mosquitto_message_callback_set(mosq, my_message_callback);

	mosquitto_connect_callback_set(mosq, my_connect_callback);

	rc = mosquitto_connect(mosq, MQTT_HOSTNAME, MQTT_PORT, 60);
	if(rc != MOSQ_ERR_SUCCESS )
	{
		printf("Error1 %s (check path names)\n",mosquitto_strerror(rc));
		mosquitto_destroy(mosq);
		mosquitto_lib_cleanup();
		return -1;

	}

	rc = mosquitto_loop_forever(mosq, -1, 1);
	if(rc != MOSQ_ERR_SUCCESS )
	{
		printf("Error1 %s (check path names)\n",mosquitto_strerror(rc));
		mosquitto_destroy(mosq);
		mosquitto_lib_cleanup();
		return -1;

	}

	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
}
