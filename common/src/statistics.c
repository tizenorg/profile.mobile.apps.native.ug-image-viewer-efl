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

#include <sys/time.h>	// gettimeofday

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "statistics.h"
#include "debug.h"


#ifdef STANDALONE
#define EXPORT_API
#endif

#include <sys/time.h>
#include <sys/utsname.h>
#include <sys/resource.h>
#include <unistd.h>

#include <stdarg.h>
#include "statistics.h"

#define MAX_UINT32 (0xFFFFFFFFL)
#define MAX_UINT64 (0xFFFFFFFFFFFFFFFFLL)

// defs.
#define MM_TA_MAX_ACCUM			100


typedef struct _iv_ta_accum_item
{
	unsigned long long elapsed_accum;
	unsigned long num_calls;
	unsigned long long elapsed_min;
	unsigned long long elapsed_max;
	unsigned long long first_start;
	unsigned long long last_end;

	char* name;
	int lvl;

	unsigned long long timestamp;
	int on_estimate;
	int num_unpair;
} iv_ta_accum_item;


static void PrintLog(const char *file, int line, const char *msg, ...)
{
	va_list va;

	va_start(va, msg);
	fprintf(stderr ,"[STAT] %s:%d:",file, line);
	vfprintf(stderr ,msg, va);
	fprintf(stderr, "\n");
	va_end(va);
}

#define MyPrintf(...)  PrintLog(__FILE__, __LINE__, ##__VA_ARGS__)


// internal func.
static void __free_accums(void);
static int __get_accum_index(int lvl, const char* name);


// global var.
static iv_ta_accum_item ** g_accums = NULL;
static int g_accum_index = 0;
static int g_accum_longest_name = 0;
static unsigned long long g_accum_first_time = MAX_UINT64;	// jmlee


int IV_PERF_INIT(void)
{
	if (g_accums)
	{
		return 0;
	}

	g_accums = (iv_ta_accum_item **) malloc (MM_TA_MAX_ACCUM * sizeof(iv_ta_accum_item *));
	if (!g_accums)
	{
		assert(0);
		return -1;
	}

	g_accum_first_time = MAX_UINT64;
	g_accum_index = 0;
	g_accum_longest_name = 0;

	return 0;
}

int IV_PERF_DEINIT(void)
{
	if (! g_accums)
	{
		return 0;
	}

	__free_accums();

	g_accum_first_time = MAX_UINT64;

	return 0;
}


static int __get_accum_index(int lvl, const char* name)
{
	int i;

	assert(name);

	// find index
	for (i = 0; i < g_accum_index; i++)
	{
		if ((lvl == g_accums[i]->lvl) && (strcmp(name, g_accums[i]->name) == 0))
			return i;
	}

	return -1;
}

static void __free_accums(void)
{
	int i = 0;

	if (! g_accums)
		return;

	for (i = 0; i < g_accum_index; i++)
	{
		if (g_accums[i])
		{
			if (g_accums[i]->name)
				free (g_accums[i]->name);

			free (g_accums[i]);

			g_accums[i] = NULL;
		}
	}

	g_accum_index = 0;
	g_accum_longest_name = 0;

	free (g_accums);
	g_accums = NULL;
}



int iv_ta_accum_item_begin(int lvl, const char* name, bool show, const char* filename, int line)
{
	iv_ta_accum_item * accum = NULL;
	int index = 0;
	int name_len = 0;
	struct timeval t;

	if (!g_accums)
		return 0;

	if (g_accum_index == MM_TA_MAX_ACCUM)
		return -1;

	if (!name)
		return -1;

	name_len = strlen(name) + (lvl * 2);
	if (name_len == 0)
		return -1;

	// if 'name' is new one. create new item.
	if ((index = __get_accum_index(lvl, name)) == -1)
	{
		accum = (iv_ta_accum_item *) malloc(sizeof(iv_ta_accum_item));
		if (!accum)
		{
			assert(0);
			return -1;
		}

		// clear first.
		memset(accum, 0, sizeof (iv_ta_accum_item));
		accum->elapsed_min = MAX_UINT64;

		accum->name = strdup(name);
		accum->lvl = lvl;
		// add it to list.
		g_accums[g_accum_index] = accum;
		g_accum_index++;

		if (g_accum_longest_name < name_len)
			g_accum_longest_name = name_len;

	}
	else
	{
		accum = g_accums[index];
	}

	// verify pairs of begin, end.
	if (accum->on_estimate)
	{
		MyPrintf("[%s] is not 'end'ed!\n", accum->name);
		accum->num_unpair ++;
		return -1;
	}

	accum->on_estimate = 1;

	// get timestamp
	gettimeofday(&t, NULL);
	accum->timestamp = t.tv_sec * 1000000ULL + t.tv_usec;

	if (accum->first_start == 0)
	{	// assum that timestamp never could be zero.
		accum->first_start = accum->timestamp;

		if (g_accum_first_time > accum->first_start)
		{
			g_accum_first_time = accum->first_start ;
		}
	}

	if (show)
		MyPrintf("[ACCUM BEGIN] %s : %ld ---(%s:%d)\n", name, accum->timestamp, filename, line);

	accum->num_calls++;

	return 0;
}

