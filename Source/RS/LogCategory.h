#pragma once

#define LOG_LINE_INFO (FString::Printf(TEXT("[%s:%d]"), TEXT(__FUNCTION__), __LINE__))

#if !UE_BUILD_SHIPPING
#define RS_LOG_DEBUG(CategoryName, Verbosity, Format, ...) UE_LOG(CategoryName, Verbosity, Format, ##__VA_ARGS__)
#define RS_LOGFMT_DEBUG(CategoryName, Verbosity, Format, ...) UE_LOGFMT(CategoryName, Verbosity, Format, ##__VA_ARGS__)
#else
#define RS_LOG_DEBUG(CategoryName, Verbosity, Format, ...)
#define RS_LOGFMT_DEBUG(CategoryName, Verbosity, Format, ...)
#endif


DECLARE_LOG_CATEGORY_EXTERN(LogRSAnimation, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogRSGAS, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogRSAI, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogRSNetwork, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogRSInit, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogRSProperties, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogRSData, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogRSBattle, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogRSItem, Log, All);

