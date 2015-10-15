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

#pragma once

#include <algorithm>

typedef double (*TranstionFunc)(double t);
typedef void (*AnimationCB)(double value, void *data);


class CTranstionFunc {
// Normalied transition fuctions.

// http://www.timotheegroleau.com/Flash/experiments/easing_function_generator.htm

public:
	static double ConstTransit(double t) {
		return 0;
	};

	static double LinearTransit(double t) {
		return t;
	};

	static double SineTransit(double t) {
		static const double SIN_TABLE[91] = {
			0.0000f, 0.0174f, 0.0349f, 0.0523f, 0.0698f,
			0.0872f, 0.1045f, 0.1219f, 0.1392f, 0.1564f,
			0.1736f, 0.1908f, 0.2079f, 0.2249f, 0.2419f,
			0.2588f, 0.2756f, 0.2924f, 0.3090f, 0.3256f,
			0.3420f, 0.3584f, 0.3746f, 0.3907f, 0.4067f,
			0.4226f, 0.4384f, 0.4540f, 0.4695f, 0.4848f,
			0.5000f, 0.5150f, 0.5299f, 0.5446f, 0.5592f,
			0.5736f, 0.5878f, 0.6018f, 0.6157f, 0.6293f,
			0.6528f, 0.6561f, 0.6691f, 0.6820f, 0.6947f,
			0.7071f, 0.7193f, 0.7314f, 0.7431f, 0.7547f,
			0.7660f, 0.7772f, 0.7880f, 0.7986f, 0.8090f,
			0.8191f, 0.8290f, 0.8387f, 0.8480f, 0.8571f,
			0.8660f, 0.8746f, 0.8829f, 0.8910f, 0.8988f,
			0.9063f, 0.9135f, 0.9205f, 0.9272f, 0.9336f,
			0.9397f, 0.9455f, 0.9511f, 0.9563f, 0.9613f,
			0.9659f, 0.9703f, 0.9744f, 0.9781f, 0.9816f,
			0.9848f, 0.9877f, 0.9903f, 0.9926f, 0.9945f,
			0.9962f, 0.9976f, 0.9986f, 0.9994f, 0.9998f,
			1.0f
		};

		int idx = (int)(90.0 * t);

		return (SIN_TABLE[idx]);
	};

	static double EaseInoutTransit(double t) {
		double ts = t * t;
		double tc = ts * t;
		return (6 * tc * ts -15 * ts * ts  + 10 * tc);
	}

	static double EaseoutQuinticTransit(double t) {
		double ts = t * t;
		double tc = ts * t;
		return (tc * ts  - 5 * ts * ts + 10 * tc - 10 * ts + 5 * t);
	}

	static double EaseoutCubicTransit(double t) {
		double ts = t * t;
		double tc = ts * t;
		return (tc + -3*ts + 3*t);
	}

};


class CTransit {
public:
	CTransit(double first, double last, double duration , TranstionFunc transition) : m_duration(duration), m_current(0), m_start(first), m_last(last),
		m_transition(transition) {
	};


	CTransit() : m_duration(1.0f), m_current(0), m_transition(CTranstionFunc::ConstTransit) {
	};

	~CTransit() {
	};

	void SetTransitFunction(TranstionFunc transition ) {
		m_transition = transition;
		m_current = 0.0f;
	};

	void SetDuration(double duration)		// X variations(time)
	{
		m_duration = m_duration;
		m_current = 0.0f;
	}

	void SetInit(double first)	// Y variationns
	{
		m_start = first;
		m_current = 0.0f;
	}

	void SetEnd(double last)	// Y variationns
	{
		m_last = last;
		m_current = 0.0f;
	}

	double Update(double dt) {
		double reminant = std::min(m_duration - m_current, dt );

		m_current += reminant;

		return m_start + m_transition( m_current/m_duration ) * ( m_last - m_start);

	};

	bool IsFinished() {
		return m_current >= m_duration;
	};

private:
	double m_duration;
	double m_current;

	double m_start;
	double m_last;

	TranstionFunc m_transition;
};



class CActor {
public:
	CActor() {
		m_tween = new CTransit();
	};
// TODO : Using functor!
	CActor(double first, double last, double duration, TranstionFunc transition, AnimationCB callback, void *data) : m_callback(callback), m_data(data) {
		m_tween = new CTransit(first, last, duration, transition);
	};

	~CActor() {
		delete m_tween;
		m_tween = NULL;
	};

	void SetCallback(AnimationCB callback, void *data) {
		m_callback = callback;
		m_data = data;
	};

	void SetConfig(double first, double last, double duration, TranstionFunc transition)
	{
		m_tween->SetTransitFunction(transition);
		m_tween->SetInit(first);
		m_tween->SetEnd(last);
		m_tween->SetDuration(duration);
	}

	void Update(double delta_t)
	{
		double value = m_tween->Update(delta_t);

		m_callback(value, m_data);
	}

	bool IsFinished() const { return m_tween->IsFinished(); };
private:
	AnimationCB m_callback;
	void *m_data;

	CTransit *m_tween;
};