int iv_ta_accum_item_end(int lvl, const char* name, bool show, const char* filename, int line)
{
	iv_ta_accum_item * accum = NULL;
	unsigned long long tval = 0LL;
	int index = 0;
	struct timeval	t;

	if (!g_accums)
		return 0;

	if (g_accum_index == MM_TA_MAX_ACCUM)
		return -1;

	if (!name)
		return -1;

	if (strlen (name) == 0)
		return -1;

	// varify the 'name' is already exist.
	if ((index = __get_accum_index(lvl, name)) == -1)
	{
		MyPrintf("[%s] is not added before!\n", name);
		return -1;
	}

	accum = g_accums[index];

	// verify pairs of begin, end.
	if (!accum->on_estimate)
	{
		MyPrintf("[%s] is not 'begin' yet!\n", accum->name);
		accum->num_unpair ++;
		return -1;
	}

	// get time first for more accuracy.
	gettimeofday(&t, NULL);
	tval = t.tv_sec*1000000ULL + t.tv_usec;

	// update last_end
	accum->last_end = tval;

	// make get elapsed time.
	tval = tval - accum->timestamp;

	// update min/max
	accum->elapsed_max = tval > accum->elapsed_max ? tval : accum->elapsed_max;
	accum->elapsed_min = tval < accum->elapsed_min ? tval : accum->elapsed_min;

	if (show)
		MyPrintf("[ACCUM END] %s : %llu + %llu ---(%s:%d)\n", name, accum->elapsed_accum, tval, filename, line);

	// add elapsed time
	accum->elapsed_accum = accum->elapsed_accum + tval;
	accum->on_estimate = 0;

	return 0;
}

void __print_some_info(FILE* fp)
{
	// General infomation
	{
		time_t t_val;
		char hostname[256] = {'\0',};
#ifdef LINUX
		struct utsname uts;
		struct rusage r_usage;
#endif
		fprintf(fp, "\n[[ General info ]]\n");

		// time and date
		time(&t_val);
		fprintf(fp, "Date : %s", ctime(&t_val));

		// system
		if (gethostname(hostname, 255) == 0)
		{
			fprintf(fp, "Hostname : %s\n", hostname);
		}
#ifdef LINUX
		if (uname(&uts) >= 0)
		{
			fprintf(fp, "System : %s\n", uts.sysname);
			fprintf(fp, "Machine : %s\n", uts.machine);
			fprintf(fp, "Nodename : %s\n", uts.nodename);
			fprintf(fp, "Release : %s \n", uts.release);
			fprintf(fp, "Version : %s \n", uts.version);
		}

		// process info.
		fprintf(fp, "Process priority : %d\n", getpriority(PRIO_PROCESS, getpid()));

		getrusage(RUSAGE_SELF, &r_usage);
		fprintf(fp, "CPU usage : User = %ld.%06ld, System = %ld.%06ld\n",
				r_usage.ru_utime.tv_sec, r_usage.ru_utime.tv_usec,
				r_usage.ru_stime.tv_sec, r_usage.ru_stime.tv_usec);
#endif

	}

	// host environment variables
	{
		extern char** environ;
		char** env = environ;

		fprintf(fp, "\n[[ Host environment variables ]]\n");
		while (*env)
		{
			fprintf(fp, "%s\n", *env);
			env++;
		}
	}

	fprintf(fp, "g_accum_first_time = %llu", g_accum_first_time);

	fprintf(fp, "\n\n");

}


void iv_ta_accum_show_result_fp(FILE *fp)
{
	int i = 0;
	char format[256] = {0,};

	__print_some_info(fp);

	fprintf(fp, "\n\n");

	fprintf(fp, "============================ BEGIN RESULT ACCUM (usec) ====================\n");

	snprintf(format, (size_t)sizeof(format), "[Idx] [Lvl] %%-%ds %%10s %%10s %%6s %%10s %%10s %%4s \n", g_accum_longest_name);

	fprintf(fp, format, "Name", "avg", "total", "hit", "min", "max", "pair");

	snprintf(format, (size_t)sizeof(format), "[%%3d] [%%3d] %%-%ds %%10llu %%10llu %%6lu %%10llu %%10llu %%4s \n", g_accum_longest_name);

	for (i = 0; i < g_accum_index; i++)
	{
		// prevent 'devide by zero' error
		if (g_accums[i]->num_calls == 0)
			g_accums[i]->num_calls = 1;

		char space[256] = {0,};

		int j;
		for (j = 0; j < g_accums[i]->lvl * 2; j++)
		{
			space[j] = '_';
		}
		space[g_accums[i]->lvl * 2] = '\0';

		if (strlen(g_accums[i]->name) < 256) {
			fprintf(fp,
				format,
				i,
				g_accums[i]->lvl,
				strncat(space, g_accums[i]->name, strlen(g_accums[i]->name)),
				(g_accums[i]->elapsed_accum == 0) ? 0 : (g_accums[i]->elapsed_accum / g_accums[i]->num_calls),  // Fix it! : devide by zero.
				g_accums[i]->elapsed_accum,
				g_accums[i]->num_calls,
				g_accums[i]->elapsed_min,
				g_accums[i]->elapsed_max,
				g_accums[i]->num_unpair == 1 ? "F" : "T");
		}
	}

	fprintf(fp, "============================ END RESULT ACCUM  ============================\n");

}

#ifdef STANDALONE
int main(int argc, char* argv[])
{
	int a = 0, b = 0;


	PERF_CHECK_BEGIN(LVL1, "Test 1");

	for (a = 0 ; a < 10; a++)
	{
		MyPrintF("AAA=%d\n", a);
		usleep(1*10E6);
	}

	PERF_CHECK_END(LVL1, "Test 1");

	MyPrintF("Test 111\n");
	return 0;
}
#endif



