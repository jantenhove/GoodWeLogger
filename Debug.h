#pragma once

#ifndef _DEBUG_h
#define _DEBUG_h
//DEBUG printing
#define REMOTE_DEBUGGING_ENABLED
#define DEBUGGING_ENABLED

#ifdef DEBUGGING_ENABLED

#ifdef REMOTE_DEBUGGING_ENABLED
#include <RemoteDebug.h>  //https://github.com/JoaoLopesF/RemoteDebug
extern RemoteDebug Debug;

#define debugPrintf(fmt, ...) \
					Debug.isRunning() ? Debug.printf( fmt, __VA_ARGS__) : Serial.printf( fmt, __VA_ARGS__)

#define debugPrint(prnt) \
				  Debug.isRunning() ? Debug.print(prnt) :  Serial.print(prnt)

#define debugPrintBase(prnt, base) \
				  Debug.isRunning() ? Debug.print(prnt, base) :  Serial.print(prnt, base)

#define debugPrintln(prnt) \
				  Debug.isRunning() ? Debug.println(prnt) : Serial.println(prnt)
#else

#define debugPrintf(fmt, ...) \
					Serial.printf( fmt, __VA_ARGS__)

#define debugPrint(prnt) \
				   Serial.print(prnt)

#define debugPrintBase(prnt) \
				   Serial.print(prnt,base)

#define debugPrintln(prnt) \
				   Serial.println(prnt)

#endif
#else
#define debugPrintf(fmt, ...) 
#define debugPrint(prnt) 
#define debugPrintBase(prnt, base)
#define debugPrintln(prnt) 
#endif // DEBUGGING_ENABLED
#endif
