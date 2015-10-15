/*
* Copyright (c) 2000-2015  Samsung Electronics Co., Ltd All Rights Reserved
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#ifndef _IVUG_DEBUG_H_
#define _IVUG_DEBUG_H_

#include <dlog.h>
#include "statistics.h"

#define IVUG_LOG_OUTPUT_DLOG

#define PERF_TIME

#define LVL0 (0)
#define LVL1 (1)
#define LVL2 (2)
#define LVL3 (3)
#define LVL4 (4)
#define LVL5 (5)
#define LVL6 (6)
#define LVL7 (7)

#ifdef PERF_TIME

// accum item handling
#define PERF_CHECK_BEGIN(lvl, name)		iv_ta_accum_item_begin(lvl, name,false,__FILE__,__LINE__)
#define PERF_CHECK_END(lvl, name)		iv_ta_accum_item_end(lvl, name,false,__FILE__,__LINE__)

// Print out
#define PERF_SHOW_RESULT(fp)		iv_ta_accum_show_result_fp(fp)

#else

#define PERF_CHECK_BEGIN(lvl, name)
#define PERF_CHECK_END(lvl, name)

// Print out
#define PERF_SHOW_RESULT(fp)

#endif		// PERF_TIME

enum {
	IVUG_MSG_COLOR_DEFAULT	= 0,
	IVUG_MSG_COLOR_BLACK	= 30,
	IVUG_MSG_COLOR_RED		= 31,
	IVUG_MSG_COLOR_GREEN	= 32,
	IVUG_MSG_COLOR_YELLOW	= 33,
	IVUG_MSG_COLOR_BLUE		= 34,
	IVUG_MSG_COLOR_MAGENTA	= 35,
	IVUG_MSG_COLOR_CYAN		= 36,
	IVUG_MSG_COLOR_WHITE		= 37,
};


#ifdef IVUG_LOG_OUTPUT_DLOG

#undef LOG_TAG
#define LOG_TAG "IV-COMMON"

#define IVUG_DEBUG_MSG(fmt, arg...)			LOGD("[%s : %05d]" fmt "\n", __func__, __LINE__, ##arg)
#define IVUG_DEBUG_WARNING(fmt, arg...)		LOGW("[%s : %05d]" fmt "\n", __func__, __LINE__, ##arg)
#define IVUG_DEBUG_ERROR(fmt, arg...)		LOGE("[%s : %05d]" fmt "\n", __func__, __LINE__ , ##arg)
#define IVUG_DEBUG_CRITICAL(fmt, arg...)	LOGE("[%s : %05d]" fmt "\n", __func__, __LINE__ , ##arg)

#else	//USE_DLOG
#define IVUG_DEBUG_MSG(fmt, arg...)			fprintf(stdout, "[D: %s : %05d]" fmt "\n", __func__, __LINE__, ##arg)
#define IVUG_DEBUG_WARNING(fmt, arg...)		fprintf(stdout, "[W: %s : %05d]" fmt "\n", __func__, __LINE__, ##arg)
#define IVUG_DEBUG_ERROR(fmt, arg...)		fprintf(stdout, "[E: %s : %05d]" fmt "\n", __func__, __LINE__ , ##arg)
#define IVUG_DEBUG_CRITICAL(fmt, arg...)	fprintf(stdout, "[E: %s : %05d]" fmt "\n", __func__, __LINE__ , ##arg)
#define IVUG_DEBUG_PRINT(fmt,arg...)		fprintf(stdout, "[%s : %05d] >>> leave \n", __func__, __LINE__ , ##arg)

#endif //IVUG_LOG_OUTPUT_DLOG

/*
	How to add new Category..

	TODO:
*/

#define MSG_MOUSE_SZCAT "IV-MOUSE"
#define LVL_MOUSE DBG_MSG_LVL_WARN

