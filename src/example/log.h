/*
	日志定义 by 陈文清 2007.7
*/
#ifndef _CWQ_LOG_H
#define _CWQ_LOG_H
#include "singleton.h"
#include "logbase.h"

#define LOGGER				Singleton<LogBase>::me()
#define rlog				LOGGER->LogInfo
#define dlog0(args...)		LOGGER->DebugInfo(0, args)
#define dlog1(args...)		LOGGER->DebugInfo(1, args)
#define dlog2(args...)		LOGGER->DebugInfo(2, args)
#define dlog3(args...)		LOGGER->DebugInfo(3, args)
#define dlog4(args...)		LOGGER->DebugInfo(4, args)
#define dlog5(args...)		LOGGER->DebugInfo(5, args)
#define dlog(l, args...)	LOGGER->DebugInfo(l, args)
#define makestr				    LOGGER->MakeStr

#define dlog_error(fmt_, ...)  LOGGER->DebugInfo(1, "[error] " fmt_,  ##__VA_ARGS__)
#define dlog_warn(fmt_, ...)   LOGGER->DebugInfo(1, "[warn] " fmt_,  ##__VA_ARGS__)

#endif

