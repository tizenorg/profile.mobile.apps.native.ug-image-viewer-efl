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


#include "global.h"

#include "exif.h"
#include <list>
#include <string>
#include <algorithm>

using namespace std;

namespace iv {

class CJPEG {
	enum {
		// JPEG Marker
		M_SOF0	=  0xC0,          // Start Of Frame N
		M_SOF1	=  0xC1,          // N indicates which compression process
		M_SOF2	=  0xC2,          // Only SOF0-SOF2 are now in common use
		M_SOF3	=  0xC3,
		M_SOF5	=  0xC5,         // NB: codes C4 and CC are NOT SOF markers
		M_SOF6	=  0xC6,
		M_SOF7	=  0xC7,
		M_SOF9	=  0xC9,
		M_SOF10	= 0xCA,
		M_SOF11	= 0xCB,
		M_SOF13	= 0xCD,
		M_SOF14	= 0xCE,
		M_SOF15	= 0xCF,

		M_SOI	= 0xD8,          // Start Of Image (beginning of datastream)
		M_EOI  	= 0xD9,          // End Of Image (end of datastream)
		M_SOS  	= 0xDA,          // Start Of Scan (begins compressed data)

		M_APP0	= 0xE0,			// "JFIF\x00", "JFXX\x00"
		M_APP1	= 0xE1,			// "EXIF\x00\x00" or "EXIF\x00\xFF", or XMP("http://ns.adobe.com/xap/1.0/\x00")
		M_APP2	= 0xE2,			// "ICC_PROFILE\x00"
		M_APP3	= 0xE3,			// "META\x00\x00" or "Meta\x00\x00"
		M_APP4	= 0xE4,
		M_APP5	= 0xE5,
		M_APP6	= 0xE6,
		M_APP7	= 0xE7,
		M_APP8	= 0xE8,
		M_APP9	= 0xE9,
		M_APP10	= 0xEA,
		M_APP11	= 0xEB,
		M_APP12	= 0xEC,			// Picture info, or Ducky
		M_APP13	= 0xED,			// "Photoshop 3.0\x00"
		M_APP14	= 0xEE,			// "Adobe\x00"
		M_APP15	= 0xEF,

		M_COM  	= 0xFE,          // COMment
		M_DQT  	= 0xDB,          // Define Quantization Table
		M_DHT  	= 0xC4,          // Define Huffmann Table
		M_DRI  	= 0xDD,
		M_IPTC 	= 0xED,          // IPTC marker
	};

	class CSegment {
		enum {
			eModeUnknown,
			eModeBuffer,
			eModeOffset,
		};

	public:
		CSegment(int type, int len, int offset, string name) : T_(type), L_(len), offset_(offset), buf(NULL), name_(name)  {};

		CSegment(int type, byte *buffer, int len, string name) : T_(type), L_(len), offset_(0), buf(buffer), name_(name) {};
	private:
		int T_;		// JPEG Marker
		int L_;		// Length. only Data length not whole segment

		int offset_;			// Data offset in file base.

// TODO : Who freed buf???
		byte *buf;

		string name_;	// Readable segment name

		void SetData(byte *buffer, int n) {
			L_ = n;
			buf = buffer;
		};

		friend class CJPEG;
	};
public:
	CJPEG() : bModified(false), m_exif(NULL) {};
	~CJPEG() {
		if ( file.IsLoaded() == true )
		{
			// Close previous file.
			file.Close();
		}

		if ( m_exif )
		{
			MSG_WARN("Remove ~CEXIF()");
			delete m_exif;
			m_exif = NULL;
		}
	};

private:
	inline int readLength(const CFile &file) {
		int ta, tb;

		ta = file.GetChar();
		tb = file.GetChar();

		return (ta << 8) | tb;
	};

	int ReadSegment(CFile &file);			// Read JPEG marker segment
	int ReadAppN(CFile &file, int marker, int len);	// Read APPn segment

public:
	bool ParseJPEG(const char *fname) {
		MSG_DEBUG("Parse JPEG - %s", fname);

		if ( file.IsLoaded() == true )
		{
			// Close previous file.
			file.Close();
		}

		if ( m_exif )
		{
			delete m_exif;
			m_exif = NULL;
		}

		if ( file.Open(fname) == true )
		{
			ReadSegment(file);
		}

		return true;
	};

	void PrintInfo() {
		MSG_DEBUG("*****************************************\n");
		for(list<CSegment>::iterator itor = m_seglist.begin(); itor != m_seglist.end(); itor++)
		{
			MSG_DEBUG("%s(0x%02X) Offset=0x%08x Length=%d\n", (*itor).name_.c_str(), (*itor).T_, (*itor).offset_ ,(*itor).L_);
		}
		MSG_DEBUG("*****************************************\n");
	};

	bool WriteJPEG(const char *fname);

// EXIF Infomation getter/setter
public:
	const char *GetUserComment() {
		{
			// Try to find Comment from M_COM
			return m_comment.c_str();
		}
		return NULL;
	};
	void SetUserComment(const char *comment) {
		bModified = true;

		list<CSegment>::iterator litor = m_seglist.begin();		// M_SOI

// Find last APPn
		list<CSegment>::iterator itor;
		for (itor = m_seglist.begin(); itor != m_seglist.end(); itor++)
		{
			CSegment &seg = *itor;

			if ( seg.T_ == M_COM )
			{
				// Found. Modify Segment
				seg.SetData((byte *)comment, strlen(comment));
				break;
			}

			if ( (seg.T_ & 0xF0 ) == 0xE0 )
			{
				// If found APPn, store segment.
				litor = itor;
			}
		}

		if ( itor == m_seglist.end() )
		{
// Insert M_COM after itor.
			m_seglist.insert(litor, CSegment(M_COM, (byte *)comment, strlen(comment), "COM"));
		}

		PrintInfo();

	}

private:
	CFile file;

	bool bModified;		// Is metadata changed?

	list<CSegment> m_seglist;

	string m_comment;		// Comment from m_comment

	string m_comment_new;  	//

// Status
	CExif *m_exif;

};

};