#define MSG_MOUSE_FATAL(...) 	__MSG_FATAL(LVL_MOUSE, MSG_MOUSE_SZCAT, ##__VA_ARGS__)
#define MSG_MOUSE_ERROR(...)	__MSG_ERROR(LVL_MOUSE, MSG_MOUSE_SZCAT, ##__VA_ARGS__)
#define MSG_MOUSE_WARN(...)		__MSG_WARN(LVL_MOUSE, MSG_MOUSE_SZCAT, ##__VA_ARGS__)
#define MSG_MOUSE_SEC(...)		__MSG_SEC(LVL_MOUSE, MSG_MOUSE_SZCAT, ##__VA_ARGS__)
#define MSG_MOUSE_HIGH(...)		__MSG_HIGH(LVL_MOUSE, MSG_MOUSE_SZCAT, ##__VA_ARGS__)
#define MSG_MOUSE_MED(...)		__MSG_MED(LVL_MOUSE, MSG_MOUSE_SZCAT, ##__VA_ARGS__)
#define MSG_MOUSE_LOW(...)		__MSG_LOW(LVL_MOUSE, MSG_MOUSE_SZCAT, ##__VA_ARGS__)


#define SLIDER_ITEM_SZ "IV-SITEM"
#define LVL_SLIDER_ITEM DBG_MSG_LVL_HIGH


#define MSG_SITEM_FATAL(...) 		__MSG_FATAL(LVL_SLIDER_ITEM, SLIDER_ITEM_SZ, ##__VA_ARGS__)
#define MSG_SITEM_ERROR(...)		__MSG_ERROR(LVL_SLIDER_ITEM, SLIDER_ITEM_SZ, ##__VA_ARGS__)
#define MSG_SITEM_WARN(...)		__MSG_WARN(LVL_SLIDER_ITEM, SLIDER_ITEM_SZ, ##__VA_ARGS__)
#define MSG_SITEM_SEC(...)		__MSG_SEC(LVL_SLIDER_ITEM, SLIDER_ITEM_SZ, ##__VA_ARGS__)
#define MSG_SITEM_HIGH(...)		__MSG_HIGH(LVL_SLIDER_ITEM, SLIDER_ITEM_SZ, ##__VA_ARGS__)
#define MSG_SITEM_MED(...)		__MSG_MED(LVL_SLIDER_ITEM, SLIDER_ITEM_SZ, ##__VA_ARGS__)
#define MSG_SITEM_LOW(...)		__MSG_LOW(LVL_SLIDER_ITEM, SLIDER_ITEM_SZ, ##__VA_ARGS__)


#define SLIDER_SZ "IV-SLIDER"
#define LVL_SLIDER DBG_MSG_LVL_HIGH

#define MSG_SLIDER_FATAL(...) 		__MSG_FATAL(LVL_SLIDER, SLIDER_SZ, ##__VA_ARGS__)
#define MSG_SLIDER_ERROR(...)		__MSG_ERROR(LVL_SLIDER, SLIDER_SZ, ##__VA_ARGS__)
#define MSG_SLIDER_WARN(...)		__MSG_WARN(LVL_SLIDER, SLIDER_SZ, ##__VA_ARGS__)
#define MSG_SLIDER_SEC(...)		__MSG_SEC(LVL_SLIDER, SLIDER_SZ, ##__VA_ARGS__)
#define MSG_SLIDER_HIGH(...)		__MSG_HIGH(LVL_SLIDER, SLIDER_SZ, ##__VA_ARGS__)
#define MSG_SLIDER_MED(...)		__MSG_MED(LVL_SLIDER, SLIDER_SZ, ##__VA_ARGS__)
#define MSG_SLIDER_LOW(...)		__MSG_LOW(LVL_SLIDER, SLIDER_SZ, ##__VA_ARGS__)

#define SETAS_SZ "IV-SETAS"
#define LVL_SETAS DBG_MSG_LVL_HIGH

