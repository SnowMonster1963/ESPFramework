/*
 * CmdProcessor.h
 *
 *  Created on: Feb 28, 2016
 *      Author: tsnow
 */

#ifndef INCLUDE_CMDPROCESSOR_H_
#define INCLUDE_CMDPROCESSOR_H_

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <gpio.h>
#include "driver/Timer.h"
#include "driver/ConfigManager.h"
#include "driver/Debug.h"
#include "driver/Stream.h"
#include "driver/Process.h"
#include "driver/WiFi.h"
#include "driver/Socket.h"
#include "driver/Mqtt.h"

class MyEOLProcess : public EOLProcess
	{
public:
	MyEOLProcess();
	void OnLineSent(const char *pszLine);
	};

extern EOLProcess *eolp;

#endif /* INCLUDE_CMDPROCESSOR_H_ */
