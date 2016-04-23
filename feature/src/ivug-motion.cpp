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

#include "ivug-debug.h"
#include "ivug-motion.h"

//#include <ui-gadget.h>
#include <Elementary.h>
#include <sensor.h>

#include <list>
#include <algorithm>

#include <Ecore.h>
using namespace std;

#undef TEST_SENSOR_ERROR

#undef LOG_LVL
#define LOG_LVL DBG_MSG_LVL_HIGH

#undef LOG_CAT
#define LOG_CAT "IV-MOTION"

class CClient
{

private:
	CClient() {
		throw "Use CClient(motion_callback_t func, void *data)";
	};

public:
	CClient(motion_callback_t func, void *data): cb_func(func), cb_data(data) {
	};

	~CClient() {
	};

	void call(motion_handle_t handle, int dx, int dy) {
		if (cb_func) {
			cb_func(handle, dx, dy, cb_data);
		}
	};

	bool operator==(const CClient & value) const {
		if ((value.cb_func == cb_func) && (value.cb_data == cb_data)) {
			return true;
		}

		return false;
	};
private:
	motion_callback_t cb_func;
	void *cb_data;
};

class CComparator
{
public:
	CComparator(const CClient & data): client(data) {
	};

	bool operator()(const CClient * value)const {
		if (client == *value) {
			return true;
		}

		return false;
	};
private:

	const CClient & client;
};

//#define USE_SENSOR_THREAD
class CMotion
{
private:
	static void thSensorStartBlockingTilt(void *data, Ecore_Thread *thread) {
		CMotion *thiz = (CMotion *)data;

		thiz->start_tilt();

		MSG_HIGH("Tilt Sensor start in thread!");
	};

	static void thSensorStartBlockingPanning(void *data, Ecore_Thread *thread) {
		CMotion *thiz = (CMotion *)data;

		thiz->start_panning();

		MSG_HIGH("Panning Sensor start in thread!");
	};

	static void thSensorStartCancel(void *data, Ecore_Thread *thread) {
		MSG_HIGH("Sensor start cancelled!.");
	};

	static void thSensorStartEnd(void *data, Ecore_Thread *thread) {
		MSG_HIGH("Sensor start end!.");
	};

	static void on_motion(unsigned long long timestamp, int x, int y, void *data);

public:
	bool start_tilt() {
		if (bInit_tilt == false) {
			MSG_ERROR("Tilt Sensor already started");
			return false;
		}

//		int ret = -1;

		/*PERF_CHECK_BEGIN(LVL5, "sensor_start");
		ret = sensor_start(mhandle_tilt, SENSOR_MOTION_TILT);
		PERF_CHECK_END(LVL5, "sensor_start");
		if(ret != SENSOR_ERROR_NONE)
		{
			MSG_ERROR("sensor_start SENSOR_MOTION_TILT fail. %d", ret);
			bStarted_tilt = false;
			return false;
		}*/

		bStarted_tilt = true;

		return true;

	}

	bool start_panning() {
		if (bInit_panning == false) {
			MSG_ERROR("Panning Sensor already started");
			return false;
		}

//		int ret = -1;

//		PERF_CHECK_BEGIN(LVL5, "sensor_start");
//		ret = sensor_start(mhandle_panning, SENSOR_MOTION_PANNING_BROWSE);
//		PERF_CHECK_END(LVL5, "sensor_start");
//		if(ret != SENSOR_ERROR_NONE)
//		{
//			MSG_ERROR("sensor_start SENSOR_MOTION_PANNING_BROWSE fail. %d", ret);
//			bStarted_panning = false;
//			return false;
//		}

		bStarted_panning = true;
		return true;

	}

	bool start_async_tilt() {
		m_thread_tilt = ecore_thread_run(thSensorStartBlockingTilt, NULL, NULL, this);
		MSG_WARN("thread func addr = 0x%08x, 0x%08x, 0x%08x", thSensorStartBlockingTilt, thSensorStartEnd, thSensorStartCancel);
		return true;
	}

	bool start_async_panning() {
		m_thread_panning = ecore_thread_run(thSensorStartBlockingPanning, NULL, NULL, this);
		MSG_WARN("thread func addr = 0x%08x, 0x%08x, 0x%08x", thSensorStartBlockingPanning, thSensorStartEnd, thSensorStartCancel);
		return true;
	}