#define MSG_SETAS_FATAL(...) 		__MSG_FATAL(LVL_SETAS, SETAS_SZ, ##__VA_ARGS__)
#define MSG_SETAS_ERROR(...)		__MSG_ERROR(LVL_SETAS, SETAS_SZ, ##__VA_ARGS__)
#define MSG_SETAS_WARN(...)			__MSG_WARN(LVL_SETAS, SETAS_SZ, ##__VA_ARGS__)
#define MSG_SETAS_SEC(...)			__MSG_SEC(LVL_SETAS, SETAS_SZ, ##__VA_ARGS__)
#define MSG_SETAS_HIGH(...)			__MSG_HIGH(LVL_SETAS, SETAS_SZ, ##__VA_ARGS__)
#define MSG_SETAS_MED(...)			__MSG_MED(LVL_SETAS, SETAS_SZ, ##__VA_ARGS__)
#define MSG_SETAS_LOW(...)			__MSG_LOW(LVL_SETAS, SETAS_SZ, ##__VA_ARGS__)

#define DETAIL_SZ "IV-DETAIL"
#define LVL_DETAIL DBG_MSG_LVL_WARN

#define MSG_DETAIL_FATAL(...) 		__MSG_FATAL(LVL_DETAIL, DETAIL_SZ, ##__VA_ARGS__)
#define MSG_DETAIL_ERROR(...)		__MSG_ERROR(LVL_DETAIL, DETAIL_SZ, ##__VA_ARGS__)
#define MSG_DETAIL_WARN(...)		__MSG_WARN(LVL_DETAIL, DETAIL_SZ, ##__VA_ARGS__)
#define MSG_DETAIL_SEC(...)		__MSG_SEC(LVL_DETAIL, DETAIL_SZ, ##__VA_ARGS__)
#define MSG_DETAIL_HIGH(...)		__MSG_HIGH(LVL_DETAIL, DETAIL_SZ, ##__VA_ARGS__)
#define MSG_DETAIL_MED(...)			__MSG_MED(LVL_DETAIL, DETAIL_SZ, ##__VA_ARGS__)
#define MSG_DETAIL_LOW(...)			__MSG_LOW(LVL_DETAIL, DETAIL_SZ, ##__VA_ARGS__)

#define BEST_SZ "IV-BEST"
#define LVL_BEST DBG_MSG_LVL_HIGH

#define MSG_BEST_FATAL(...) 		__MSG_FATAL(LVL_BEST, BEST_SZ, ##__VA_ARGS__)
#define MSG_BEST_ERROR(...)		__MSG_ERROR(LVL_BEST, BEST_SZ, ##__VA_ARGS__)
#define MSG_BEST_WARN(...)		__MSG_WARN(LVL_BEST, BEST_SZ, ##__VA_ARGS__)
#define MSG_BEST_SEC(...)		__MSG_SEC(LVL_BEST, BEST_SZ, ##__VA_ARGS__)
#define MSG_BEST_HIGH(...)		__MSG_HIGH(LVL_BEST, BEST_SZ, ##__VA_ARGS__)
#define MSG_BEST_MED(...)			__MSG_MED(LVL_BEST, BEST_SZ, ##__VA_ARGS__)
#define MSG_BEST_LOW(...)			__MSG_LOW(LVL_BEST, BEST_SZ, ##__VA_ARGS__)


#define SDATA_SZ "IV-SDATA"
#define LVL_SDATA DBG_MSG_LVL_HIGH

#define MSG_SDATA_FATAL(...) 		__MSG_FATAL(LVL_SDATA, SDATA_SZ, ##__VA_ARGS__)
#define MSG_SDATA_ERROR(...)		__MSG_ERROR(LVL_SDATA, SDATA_SZ, ##__VA_ARGS__)
#define MSG_SDATA_WARN(...)			__MSG_WARN(LVL_SDATA, SDATA_SZ, ##__VA_ARGS__)
#define MSG_SDATA_SEC(...)			__MSG_SEC(LVL_SDATA, SDATA_SZ, ##__VA_ARGS__)
#define MSG_SDATA_HIGH(...)			__MSG_HIGH(LVL_SDATA, SDATA_SZ, ##__VA_ARGS__)
#define MSG_SDATA_MED(...)			__MSG_MED(LVL_SDATA, SDATA_SZ, ##__VA_ARGS__)
#define MSG_SDATA_LOW(...)			__MSG_LOW(LVL_SDATA, SDATA_SZ, ##__VA_ARGS__)


