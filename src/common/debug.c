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

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>		// localtime_r
#include <sys/time.h>	// gettimeofday
#include <unistd.h>
#include <string.h>		// strrchr
#include <dlog.h>

#undef ECORE_TIMER

#ifdef ECORE_TIMER
#include <Ecore.h>
#endif

#undef PRINT_TID

#ifdef PRINT_TID
#include <pthread.h>
#endif

#ifndef __G_LIB_H__
	#define G_UNLIKELY(x) x
#endif

/*
	Enable when Do file logging in case of FATAL
*/
#undef IV_FATURE_LOGGING_FATAL

#include "debug.h"

enum {
	_DEBUG_OUTPUT_DIRECTION_CONSOLE = (1 << 0),
	_DEBUG_OUTPUT_DIRECTION_SYSLOG = (1 << 1),
	_DEBUG_OUTPUT_DIRECTION_FILE = (1 << 2),
	_DEBUG_OUTPUT_DIRECTION_DLOG = (1 << 3),
};

// Configuration for output
// console message slower than dlog about 30~40 ms
#define _DEBUG_OUTPUT_DIRECTION (_DEBUG_OUTPUT_DIRECTION_DLOG)

#include <dlog.h>

#ifndef INI_PATH
#define INI_PATH "/var/log"
#endif

#ifndef _DEBUG_MODULE
#define _DEBUG_MODULE "IVUG"
#endif

#define _DEBUG_ERR_TRACE_FILE_NAME	INI_PATH"/"_DEBUG_MODULE"_FATAL.txt"
#define _DEBUG_PREFIX "["_DEBUG_MODULE"]"

static bool bInit = false;

#ifndef ECORE_TIMER
static char *get_time_string(unsigned long dwmSec)
{
	static char buffer[30];

	unsigned long msec=0;
	unsigned long sec=0;
	unsigned long min=0;
	unsigned long hour=0;

// Don't forget turn on compiler optimization options.
	sec = (dwmSec / 1000);
	msec = (dwmSec % 1000);

	min = (sec / 60);
	sec = (sec % 60);

	hour = (min / 60);
	min = (min % 60);

	snprintf(buffer, (size_t)sizeof(buffer), "%1d:%02d:%02d.%03d",	(int)hour,(int)min,(int)sec,(int)msec);

	return buffer;
}


/*Retrieves the number of milliseconds that have elapsed since the system was started*/
unsigned long get_sys_elapsed_time(void)
{
	static struct timeval init_time = { 0 , 0 };
	static bool bFirst = false;
	struct timeval current_time;

	if (bFirst == false)
	{
		bFirst = true;
		gettimeofday(&init_time, NULL);
	}

	gettimeofday(&current_time, NULL);

	// 	return 	(current_time.tv_sec - init_time.tv_sec) * 1000UL + 	(UINT32)((current_time.tv_usec - init_time.tv_usec) / 1000.0)	;
	return 	((current_time.tv_sec * 1E3 + current_time.tv_usec / 1E3) - (init_time.tv_sec * 1E3 + init_time.tv_usec / 1E3));
}
#endif

void _custom_debug_init()
{
#ifdef ECORE_TIMER
	ecore_get_sys_elapsed_time();
#else
	get_sys_elapsed_time();
#endif
}

void _custom_debug_deinit()
{

}

void
_custom_err_trace_write(const char *func_name, int line_num, const char *fmt, ...)
{
	FILE *f    = NULL;
	va_list ap = {0};
	char buf[128];

	time_t current_time;
	struct tm new_time;

	current_time = time(NULL);
	localtime_r(&current_time, &new_time);

	f = fopen(_DEBUG_ERR_TRACE_FILE_NAME, "a");
	if (f == NULL)
	{
//		MyPrintF("Failed to open file.[%s]\n", _DEBUG_ERR_TRACE_FILE_NAME);
		return;
	}

	fprintf(f, "[%.19s][%05d][%s]", asctime_r(&new_time, buf), line_num, func_name);

	va_start(ap, fmt);
	vfprintf(f, fmt, ap);
	va_end(ap);

	fprintf(f, "\n");

	fclose(f);
}

void
_custom_err_trace_fvprintf(const char *func_name, int line_num, const char *fmt, va_list ap)
{
	FILE *f    = NULL;

	time_t current_time;
	struct tm new_time;
	char buf[128];

	current_time = time(NULL);
	localtime_r(&current_time, &new_time);

	f = fopen(_DEBUG_ERR_TRACE_FILE_NAME, "a");
	if (f == NULL)
	{
//		MyPrintF("Failed to open file.[%s]\n", _DEBUG_ERR_TRACE_FILE_NAME);
		return;
	}

	fprintf(f, "[%.19s][[F:%-16.16s L:%5d] ", asctime_r(&new_time, buf), func_name, line_num);
	vfprintf(f, fmt, ap);
	fprintf(f, "\n");

	fclose(f);
}