	bool stop_async_tilt() {
		if (m_thread_tilt) {
			ecore_thread_cancel(m_thread_tilt);
			m_thread_tilt = NULL;
		}
		return true;
	}

	bool stop_async_panning() {
		if (m_thread_panning) {
			ecore_thread_cancel(m_thread_panning);
			m_thread_panning = NULL;
		}
		return true;
	}

	bool stop_tilt() {
		if (bInit_tilt == false) {
			MSG_ERROR("Tilt Sensor already stopped");
			return false;
		}

//		int ret = -1;

		/*ret = sensor_stop(mhandle_tilt, SENSOR_MOTION_TILT);
		if(ret != SENSOR_ERROR_NONE)
		{
			MSG_ERROR("sensor_stop SENSOR_MOTION_TILT fail. %d", ret);
			bStarted_tilt = true;
			return false;
		}*/

		bStarted_tilt = false;
		return true;
	}

	bool stop_panning() {
		if (bInit_panning == false) {
			MSG_ERROR("Panning Sensor already stopped");
			return false;
		}

//		int ret = -1;

//		ret = sensor_stop(mhandle_panning, SENSOR_MOTION_PANNING_BROWSE);
//		if(ret != SENSOR_ERROR_NONE)
//		{
//			MSG_ERROR("sensor_stop SENSOR_MOTION_PANNING_BROWSE fail. %d", ret);
//			bStarted_panning = true;
//			return false;
//		}

		bStarted_panning = false;

		return true;
	}

public:
	CMotion() : rotate(0), bStarted_tilt(false), bStarted_panning(false), bInit_tilt(false), bInit_panning(false),
		m_thread_tilt(NULL), m_thread_panning(NULL) {
	};
	virtual ~ CMotion() {
		// it is called after main loop end, because motion var is static
		//deinit_sensor();

		//stop_async();
	};

	bool init_sensor_tilt();
	bool init_sensor_panning();
	bool deinit_sensor_tilt();
	bool deinit_sensor_panning();

	CClient *register_sensor_tilt(motion_callback_t cb_func, void *data) {
		const CClient find(cb_func, data);

		CComparator compare(find);

		list < CClient * >::iterator it =
		    find_if(client_list.begin(), client_list.end(), compare);

		if (it != client_list.end()) {
			MSG_HIGH("Same client is exist.");
			return *it;
			// Founded
		}

		CClient *client = new CClient(cb_func, data);

		client_list.push_back(client);

		if (client_list.size() == 1) {
			MSG_HIGH("Start tilt sensor");
#ifdef USE_SENSOR_THREAD
			PERF_CHECK_BEGIN(LVL5, "sensor_start_async");
			start_async_tilt();
			PERF_CHECK_END(LVL5, "sensor_start_async");
#else
			start_tilt();
#endif
		}

		MSG_HIGH("Add client. Client=%d Handle=0x%08x",
		         client_list.size(), client);

		return client;
	}

	CClient *register_sensor_panning(motion_callback_t cb_func, void *data) {
		const CClient find(cb_func, data);

		CComparator compare(find);

		list < CClient * >::iterator it =
		    find_if(client_list.begin(), client_list.end(), compare);

		if (it != client_list.end()) {
			MSG_HIGH("Same client is exist.");
			return *it;
			// Founded
		}

		CClient *client = new CClient(cb_func, data);

		client_list.push_back(client);

		if (client_list.size() == 1) {
			MSG_HIGH("Start panning sensor");
#ifdef USE_SENSOR_THREAD
			PERF_CHECK_BEGIN(LVL5, "sensor_start_async");
			start_async_panning();
			PERF_CHECK_END(LVL5, "sensor_start_async");
#else
			start_panning();
#endif
		}

		MSG_HIGH("Add client. Client=%d Handle=0x%08x",
		         client_list.size(), client);

		return client;
	}

