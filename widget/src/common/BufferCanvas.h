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

class CBufferCanvas {
public:
	CBufferCanvas() : m_ee(NULL), m_evas(NULL) {};
	~CBufferCanvas() {
		if ( m_ee)
			ecore_evas_free(m_ee);
	}

	void Create(int w, int h) {
		m_ee = ecore_evas_buffer_new(w,h);
		MSG_ASSERT(m_ee != NULL);

		m_evas = ecore_evas_get(m_ee);

		evas_image_cache_set(m_evas, 0);
	};

	Evas *GetCanvas() const {
		return m_evas;
	};

	const void *GetPixels() const {
		return ecore_evas_buffer_pixels_get(m_ee);
	};

	void Render() const {
		ecore_evas_manual_render(m_ee);
	};


private:

private:
	Ecore_Evas *m_ee;
	Evas *m_evas;


};