/*
typedef enum log_priority {
        DLOG_UNKNOWN = 0,
        DLOG_DEFAULT,
        DLOG_VERBOSE,
        DLOG_DEBUG,
        DLOG_INFO,
        DLOG_WARN,
        DLOG_ERROR,
        DLOG_FATAL,
        DLOG_SILENT,
} log_priority;

#define LOG(priority, tag, ...) \
#define LOG_VA(priority, tag, fmt, args) \
*/

inline log_priority convert_to_dlog_priority(int msg_level)
{
/*
DBG_MSG_LOW 	= 0,
DBG_MSG_MED 		= 1,
DBG_MSG_HIGH		= 2,

DBG_MSG_WARN		= 3,
DBG_MSG_ERROR		= 4,
DBG_MSG_FATAL		= 5,

DBG_MSG_CUST5		= 6,
DBG_MSG_CUST6		= 7,
DBG_MSG_CUST7		= 8,
DBG_MSG_CUST8		= 9,
DBG_MSG_CUST9		= 10,
DBG_MSG_CUST10	= 11,
DBG_MSG_CUST11	= 12,
DBG_MSG_CUST12	= 13,
DBG_MSG_CUST13	= 14,
*/

	static const log_priority priority[] = {
		DLOG_WARN,
		DLOG_WARN,
		DLOG_WARN, 		// MSG HIGH
		DLOG_WARN,
		DLOG_ERROR,
		DLOG_FATAL,		// 5
	};

	if (msg_level <= DBG_MSG_FATAL)
		return priority[msg_level];

	if (msg_level == DBG_MSG_CUST12)
		return DLOG_WARN;

	return DLOG_DEBUG;

}



void __custom_debug_msg(debug_msg_type *debug_msg, const char *msg, ...)
{
	va_list va;

	static const char *level ;

	if (G_UNLIKELY(bInit == false))
	{
		_custom_debug_init();
		bInit = true;
	}

#define DIRECORY_SPLITTER '/'
	const char*pFileName = NULL;

#ifdef ECORE_TIMER
	debug_msg->time = ecore_get_sys_elapsed_time();
#else
	debug_msg->time = get_sys_elapsed_time();
#endif

	pFileName  =   strrchr(debug_msg->fname, DIRECORY_SPLITTER);
	pFileName = (NULL == pFileName)?debug_msg->fname:(pFileName+1);

	level = debug_msg->szlevel;

	// File
#ifdef ECORE_TIMER
	char *time_string = ecore_get_time_string(debug_msg->time);
#else
	char *time_string = get_time_string(debug_msg->time);
#endif
	va_start(va, msg);

	if (_DEBUG_OUTPUT_DIRECTION & _DEBUG_OUTPUT_DIRECTION_DLOG)
	{
		char buf[2048];

		int i ;
#ifdef PRINT_TID
		i = snprintf(buf, 2048, "%s[F:%-16.16s L:%5d][%08x][%s] ",time_string , pFileName, debug_msg->nline , pthread_self(), level);
#else
		i = snprintf(buf, 2048, "%s[F:%-16.16s L:%5d][%s] ",time_string , pFileName, debug_msg->nline , level);
#endif
		vsnprintf(buf + i, 2048 - i, msg, va);

// Prevent Format string attack
//		print_log(convert_to_dlog_priority(debug_msg->msg_level), debug_msg->szcategory, "%s", buf);
		dlog_print(DLOG_WARN, LOG_TAG, "%s", buf);
//		print_log(prio, _DEBUG_MODULE, "%s[F:%-16.16s L:%5d][%s:%s] ",time_string , pFileName, debug_msg->nline , szCategory[debug_msg->category], level);
//		vprint_log(prio,_DEBUG_MODULE, msg, va);
	}

	va_end(va);

	if (G_UNLIKELY(debug_msg->msg_level == DBG_MSG_FATAL))
	{
		fflush (stdout);
#ifdef IV_FATURE_LOGGING_FATAL
		va_start(va, msg);
		_custom_err_trace_fvprintf(pFileName, debug_msg->nline, msg, va);		// Save to file.
		va_end(va);
#endif
		assert(0);
	}
}