	void unregister_sensor_tilt(CClient * client) {
		list < CClient * >::iterator it =
		    find(client_list.begin(), client_list.end(), client);

		if (it == client_list.end()) {
			MSG_HIGH("Not found client Handle=0x%08x", client);
			return;
			// Founded
		}

		client_list.remove(client);

		if (client_list.size() == 0) {
			MSG_HIGH("Stop tilt sensor");

			if (stop_tilt() == true) {
				bStarted_tilt = true;
			} else {
				bStarted_tilt = false;
			}
			stop_async_tilt();
		}

		MSG_HIGH("Remove client. Client=%d Handle=0x%08x",
		         client_list.size(), client);

		delete client;
	}

	void unregister_sensor_panning(CClient * client) {
		list < CClient * >::iterator it =
		    find(client_list.begin(), client_list.end(), client);

		if (it == client_list.end()) {
			MSG_HIGH("Not found client Handle=0x%08x", client);
			return;
			// Founded
		}

		client_list.remove(client);

		if (client_list.size() == 0) {
			MSG_HIGH("Stop panning sensor");

			if (stop_panning() == true) {
				bStarted_panning = true;
			} else {
				bStarted_panning = false;
			}
			stop_async_panning();
		}

		MSG_HIGH("Remove client. Client=%d Handle=0x%08x",
		         client_list.size(), client);

		delete client;
	}

	list < CClient * > client_list;

	void swap(int &a, int &b) {
		int tmp;

		tmp = a;
		a = b;
		b = tmp;
	}

	void inform_client(int dx, int dy) {

		if (bStarted_panning == false && bStarted_tilt == false) {
			MSG_HIGH("ignore motion event.");
			return ;
		}

		switch (rotate) {
		case 0:
			dy = -dy;
			break;
		case 90:
			swap(dx, dy);
			break;
		case 180:
			break;
		case 270:
			swap(dx, dy);
			dy = -dy;
			break;
		default:
			break;
		}

		for (list < CClient * >::iterator it = client_list.begin(); it != client_list.end(); it++) {
			(*it)->call(static_cast<motion_handle_t>(*it), dx, dy);
		}

		MSG_HIGH("x:y = [%5d:%5d] rot=%d", dx, dy, rotate);
	};

	void set_rotate(int rotate) {
		this->rotate = rotate;
	};

private:
	sensor_h mhandle_tilt;		// Motion tilt handle
	sensor_h mhandle_panning;		// Motion panning handle

	int rotate;
	bool bStarted_tilt;
	bool bStarted_panning;
	bool bInit_tilt;
	bool bInit_panning;

	Ecore_Thread *m_thread_tilt;
	Ecore_Thread *m_thread_panning;

};

/************************ MOTION SENSOR *************************/

void CMotion::on_motion(unsigned long long timestamp, int x, int y, void *data)
{
	CMotion *thiz = (CMotion *) data;
	IV_ASSERT(data != NULL);

//	int rot = elm_win_rotation_get((Evas_Object *) ug_get_window()); [ToDo] Check Appropriate replacement
	int rot = 90;

	thiz->set_rotate(rot);

	thiz->inform_client(x, y);
}

bool CMotion::init_sensor_tilt()
{
//	int ret = -1;

	if (bInit_tilt == true) {
		MSG_WARN("already sensor is initialized");
		return true;
	}

#ifdef TEST_SENSOR_ERROR
	MSG_ERROR("motion sensor attach fail.");
	return false;
#endif
	MSG_HIGH("init sensor");

	PERF_CHECK_BEGIN(LVL4, "sensor_create");
//	ret = sensor_create(&mhandle_tilt);
//	if(ret != SENSOR_ERROR_NONE)
//	{
//		MSG_ERROR("tilt sensor_create fail. %d", ret);
//		return false;
//	}
//	PERF_CHECK_END(LVL4, "sensor_create");
//
//	PERF_CHECK_BEGIN(LVL4, "sensor_motion_tilt_set_cb");
//	ret = sensor_motion_tilt_set_cb(mhandle_tilt, on_motion, (void *)this);
//	if(ret != SENSOR_ERROR_NONE)
//	{
//		MSG_ERROR("sensor_motion_tilt_set_cb fail. %d", ret);
//		sensor_destroy(mhandle_tilt);
//		return false;
//	}
	PERF_CHECK_END(LVL4, "sensor_motion_tilt_set_cb");

	MSG_HIGH("sensor init succeded");

	bInit_tilt = true;
	return true;

}

