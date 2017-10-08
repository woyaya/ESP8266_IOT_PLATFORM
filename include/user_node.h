#ifndef __USER_NODES__
#define __USER_NODES__

#include <stdint.h>
#include "user_config.h"

#define NODE_SERVER_BASE_PORT	5000

#define NAME_LENGTH_MAX		32		
#define UUID_LENGTH_MAX		12

typedef struct{
	uint16_t key_io;			//key IO
	uint16_t relay_io;			//relay IO
}node_hardware_t;

typedef struct{
	char name[NAME_LENGTH_MAX];	//human friendly name
	char uuid[UUID_LENGTH_MAX];	//uuid
	node_hardware_t hardware;
}node_t;

#define NODE_NAME(node)		((node)->name)
#define NODE_UUID(node)		((node)->uuid)
#define NODE_KEY(node)		((node)->hardware.key_io)
#define NODE_RELAY(node)	((node)->hardware.relay_io)

/******************************************
 * FunctionName : user_node_init
 * Description  : initial one node, set struct to default value
 * Parameters   : node: node struct pointer
 *                index: index of the node
 * Returns      : none
******************************************/
void user_node_init(node_t *node, int index);
/******************************************
 * FunctionName : user_node_get_number
 * Description  : Get total number of nodes
 * Parameters   : none
 * Returns      : number of nodes
******************************************/
int user_node_get_number(void);
#endif	//__USER_NODES__

