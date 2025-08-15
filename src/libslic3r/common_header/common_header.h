#ifndef _COMMON_HEADER_H_
#define _COMMON_HEADER_H_
#include <string>

#include "buildinfo.h"

#if defined(CUSTOMIZED) && !defined(CUSTOM_CXCLOUD_ENABLED)
	#define CUSTOM_CXCLOUD false
	#define CUSTOM_COMMUNITY_ENABLE false
#else
	#define CUSTOM_CXCLOUD true
	#define CUSTOM_COMMUNITY_ENABLE true
#endif



#pragma execution_character_set("utf-8")
namespace Slic3r {
namespace CxBuildInfo {
static bool       g_is_dark = true;

void              setDarkMode(bool isDark);
const std::string getVersion();
const std::string getSlic3rVersion();
const std::string getBuildTime();
const std::string getBuildId();
const std::string getBuildOs();

const std::string getBuildType();
const std::string getProjectName();

const bool isCusotmized();
const bool isEnabledCxCloud();

const std::string getIconName_noTheme();
const std::string getIconName();
}	//namespace CxBuildInfo
} // namespace Slic3r
#endif // _COMMON_HEADER_H_