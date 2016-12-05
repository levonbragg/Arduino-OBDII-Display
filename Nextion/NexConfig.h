/**
 * @file NexConfig.h
 *
 * Options for user can be found here. 
 *
 * @author  Wu Pengfei (email:<pengfei.wu@itead.cc>)
 * @date    2015/8/13
 * @copyright 
 * Copyright (C) 2014-2015 ITEAD Intelligent Systems Co., Ltd. \n
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */
#ifndef __NEXCONFIG_H__
#define __NEXCONFIG_H__

//*******************************************************************
// UN comment to use Software Serial for Nextion...
	
	#include <SoftwareSerial.h>
	extern SoftwareSerial HMISerial;
	#define nexSerial HMISerial

// Add these lines to your arduino sketch for software serial
// #include "Nextion.h"
// // Master RX, TX, connect to Nextion TX, RX
// SoftwareSerial HMISerial(10,11);
//*******************************************************************

/**
 * @addtogroup Configuration 
 * @{ 
 */

/** 
 * Define DEBUG_SERIAL_ENABLE to enable debug serial. 
 * Comment it to disable debug serial. 
 */
//#define DEBUG_SERIAL_ENABLE

/**
 * Define dbSerial for the output of debug messages. 
 */
#define dbSerial Serial

/**
 * Define nexSerial for communicate with Nextion touch panel. 
 * comment this line out to use SoftwareSerial
 */
//#define nexSerial Serial2


#ifdef DEBUG_SERIAL_ENABLE
#define dbSerialPrint(a)    dbSerial.print(a)
#define dbSerialPrintln(a)  dbSerial.println(a)
#define dbSerialBegin(a)    dbSerial.begin(a)
#else
#define dbSerialPrint(a)    do{}while(0)
#define dbSerialPrintln(a)  do{}while(0)
#define dbSerialBegin(a)    do{}while(0)
#endif

/**
 * @}
 */

#endif /* #ifndef __NEXCONFIG_H__ */