#define IVCOMMON_SZ "IV-COMMON"
#define LVL_IVCOMMON DBG_MSG_LVL_HIGH

#define MSG_IMAGEVIEW_FATAL(...) 		__MSG_FATAL(LVL_IVCOMMON, IVCOMMON_SZ, ##__VA_ARGS__)
#define MSG_IMAGEVIEW_ERROR(...)		__MSG_ERROR(LVL_IVCOMMON, IVCOMMON_SZ, ##__VA_ARGS__)
#define MSG_IMAGEVIEW_WARN(...)			__MSG_WARN(LVL_IVCOMMON, IVCOMMON_SZ, ##__VA_ARGS__)
#define MSG_IMAGEVIEW_SEC(...)			__MSG_SEC(LVL_IVCOMMON, IVCOMMON_SZ, ##__VA_ARGS__)
#define MSG_IMAGEVIEW_HIGH(...)			__MSG_HIGH(LVL_IVCOMMON, IVCOMMON_SZ, ##__VA_ARGS__)
#define MSG_IMAGEVIEW_MED(...)			__MSG_MED(LVL_IVCOMMON, IVCOMMON_SZ, ##__VA_ARGS__)
#define MSG_IMAGEVIEW_LOW(...)			__MSG_LOW(LVL_IVCOMMON, IVCOMMON_SZ, ##__VA_ARGS__)

#define MSG_IVUG_FATAL(...) 			__MSG_FATAL(LVL_IVCOMMON, IVCOMMON_SZ, ##__VA_ARGS__)
#define MSG_IVUG_ERROR(...)				__MSG_ERROR(LVL_IVCOMMON, IVCOMMON_SZ, ##__VA_ARGS__)
#define MSG_IVUG_WARN(...)				__MSG_WARN(LVL_IVCOMMON, IVCOMMON_SZ, ##__VA_ARGS__)
#define MSG_IVUG_SEC(...)				__MSG_SEC(LVL_IVCOMMON, IVCOMMON_SZ, ##__VA_ARGS__)
#define MSG_IVUG_HIGH(...)				__MSG_HIGH(LVL_IVCOMMON, IVCOMMON_SZ, ##__VA_ARGS__)
#define MSG_IVUG_MED(...)				__MSG_MED(LVL_IVCOMMON, IVCOMMON_SZ, ##__VA_ARGS__)
#define MSG_IVUG_LOW(...)				__MSG_LOW(LVL_IVCOMMON, IVCOMMON_SZ, ##__VA_ARGS__)



#define MAINVIEW_SZ "IV-MAIN"
#define LVL_MAINVIEW DBG_MSG_LVL_HIGH

#define MSG_MAIN_FATAL(...) 		__MSG_FATAL(LVL_MAINVIEW, MAINVIEW_SZ, ##__VA_ARGS__)
#define MSG_MAIN_ERROR(...)		__MSG_ERROR(LVL_MAINVIEW, MAINVIEW_SZ, ##__VA_ARGS__)
#define MSG_MAIN_WARN(...)			__MSG_WARN(LVL_MAINVIEW, MAINVIEW_SZ, ##__VA_ARGS__)
#define MSG_MAIN_SEC(...)			__MSG_SEC(LVL_MAINVIEW, MAINVIEW_SZ, ##__VA_ARGS__)
#define MSG_MAIN_HIGH(...)			__MSG_HIGH(LVL_MAINVIEW, MAINVIEW_SZ, ##__VA_ARGS__)
#define MSG_MAIN_MED(...)			__MSG_MED(LVL_MAINVIEW, MAINVIEW_SZ, ##__VA_ARGS__)
#define MSG_MAIN_LOW(...)			__MSG_LOW(LVL_MAINVIEW, MAINVIEW_SZ, ##__VA_ARGS__)

