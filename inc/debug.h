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

#ifndef __ELM_MYPHOTOCAM_DEBUG_H__
#define __ELM_MYPHOTOCAM_DEBUG_H__

#include <assert.h>


/*
	Category :
		FATAL
		ERROR
		WARN
		HIGH
		MED
		LOW
		DEBUG
*/


enum
{
	DBG_MSG_LOW    	= 0,
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
};


// level
enum
{
	DBG_MSG_BIT_LOW 		= (1<<DBG_MSG_LOW),
	DBG_MSG_BIT_MED 		= (1<<DBG_MSG_MED),
	DBG_MSG_BIT_HIGH		= (1<<DBG_MSG_HIGH),
	DBG_MSG_BIT_WARN		= (1<<DBG_MSG_WARN),
	DBG_MSG_BIT_ERROR		= (1<<DBG_MSG_ERROR),
	DBG_MSG_BIT_FATAL		= (1<<DBG_MSG_FATAL),

	DBG_MSG_BIT_CUST5		= (1<<DBG_MSG_CUST5),
	DBG_MSG_BIT_CUST6		= (1<<DBG_MSG_CUST6),
	DBG_MSG_BIT_CUST7		= (1<<DBG_MSG_CUST7),
	DBG_MSG_BIT_CUST8		= (1<<DBG_MSG_CUST8),
	DBG_MSG_BIT_CUST9		= (1<<DBG_MSG_CUST9),
	DBG_MSG_BIT_CUST10	= (1<<DBG_MSG_CUST10),
	DBG_MSG_BIT_CUST11	= (1<<DBG_MSG_CUST11),
	DBG_MSG_BIT_CUST12	= (1<<DBG_MSG_CUST12),
	DBG_MSG_BIT_CUST13	= (1<<DBG_MSG_CUST13),
};

#define DBG_MSG_LVL_DEBUG 		(DBG_MSG_BIT_CUST13)
#define DBG_MSG_LVL_SEC 		(DBG_MSG_BIT_CUST12)

#define DBG_MSG_LVL_ALL			(DBG_MSG_LVL_LOW)

#define DBG_MSG_LVL_FATAL		(DBG_MSG_BIT_FATAL | DBG_MSG_BIT_CUST12 )
#define DBG_MSG_LVL_ERROR		(DBG_MSG_LVL_FATAL	| DBG_MSG_BIT_ERROR | DBG_MSG_BIT_CUST12)
#define DBG_MSG_LVL_WARN		(DBG_MSG_LVL_ERROR	| DBG_MSG_BIT_WARN | DBG_MSG_BIT_CUST12 )
#define DBG_MSG_LVL_HIGH		(DBG_MSG_LVL_WARN	| DBG_MSG_BIT_HIGH | DBG_MSG_BIT_CUST12)
#define DBG_MSG_LVL_MED		(DBG_MSG_LVL_HIGH	| DBG_MSG_BIT_MED | DBG_MSG_BIT_CUST12)
#define DBG_MSG_LVL_LOW		(DBG_MSG_LVL_MED	| DBG_MSG_BIT_LOW | DBG_MSG_BIT_CUST12)
#define DBG_MSG_LVL_NONE		(0)

#undef LOG_TAG
#define LOG_TAG "IMAGE_VIEWER"


// Get time of day
#include <sys/time.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

#undef DISABLE_LOG

// Time analyzer
#define DEFINE_PERFORM_TIME(aa) long aa = 0; struct timeval tv

#define BEGIN_PERFORM_TIME(aa) \
	{ \
		gettimeofday(&tv, NULL); \
		aa = tv.tv_sec * 1000 + tv.tv_usec / 1000; \
	} while(0)

#define END_PERFORM_TIME(aa) \
	{  \
		gettimeofday(&tv, NULL);  \
		aa = ( tv.tv_sec * 1000 + tv.tv_usec / 1000 ) - aa; \
	} while(0)

// TODO : Need align(1)
typedef struct {
	const char *fname;
	int nline;
	const char *szcategory;
	int msg_level;
	const char *szlevel;
	unsigned long time;
} debug_msg_type;

void __custom_debug_msg(debug_msg_type *debug_msg, const char *msg, ...);
void __custom_sec_debug_msg(debug_msg_type *debug_msg, const char *msg, ...);

unsigned long get_sys_elapsed_time(void);


#ifdef DISABLE_LOG

#define __MSG_FATAL(level, szCat,  ...)
#define __MSG_ERROR(level, szCat,  ...)
#define __MSG_WARN(level, szCat, ...)
#define __MSG_HIGH(level, szCat, ...)
#define __MSG_MED(level, szCat,  ...)
#define __MSG_LOW(level, szCat,  ...)
#define __MSG_DEBUG(level, szCat,  ...)

