
/**
 * author:lijian2@ucweb.com
 * date:2012-03-08
 */

#ifndef __DONKEY_UTIL_INCLUDE__
#define __DONKEY_UTIL_INCLUDE__

#include "donkey_common.h"

bool DonkeyGetHostByName(const std::string &host, std::string &ip);
bool DonkeyGetHostName(std::string &host);
bool DonkeyGetHostIp(std::string &ip);

#endif