#define UTIL_SZ "IV-UTIL"
#define LVL_UTIL DBG_MSG_LVL_HIGH

#define MSG_UTIL_FATAL(...) 		__MSG_FATAL(LVL_UTIL, UTIL_SZ, ##__VA_ARGS__)
#define MSG_UTIL_ERROR(...)		__MSG_ERROR(LVL_UTIL, UTIL_SZ, ##__VA_ARGS__)
#define MSG_UTIL_WARN(...)			__MSG_WARN(LVL_UTIL, UTIL_SZ, ##__VA_ARGS__)
#define MSG_UTIL_SEC(...)			__MSG_SEC(LVL_UTIL, UTIL_SZ, ##__VA_ARGS__)
#define MSG_UTIL_HIGH(...)			__MSG_HIGH(LVL_UTIL, UTIL_SZ, ##__VA_ARGS__)
#define MSG_UTIL_MED(...)			__MSG_MED(LVL_UTIL, UTIL_SZ, ##__VA_ARGS__)
#define MSG_UTIL_LOW(...)			__MSG_LOW(LVL_UTIL, UTIL_SZ, ##__VA_ARGS__)

#define NOTI_SZ "IV-NOTI"
#define LVL_NOTI DBG_MSG_LVL_WARN

#define MSG_NOTI_FATAL(...) 				__MSG_FATAL(LVL_NOTI, NOTI_SZ, ##__VA_ARGS__)
#define MSG_NOTI_ERROR(...)		__MSG_ERROR(LVL_NOTI, NOTI_SZ, ##__VA_ARGS__)
#define MSG_NOTI_WARN(...)			__MSG_WARN(LVL_NOTI, NOTI_SZ, ##__VA_ARGS__)
#define MSG_NOTI_SEC(...)			__MSG_SEC(LVL_NOTI, NOTI_SZ, ##__VA_ARGS__)
#define MSG_NOTI_HIGH(...)			__MSG_HIGH(LVL_NOTI, NOTI_SZ, ##__VA_ARGS__)
#define MSG_NOTI_MED(...)			__MSG_MED(LVL_NOTI, NOTI_SZ, ##__VA_ARGS__)
#define MSG_NOTI_LOW(...)			__MSG_LOW(LVL_NOTI, NOTI_SZ, ##__VA_ARGS__)

#define EFFECT_SZ "IV-EFFECT"
#define LVL_EFFECT DBG_MSG_LVL_WARN

#define MSG_EFFECT_FATAL(...) 				__MSG_FATAL(LVL_EFFECT, EFFECT_SZ, ##__VA_ARGS__)
#define MSG_EFFECT_ERROR(...)		__MSG_ERROR(LVL_EFFECT, EFFECT_SZ, ##__VA_ARGS__)
#define MSG_EFFECT_WARN(...)			__MSG_WARN(LVL_EFFECT, EFFECT_SZ, ##__VA_ARGS__)
#define MSG_EFFECT_SEC(...)			__MSG_SEC(LVL_EFFECT, EFFECT_SZ, ##__VA_ARGS__)
#define MSG_EFFECT_HIGH(...)			__MSG_HIGH(LVL_EFFECT, EFFECT_SZ, ##__VA_ARGS__)
#define MSG_EFFECT_MED(...)			__MSG_MED(LVL_EFFECT, EFFECT_SZ, ##__VA_ARGS__)
#define MSG_EFFECT_LOW(...)			__MSG_LOW(LVL_EFFECT, EFFECT_SZ, ##__VA_ARGS__)

#define SLIST_ITEM_SZ "IV-SLIST"
#define LVL_SLIST_ITEM DBG_MSG_LVL_WARN

