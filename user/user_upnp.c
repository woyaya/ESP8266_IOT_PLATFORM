#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "user_node.h"

#include "esp_common.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "lwip/sockets.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include "user_configstore.h"
#include "user_config.h"
#include "user_node.h"
#include "common.h"
#include "errno.h"

#if ALEXA_NODE
#define ALEXA_UPNP_SEARCH_TARGET	"urn:Belkin:device:**"
#endif
#if HASS_NODE
#define HASS_UPNP_SEARCH_TARGET		"ssdp:all"
#endif

static volatile signed short stop = 1;

#define UPNP_SERVER_PORT	 1900

#ifdef DEBUG
static const int nNetTimeout=5000;	// 5 Sec
#endif	//DEBUG
#define UPNP_SEARCH_MESSAGE_MAX		0xA0
/*****************************************/
static const char *const upnp_search_const_keys[] = {
	"M-SEARCH * HTTP/1.1",
	"HOST: 239.255.255.250:1900",
	"MAN: \"ssdp:discover\"",
	/*"MX:",*/
	"ST:"
};
/******************************************
 * FunctionName : user_upnp_msearch_check
 * Description  : check if a valid M-SEARCH message of UPnP
 * Parameters : str -- message body
 *				search_target -- search target, NULL means no search target check
 * Returns	  : 0: invalid M-SEARCH message
 *					 1: valid M-SEARCH message
******************************************/
LOCAL int
user_upnp_msearch_check(const char *str, const char *search_target)
{
	int i;
	int ret;
	const char *dst;
	if (NULL == str)
		return 0;
	for (i=0;i<ARRAY_SIZE(upnp_search_const_keys);++i)
	{
		dst = strstr(str, upnp_search_const_keys[i]);
		if (NULL == dst)
			return 0;
	}
	if (NULL != search_target)
	{
		int len;
		len = strlen(search_target);
		dst = strstr(str, "ST:");
		dst += 3;
		//skip ' '
		while(' ' == dst[0])
			dst++;
		ret = strncmp(dst, search_target, len);
		if (0 != ret)
			return 0;
	}
	return 1;
}
static const char *const upnp_search_response = 
	"HTTP/1.1 200 OK\r\n"
	"CACHE-CONTROL: max-age=86400\r\n"
	"DATE: Sat, 26 Nov 2016 04:56:29 GMT\r\n"
	"EXT:\r\n"
	"LOCATION: http://%s:%d/setup.xml\r\n"
	"OPT: \"http://schemas.upnp.org/upnp/1/0/\"; ns=01\r\n"
	"01-NLS: b9200ebb-736d-4b93-bf03-%s\r\n"
	"SERVER: Unspecified, UPnP/1.0, Unspecified\r\n"
	"ST: urn:Belkin:device:**\r\n"
	"USN: uuid:Socket-1_0-%s::urn:Belkin:device:**\r\n"
	"X-User-Agent: redsonic\r\n\r\n";

static inline int upnp_response_length(void)
{
	return strlen(upnp_search_response) 
		+ 4*4-1/*max length of IP*/ 
		+ 5/*max length of port*/
		+ 14/*length of NLS*/ 
		+ 10/*length of UUID*/
		- 2/*length of "%?"*/*4 
		+ 1/*'\0'*/;
}

/******************************************
 * FunctionName : user_upnp_make_response
 * Description  : make response of UPnP search
 * Parameters : buf -- response data buffer pointer
 *				length -- The length of data buffer data
 *		  node -- node struct
 *		  ip -- IP address of device
 *		  port -- server port of the node
 * Returns	  : length of response
******************************************/
LOCAL int
user_upnp_make_response(char *buf, int length, const node_t *node,
	const char *ip,  int port)
{
	int ret;
	ret = snprintf(buf, length, upnp_search_response,
		ip, port,
		node->uuid, node->uuid);
	assert(ret < length);
	return ret;
}
/******************************************
 * FunctionName : user_upnp_data_process
 * Description  : Processing the received data from the host
 * Parameters : fd -- socket fd
 *		pusrdata -- The received data (or NULL when the connection has been closed!)
 *		length -- The length of received datia
 *		premote_addr: remote address infor
 * Returns	: none
******************************************/
LOCAL void  
user_upnp_data_process(int fd, 
		char *pusrdata, unsigned short length, 
		struct sockaddr_in *premote_addr)
{
	int i;
	int nodes;
	int valid;
	char *resp;
 
	char ip[4*4]; 
	struct ip_info ipconfig;
	
	if (pusrdata == NULL) {
		return;
	}

	valid = 0;
#if ALEXA_NODE
	valid = user_upnp_msearch_check(pusrdata, ALEXA_UPNP_SEARCH_TARGET);
#endif
#if HASS_NODE
	valid |= user_upnp_msearch_check(pusrdata, HASS_UPNP_SEARCH_TARGET);
#endif
	if (!valid)
	{//Invalid M-SEARCH message
	WRN("Invalid M-SEARCH message: %s", pusrdata);
	return;
	}

