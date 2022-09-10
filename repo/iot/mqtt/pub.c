#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <mosquitto.h>
#include <pthread.h>

// Server connection parameters
#define MQTT_HOSTNAME 	"localhost"
#define MQTT_PORT 	1883

static bool connected = true;
static bool disconnect_sent = false;

struct mosquitto *mosq = NULL;
char msg_data[64];
char topic[32];
int flag;

void clean_stdin(void)
{
    int c;
    do {
        c = getchar();
    } while (c != '\n' && c != EOF);
}

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
}

void my_connect_callback(struct mosquitto *mosq, void *obj, int result)
{
        int rc;

        rc = mosquitto_subscribe(mosq, NULL, "home/ack", 0);
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
        printf("subscribed to topic: %s\n", "home/ack");
}

void *thread_pub_func(void *arg)
{
	printf("test3\n");
	int ret;
	printf("publishing msg from thread\n");
	while (1) {
		if (flag) {
			printf("publishing msg on topic: %s\n", topic);
			ret = mosquitto_publish(mosq, NULL, topic, strlen(msg_data), msg_data, 0, false);
			if (ret != MOSQ_ERR_SUCCESS) {
				printf("error in mosquitto_publish\n");
			}
			flag = 0;
		}
		sleep(1);
	}
}

void *thread_sub_func(void *arg)
{
	printf("test4\n");
	int ret;
	struct mosquitto *mosq_sub = NULL;
	        
	mosquitto_lib_init();

        mosq_sub = mosquitto_new (NULL, true, NULL);
        if (!mosq_sub)
        {
                printf("Can't initialize Mosquitto library\n");
                return NULL;
        }

        mosquitto_message_callback_set(mosq_sub, my_message_callback);
        mosquitto_connect_callback_set(mosq_sub, my_connect_callback);

        ret = mosquitto_connect(mosq_sub, MQTT_HOSTNAME, MQTT_PORT, 60);
        if (ret != MOSQ_ERR_SUCCESS)
        {
                printf("Error4 : %s \n", mosquitto_strerror(ret));
                mosquitto_destroy (mosq_sub);
                mosquitto_lib_cleanup();
                return NULL;
        }

	mosquitto_loop_forever(mosq_sub, 20, 1);
}


int main (int argc, char **argv)
{
	int ret, i;
	int choice;

	pthread_t tid_pub;
	pthread_t tid_sub;

	mosquitto_lib_init();

	mosq = mosquitto_new (NULL, true, NULL);
	if (!mosq)
	{
		printf("Can't initialize Mosquitto library\n");
		return -1;
	}

	ret = mosquitto_connect(mosq, MQTT_HOSTNAME, MQTT_PORT, 60);
	if (ret != MOSQ_ERR_SUCCESS)
	{
		printf("Error4 : %s \n", mosquitto_strerror(ret));
		mosquitto_destroy (mosq);
		mosquitto_lib_cleanup();
		return -1;
	}
	
	pthread_create(&tid_pub, NULL, thread_pub_func, NULL);
	printf("test1\n");
	pthread_create(&tid_sub, NULL, thread_sub_func, NULL);
	printf("test2\n");

	while (1) {
		fflush(stdin);
		printf("enter your choice\n1: for light\n2: for fan\n3: for AC\n");
		scanf("%d", &choice);

		switch (choice) {
			case 1:
				strcpy(topic, "home/light");
				printf("enter message for publish\n");
				clean_stdin();
				gets(msg_data);
				flag = 1;
				break;
			case 2:
				strcpy(topic, "home/fan");
				printf("enter message for publish\n");
				clean_stdin();
				gets(msg_data);
				flag = 1;
				break;
			case 3:
				strcpy(topic, "home/AC");
				printf("enter message for publish\n");
				clean_stdin();
				gets(msg_data);
				flag = 1;
				break;
			default:
				printf("wrong choice\n");
		}
	}

	ret = mosquitto_loop_forever(mosq, 20, 1);

	sleep (1);

	mosquitto_destroy (mosq);
	mosquitto_lib_cleanup();

	return 0;
}
