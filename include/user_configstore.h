#ifndef __USER_CONFIGSTORE_H__
#define __USER_CONFIGSTORE_H__

#include "user_config.h"
#include "user_node.h"
#include "user_light_action.h"
#include "c_types.h"

#define CONFIG_MAC_CNT 16
#define DEVICE_NAME_LENGTH_MAX	32

typedef struct {
	uint32_t r;
	uint32_t g;
	uint32_t b;
	uint32_t cw;
	uint32_t ww;
} LedCol;

typedef struct {
	uint8_t chsum;
	uint8_t seq;
	uint8_t pad;
//	uint8_t wifiChan;
#if LIGHT_DEVICE
	LedCol bval[5];
#endif
#if HASS_NODE || ALEXA_NODE
	node_t nodes[MAX_NODE_NUMBER];
#endif
//	LampMacType macData[CONFIG_MAC_CNT];
	int wlanChannel[CONFIG_MAC_CNT];
} MyConfig;

extern MyConfig myConfig;
void configLoad();
void configSave();

#if HASS_NODE || ALEXA_NODE
#define NODE(i)		(&(myConfig.nodes[i]))
#endif

#endif