#else		// DISABLE_LOG

/* coverity[+kill] */
#define __MSG_FATAL(level, szCat,  ...)	\
	do { \
		if ((level) & DBG_MSG_BIT_FATAL) \
		{ \
			debug_msg_type msg______unique______name___ = {		\
				__FILE__,							\
				__LINE__,							\
				szCat,								\
				DBG_MSG_FATAL,					\
				"FATAL", \
			};									\
			__custom_debug_msg(&msg______unique______name___, ##__VA_ARGS__); \
		} \
	} while(0)

#define __MSG_ERROR(level, szCat,  ...)	\
	do { \
		if ((level) & DBG_MSG_BIT_ERROR)  \
		{ \
			debug_msg_type msg______unique______name___ = {		\
				__FILE__,							\
				__LINE__,							\
				szCat,								\
				DBG_MSG_ERROR,					\
				"ERROR", \
			};									\
			__custom_debug_msg(&msg______unique______name___, ##__VA_ARGS__); \
		} \
	} while(0)


#define __MSG_WARN(level, szCat, ...)	\
	do { \
		if ((level) & DBG_MSG_BIT_WARN)  \
		{ \
			debug_msg_type msg______unique______name___ = {		\
				__FILE__,							\
				__LINE__,							\
				szCat,							\
				DBG_MSG_WARN,					\
				"WARN", \
			};									\
			__custom_debug_msg(&msg______unique______name___, ##__VA_ARGS__); \
		} \
	} while(0)

#define __MSG_SEC(level, szCat,  ...)	\
	do { \
		if ((level) & DBG_MSG_BIT_CUST12)  \
		{ \
			debug_msg_type msg______unique______name___ = { 	\
				__FILE__,							\
				__LINE__,							\
				szCat,								\
				DBG_MSG_CUST12,					\
				"SECURE", \
			};									\
			__custom_sec_debug_msg(&msg______unique______name___, ##__VA_ARGS__); \
		} \
	} while(0)


#define __MSG_HIGH(level, szCat, ...)	\
	do { \
		if ((level) & DBG_MSG_BIT_HIGH) \
		{ \
			debug_msg_type msg______unique______name___ = {		\
				__FILE__,							\
				__LINE__,							\
				szCat,							\
				DBG_MSG_HIGH,					\
				"HIGH", \
			};									\
			__custom_debug_msg(&msg______unique______name___, ##__VA_ARGS__); \
		} \
	} while(0)

#define __MSG_MED(level, szCat,  ...)	\
	do { \
		if ((level) & DBG_MSG_BIT_MED) \
		{ \
			debug_msg_type msg______unique______name___ = {		\
				__FILE__,							\
				__LINE__,							\
				szCat,							\
				DBG_MSG_MED,					\
				"MED", \
			};									\
			__custom_debug_msg(&msg______unique______name___, ##__VA_ARGS__); \
		} \
	} while(0)


#define __MSG_LOW(level, szCat,  ...)	\
	do { \
		if ((level) & DBG_MSG_BIT_LOW) \
		{ \
			debug_msg_type msg______unique______name___ = {		\
				__FILE__,							\
				__LINE__,							\
				szCat,								\
				DBG_MSG_LOW,					\
				"LOW", \
			};									\
			__custom_debug_msg(&msg______unique______name___, ##__VA_ARGS__); \
		} \
	} while(0)

#define __MSG_DBG(level, szCat,  ...)	\
			do { \
				if ((level) & DBG_MSG_BIT_CUST13) \
				{ \
					debug_msg_type msg______unique______name___ = {		\
						__FILE__,							\
						__LINE__,							\
						szCat,								\
						DBG_MSG_BIT_CUST13,					\
						"DBG", \
					};									\
					__custom_debug_msg(&msg______unique______name___, ##__VA_ARGS__); \
				} \
			} while(0)


#endif 		// DISABLE_LOG

#define ASSERT(level, szCat, expr) \
	do { \
		if( !(expr) )					\
		{							\
			__MSG_FATAL(level, szCat, "[%s] ASSERT : " #expr , __func__ );	\
		} \
	} while(0)

#define NEVER_GET_HERE(level, szCat) \
	do { \
		__MSG_FATAL(level, szCat, "NeverGetHere : %s(%d)", __func__, __LINE__);	\
	} while(0)

#ifdef __cplusplus
}
#endif


#endif 		// __ELM_MYPHOTOCAM_DEBUG_H__