#define MSG_SLIST_FATAL(...) 		__MSG_FATAL(LVL_SLIST_ITEM, SLIST_ITEM_SZ, ##__VA_ARGS__)
#define MSG_SLIST_ERROR(...)		__MSG_ERROR(LVL_SLIST_ITEM, SLIST_ITEM_SZ, ##__VA_ARGS__)
#define MSG_SLIST_WARN(...)		__MSG_WARN(LVL_SLIST_ITEM, SLIST_ITEM_SZ, ##__VA_ARGS__)
#define MSG_SLIST_SEC(...) 		__MSG_SEC(LVL_SLIST_ITEM, SLIST_ITEM_SZ, ##__VA_ARGS__)
#define MSG_SLIST_HIGH(...)		__MSG_HIGH(LVL_SLIST_ITEM, SLIST_ITEM_SZ, ##__VA_ARGS__)
#define MSG_SLIST_MED(...)		__MSG_MED(LVL_SLIST_ITEM, SLIST_ITEM_SZ, ##__VA_ARGS__)
#define MSG_SLIST_LOW(...)		__MSG_LOW(LVL_SLIST_ITEM, SLIST_ITEM_SZ, ##__VA_ARGS__)

#define IV_ASSERT(expr) ASSERT(DBG_MSG_LVL_ALL, "IV-COMMON", expr)

#define MSG_ASSERT(expr) ASSERT(DBG_MSG_LVL_ALL, LOG_CAT, expr)

#define MSG_FATAL(...) 	__MSG_FATAL(LOG_LVL, LOG_CAT, ##__VA_ARGS__)
#define MSG_ERROR(...) 	__MSG_ERROR(LOG_LVL, LOG_CAT, ##__VA_ARGS__)
#define MSG_WARN(...) 	__MSG_WARN(LOG_LVL, LOG_CAT, ##__VA_ARGS__)
#define MSG_SEC(...) 	__MSG_SEC(LOG_LVL, LOG_CAT, ##__VA_ARGS__)
#define MSG_HIGH(...)	__MSG_HIGH(LOG_LVL, LOG_CAT, ##__VA_ARGS__)
#define MSG_MED(...)	__MSG_MED(LOG_LVL, LOG_CAT, ##__VA_ARGS__)
#define MSG_LOW(...)	__MSG_LOW(LOG_LVL, LOG_CAT, ##__VA_ARGS__)
#define MSG_DEBUG(...)	__MSG_DBG(LOG_LVL, LOG_CAT, ##__VA_ARGS__)


//expr check statements.
// Will deprecated

#define IVUG_FUNC_ENTER()					MSG_IVUG_HIGH("ENTER %s(L%d)", __func__, __LINE__)
#define IVUG_FUNC_LEAVE()					MSG_IVUG_HIGH("LEAVE %s(L%d)", __func__, __LINE__)


#define ivug_ret_if(expr) \
	do { \
		if(expr) { \
			MSG_IVUG_ERROR("[%s] Return", #expr );\
			return; \
		} \
	} while (0)

#define ivug_retv_if(expr, val) \
	do { \
		if(expr) { \
			MSG_IVUG_ERROR("[%s] Return value %d", #expr, val );\
			return (val); \
		} \
	} while (0)

#define ivug_retm_if(expr, fmt, args...) \
	do { \
		if(expr) { \
			MSG_IVUG_ERROR("[%s] Return, message "fmt, #expr, ##args );\
			return; \
		} \
	} while (0)

#define ivug_retvm_if(expr, val, fmt, args...) \
	do { \
		if(expr) { \
			MSG_IVUG_ERROR("[%s] Return value, message "fmt, #expr, ##args );\
			return (val); \
		} \
	} while (0)


// Use this instead of ivug_ret_if

#define IV_RET_IF(expr, fmt, args...) \
			do { \
				if(expr) { \
					MSG_IVUG_FATAL("[%s] Return, message "fmt, #expr, ##args );\
					return; \
				} \
			} while (0)

#define IV_RETV_IF(expr, val, fmt, args...) \
			do { \
				if(expr) { \
					MSG_IVUG_FATAL("[%s] Return value, message "fmt, #expr, ##args );\
					return (val); \
				} \
			} while (0)

#include "debug.h"


#endif //_IVUG_DEBUG_H_
