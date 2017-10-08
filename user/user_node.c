#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "user_node.h"
#include "user_config.h"
#include "common.h"

#if ALEXA_NODE
#define ALEXA_UPNP_SEARCH_TARGET	"urn:Belkin:device:**"
#endif
#if HASS_NODE
#define HASS_UPNP_SEARCH_TARGET		"ssdp:all"
#endif

static const node_hardware_t node_ios[] = 
{
	{
		.key_io =  12,
		.relay_io = 15,	
	},

};
#define USER_NODES_NUMBER	(ARRAY_SIZE(node_ios))

#if (UUID_LENGTH_MAX <= (8+2))
  #error UUID_LENGTH_MAX MUST bigger then 10
#endif
static void __make_uuid(char *buf, int size, int index)
{
	int ret;
	uint32_t chip_id;
	chip_id = system_get_chip_id();
	ret = snprintf(buf, size, "%08x%02x", chip_id, index);
	assert(ret < size);
	return;
}

/******************************************
 * FunctionName : user_node_init
 * Description  : initial one node, set struct to default value
 * Parameters   : node: node struct pointer
 *                index: index of the node
 * Returns      : none
******************************************/
void user_node_init(node_t *node, int index)
{
	int ret;
	assert(index < USER_NODES_NUMBER);
	//node->port = __make_server_port(index);
	__make_uuid(node->uuid, sizeof(node->uuid), index);
	ret = snprintf(node->name, sizeof(node->name),
			"Simulater %02x", index);
	assert(ret < sizeof(node->name));
	node->hardware = node_ios[index];

	return;
}
/******************************************
 * FunctionName : user_node_get_number
 * Description  : Get total number of nodes
 * Parameters   : none
 * Returns      : number of nodes
******************************************/
int user_node_get_number(void)
{
	//if following assert fail, Check setting of 
	//"MAX_NODE_NUMBER" in file "user_config.h"
	assert(USER_NODES_NUMBER <= MAX_NODE_NUMBER);	
	return USER_NODES_NUMBER;
}