void __custom_sec_debug_msg(debug_msg_type *debug_msg, const char *msg, ...)
{
	va_list va;

	static const char *level ;

	if (G_UNLIKELY(bInit == false))
	{
		_custom_debug_init();
		bInit = true;
	}

#define DIRECORY_SPLITTER '/'
	const char*pFileName = NULL;

#ifdef ECORE_TIMER
		debug_msg->time = ecore_get_sys_elapsed_time();
#else
		debug_msg->time = get_sys_elapsed_time();
#endif

	pFileName  =   strrchr(debug_msg->fname, DIRECORY_SPLITTER);
	pFileName = (NULL == pFileName)?debug_msg->fname:(pFileName+1);

	level = debug_msg->szlevel;

	// File

#ifdef ECORE_TIMER
		char *time_string = ecore_get_time_string(debug_msg->time);
#else
		char *time_string = get_time_string(debug_msg->time);
#endif

	va_start(va, msg);

	if (_DEBUG_OUTPUT_DIRECTION & _DEBUG_OUTPUT_DIRECTION_DLOG)
	{
		char buf[2048];

		int i ;
#ifdef PRINT_TID
		i = snprintf(buf, 2048, "%s[F:%-16.16s L:%5d][%08x][%s] ",time_string , pFileName, debug_msg->nline , pthread_self(), level);
#else
		i = snprintf(buf, 2048, "%s[F:%-16.16s L:%5d][%s] ",time_string , pFileName, debug_msg->nline , level);
#endif
		vsnprintf(buf + i, 2048 - i, msg, va);

// Prevent Format string attack
//		SECURE_LOG_(LOG_ID_APPS, convert_to_dlog_priority(debug_msg->msg_level), debug_msg->szcategory, "%s", buf);
		dlog_print(DLOG_DEBUG, LOG_TAG, "%s", buf);
//		print_log(prio, _DEBUG_MODULE, "%s[F:%-16.16s L:%5d][%s:%s] ",time_string , pFileName, debug_msg->nline , szCategory[debug_msg->category], level);
//		vprint_log(prio,_DEBUG_MODULE, msg, va);
	}

	va_end(va);

	if (G_UNLIKELY(debug_msg->msg_level == DBG_MSG_FATAL))
	{
		fflush (stdout);
		va_start(va, msg);
		_custom_err_trace_fvprintf(pFileName, debug_msg->nline, msg, va);		// Save to file.
		va_end(va);
		assert(0);
	}

}



#ifdef FMRADIO_FEATURE_ENABLE_GSTREAMER_LOGGING

enum {
	DEBUG_COLOR_DEFAULT 	= 0,
	DEBUG_COLOR_BLACK		= 30,
	DEBUG_COLOR_RED 		= 31,
	DEBUG_COLOR_GREEN		= 32,
	DEBUG_COLOR_YELLOW		= 33,
	DEBUG_COLOR_BLUE		= 34,
	DEBUG_COLOR_MAGENTA 	= 35,
	DEBUG_COLOR_CYAN		= 36,
	DEBUG_COLOR_WHITE		= 37,
};

static gchar *custom_print_object(GObject *object)
{
	if (object == NULL)
	{
		return g_strdup("Unknown");
	}
/*
	if (*(GType *) ptr == GST_TYPE_CAPS)
	{
		return gst_caps_to_string((GstCaps *) ptr);
	}

	if (*(GType *) ptr == GST_TYPE_STRUCTURE)
	{
		return gst_structure_to_string((GstStructure *) ptr);
	}
*/
	if (GST_IS_PAD(object))
	{
		return g_strdup_printf("%s:%s", GST_STR_NULL(GST_OBJECT_NAME(GST_PAD_PARENT(object))) , GST_STR_NULL(GST_PAD_NAME(object)));
	}

	if (GST_IS_ELEMENT(object))
	{
		return g_strdup_printf("%s", GST_STR_NULL(GST_ELEMENT_NAME(object)));
	}

	if (G_IS_OBJECT(object))
	{
		return g_strdup_printf("%s(0x%0x)", G_OBJECT_TYPE_NAME(object), object);
	}

	return g_strdup_printf("0x%08x", object);
}

void custom_log_func(GstDebugCategory *category, GstDebugLevel level,
		const gchar *file, const gchar *function, gint line,
		GObject *object, GstDebugMessage *message, gpointer unused)
{
	static const char *szLevel[] = {"LOW", "MED", "HIGH", "WARN", "ERROR", "FATAL"};

	static const gint levelcolor[] = {
		DEBUG_COLOR_DEFAULT,		/* GST_LEVEL_NONE */
		DEBUG_COLOR_RED,			/* GST_LEVEL_ERROR */
		DEBUG_COLOR_YELLOW, 		/* GST_LEVEL_WARNING */
		DEBUG_COLOR_GREEN,			/* GST_LEVEL_INFO */
		DEBUG_COLOR_CYAN,			/* GST_LEVEL_DEBUG */
		DEBUG_COLOR_WHITE,			/* GST_LEVEL_LOG */
	};

	if (level > gst_debug_category_get_threshold(category))
		return;

	gchar *obj = custom_print_object(object);

#define DIRECORY_SPLITTER '/'

	const char*pFileName = NULL;

	pFileName  =   strrchr(file, DIRECORY_SPLITTER);
	pFileName = (NULL == pFileName) ? file:(pFileName+1);

// File
#ifdef ECORE_TIMER
	char *time_string = ecore_get_time_string(ecore_get_sys_elapsed_time());
#else
	char *time_string = get_time_string(get_sys_elapsed_time());
#endif
	log_print_rel(LOG_CAMCORDER,LOG_CLASS_ERR,
		_DEBUG_PREFIX "%s[F:%-16.16s L:%5d][%s][%s][%s] %s\n", time_string, pFileName, line,
				gst_debug_category_get_name(category), gst_debug_level_get_name(level),obj,
				gst_debug_message_get(message));

	g_free(obj);
}


#endif // FMRADIO_FEATURE_ENABLE_GSTREAMER_LOGGING



