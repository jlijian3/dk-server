
/**
 * author:jlijian3@gmail.com
 * date:2012-03-08
 */

#ifndef __DONKEY_UTIL_INCLUDE__
#define __DONKEY_UTIL_INCLUDE__

#include "dk_common.h"

bool DKGetHostByName(const std::string &host, std::string &ip);
bool DKGetHostName(std::string &host);
bool DKGetHostIp(std::string &ip);

#endif