	//get IP and MAC from correct interface
	if (wifi_get_opmode() != STATION_MODE) {
		struct ip_info ipconfig_r;
		
		wifi_get_ip_info(SOFTAP_IF, &ipconfig);
		
		inet_addr_to_ipaddr(&ipconfig_r.ip,&premote_addr->sin_addr);
		if(!ip_addr_netcmp(&ipconfig_r.ip, &ipconfig.ip, &ipconfig.netmask)) {
			//printf("udpclient connect with sta\n");
			wifi_get_ip_info(STATION_IF, &ipconfig);
		}
	} else {
		wifi_get_ip_info(STATION_IF, &ipconfig);
	}
	
	DBG("Catch \"%s\" and response\n", pusrdata);
	//alloc buffer for response
	valid = upnp_response_length();
	resp = (char*)malloc(valid);
	if (NULL == resp)
	{
		ERR("Alloc memory for response fail");
		return;
	}
	sprintf(ip, IPSTR, IP2STR(&ipconfig.ip));

	nodes = user_node_get_number();
	for (i=0;i<nodes;++i)
	{
		int ret;
		ret = user_upnp_make_response(resp, valid, NODE(i), 
				ip, __make_server_port(i));
		DBG("%s %d\n", resp,length, ret);
		sendto(fd, resp, ret, 0, 
			(struct sockaddr *)premote_addr, 
			sizeof(struct sockaddr_in));
	}

	if(resp)
		free(resp);
}

/******************************************
 * FunctionName : user_create_udp_server_socket
 * Description  : create socket for udp server
 * Parameters : port: UDP server port
 * Returns	  : <0 : fail
 *					 >=0: UDP socket
******************************************/
LOCAL inline int
user_create_udp_server_socket(int port)
{
	int fd;
	int ret;
	int retry;
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;	
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);
	server_addr.sin_len = sizeof(server_addr);

	retry = 20;
	do{
		fd = socket(AF_INET, SOCK_DGRAM, 0);
		if (fd >= 0)
			break;
		ERR("ERROR:upnp failed to create sock!\n");
		vTaskDelay(1000/portTICK_RATE_MS);
	}while(retry--);
	if (unlikely(fd < 0))
	{
		ERR("Create socket for port %d fail", port);
		return -EBUSY;
	}

	retry = 20;
	do{
		ret = bind(fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
		if (0 == ret)
			break;
		ERR("ERROR:upnp failed to bind sock!\n");
		vTaskDelay(1000/portTICK_RATE_MS);
	}while(retry--);
	if (unlikely(ret))
	{
		ERR("Bind socket to port %d fail", port);
		return -EBUSY;
	}	
	
	return fd;
}

/******************************************
 * FunctionName : user_upnp_init
 * Description  : the espconn struct parame init
 * Parameters : none
 * Returns	  : none
******************************************/
LOCAL void  
user_upnp_task(void *pvParameters)
{
	int ret;

	int sock_fd;	
	struct sockaddr_in from;
	socklen_t fromlen;
	char  *udp_msg;
#ifdef DEBUG
	int stack_counter=0;
#endif
	sock_fd = user_create_udp_server_socket(UPNP_SERVER_PORT);
	udp_msg = (char*)malloc(UPNP_SEARCH_MESSAGE_MAX);
	if (NULL == udp_msg)
	{
		ERR("Alloc memory for UPnP receive buffer fail");
		goto __end;
	}

	while(!stop)
	{
		udp_msg[0] = '\0';
		memset(&from, 0, sizeof(from));
#ifdef DEBUG
		setsockopt(sock_fd,SOL_SOCKET,SO_RCVTIMEO,
				(char *)&nNetTimeout,sizeof(int));
#endif	//DEBUG
		fromlen = sizeof(struct sockaddr_in);
		ret = recvfrom(sock_fd, (u8 *)udp_msg, UPNP_SEARCH_MESSAGE_MAX, 0,
			(struct sockaddr *)&from,(socklen_t *)&fromlen);
		if (ret > 0) {
			MSG("recieve from->port %d  %s\n",
				ntohs(from.sin_port),
				inet_ntoa(from.sin_addr));
			user_upnp_data_process(sock_fd, udp_msg,ret,&from);
		}
#ifdef DEBUG
		if(stack_counter++ ==1){
			stack_counter=0;
			DBG("user_upnp_task %d word left\n",
				uxTaskGetStackHighWaterMark(NULL));
		}
#endif	//DEBUG
	}

	if(udp_msg)free(udp_msg);

__end:
	close(sock_fd);
	vTaskDelete(NULL);

}

/*
LOCAL void  
user_upnp_task1(void *pvParameters)
{
	int stack_counter =0;
	while(1){
		vTaskDelay(1000/portTICK_RATE_MS);
		if(stack_counter++ ==2){
			stack_counter=0;
			DBG("user_upnp_task %d word left\n",uxTaskGetStackHighWaterMark(NULL));//256-186=70
		}
	}
}
*/

void user_upnp_start(void)
{
	assert(stop > 0);	//Can not start more then once
	stop = 0;
	xTaskCreate(user_upnp_task, "user_upnp", 256, NULL, 3, NULL);
	
}

void user_upnp_stop(void)
{
	if (stop > 0)
		return;
	stop++;
	taskYIELD();
}

