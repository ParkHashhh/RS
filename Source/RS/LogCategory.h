#pragma once


#if !UE_BUILD_SHIPPING
#define RS_LOG_DEBUG(CategoryName, Verbosity, Format, ...) UE_LOG(CategoryName, Verbosity, Format, ##__VA_ARGS__)
#define RS_LOGFMT_DEBUG(CategoryName, Verbosity, Format, ...) UE_LOGFMT(CategoryName, Verbosity, Format, ##__VA_ARGS__)
#else
#define RS_LOG_DEBUG(CategoryName, Verbosity, Format, ...)
#define RS_LOGFMT_DEBUG(CategoryName, Verbosity, Format, ...)
#endif


DECLARE_LOG_CATEGORY_EXTERN(RSAnimation, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(RSGAS, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(RSAI, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(RSNetwork, Log, All);