bool CMotion::init_sensor_panning()
{
//	int ret = -1;

	if (bInit_panning == true) {
		MSG_WARN("already sensor is initialized");
		return true;
	}

#ifdef TEST_SENSOR_ERROR
	MSG_ERROR("motion sensor attach fail.");
	return false;
#endif
	MSG_HIGH("init sensor");

//	PERF_CHECK_BEGIN(LVL4, "sensor_create");
//	ret = sensor_create(&mhandle_panning);
//	if(ret != SENSOR_ERROR_NONE)
//	{
//		MSG_ERROR("panning sensor_create fail. %d", ret);
//		return false;
//	}
//	PERF_CHECK_END(LVL4, "sensor_create");
//
//	PERF_CHECK_BEGIN(LVL4, "sensor_motion_panning_browse_set_cb");
//	ret = sensor_motion_panning_browse_set_cb(mhandle_panning, on_motion, (void *)this);
//	if(ret != SENSOR_ERROR_NONE)
//	{
//		MSG_ERROR("sensor_motion_panning_set_cb fail. %d", ret);
//		sensor_destroy(mhandle_panning);
//		return false;
//	}
	PERF_CHECK_END(LVL4, "sensor_motion_panning_browse_set_cb");

	MSG_HIGH("sensor init succeded");

	bInit_panning = true;
	return true;
}


bool CMotion::deinit_sensor_tilt()
{
//	int ret = -1;

	if (bInit_tilt == false) {
		return true;
	}

	if (client_list.size() != 0) {
		MSG_HIGH("Client is remain");
		return false;
	}

	bInit_tilt = false;

	MSG_HIGH("deinit sensor tilt");

//	ret = sensor_motion_tilt_unset_cb(mhandle_tilt);
//	if(ret != SENSOR_ERROR_NONE)
//	{
//		MSG_ERROR("sensor_motion_tilt_unset_cb fail. %d", ret);
//	}
//
//	ret = sensor_destroy(mhandle_tilt);
//	if(ret != SENSOR_ERROR_NONE)
//	{
//		MSG_ERROR("tilt sensor_destroy fail. %d", ret);
//	}

	return true;
}

bool CMotion::deinit_sensor_panning()
{
//	int ret = -1;

	if (bInit_panning == false) {
		return true;
	}

	if (client_list.size() != 0) {
		MSG_HIGH("Client is remain");
		return false;
	}

	bInit_panning = false;

	MSG_HIGH("deinit sensor panning");

//	ret = sensor_motion_panning_browse_unset_cb(mhandle_panning);
//	if(ret != SENSOR_ERROR_NONE)
//	{
//		MSG_ERROR("sensor_motion_panning_browse_unset_cb fail. %d", ret);
//	}
//
//	ret = sensor_destroy(mhandle_panning);
//	if(ret != SENSOR_ERROR_NONE)
//	{
//		MSG_ERROR("paning sensor_destroy fail. %d", ret);
//	}

	return true;
}


CMotion motion;

extern "C" motion_handle_t ivug_motion_register_sensor(motion_type_e type, motion_callback_t
        cb_func, void *data)
{
	CClient *client = NULL;

	if (type == IV_MOTION_TILT) {
		motion.init_sensor_tilt();

		PERF_CHECK_BEGIN(LVL4, "register_sensor");
		client = motion.register_sensor_tilt(cb_func, data);
		PERF_CHECK_END(LVL4, "register_sensor");
	} else if (type == IV_MOTION_PANNING) {
		motion.init_sensor_panning();

		PERF_CHECK_BEGIN(LVL4, "register_sensor");
		client = motion.register_sensor_panning(cb_func, data);
		PERF_CHECK_END(LVL4, "register_sensor");
	}

	return (motion_handle_t) client;

}

extern "C" void ivug_motion_unregister_sensor(motion_type_e type, motion_handle_t handle)
{
	CClient *client = (CClient *) handle;

	if (type == IV_MOTION_TILT) {
		motion.unregister_sensor_tilt(client);

		motion.deinit_sensor_tilt();
	} else if (type == IV_MOTION_PANNING) {
		motion.unregister_sensor_panning(client);

		motion.deinit_sensor_panning();
	}
}

