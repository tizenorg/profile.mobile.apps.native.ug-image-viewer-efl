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

#include <string>
#include <list>

#define TIFF_TAG_IMAGEWIDTH					(0x0100)
#define TIFF_TAG_IMAGELENGTH				(0x0101)
#define TIFF_TAG_IMAGDESCRIPTION			(0x010E)
#define TIFF_TAG_ORIENTATION				(0x0112)
#define TIFF_TAG_MAKE						(0x010F)

#define TIFF_TAG_MODEL						(0x0110)
#define TIFF_TAG_SOFTWARE					(0x0131)

#define TAG_EXIF_IFD						(0x8769)
#define TAG_GPS_IFD							(0x8825)
#define TAG_INTEROPERABILITY_IFD			(0xA005)

/*
	Table 4 in specification
*/
#define EXIF_IFD_TAG_VERSION				(0x9000)
#define EXIF_IFD_TAG_FLASHPIXVER			(0xA000)
#define EXIF_IFD_TAG_COLORSPACE				(0xA001)
#define EXIF_IFD_TAG_COMPONENT_CONFIG		(0x9101)
#define EXIF_IFD_TAG_COMPRESSED_BPP			(0x9102)
#define EXIF_IFD_TAG_PIXEL_XDIM				(0xA002)
#define EXIF_IFD_TAG_PIXEL_YDIM				(0xA003)
#define EXIF_IFD_TAG_MAKER_NOTE				(0x927C)
#define EXIF_IFD_TAG_USER_COMMENT			(0x9286)
#define EXIF_IFD_TAG_RELATED_SOUND_FILE		(0xA004)
#define EXIF_IFD_TAG_DATATIME_ORIGINAL		(0x9003)
#define EXIF_IFD_TAG_DATATIME_DIGITIZED		(0x9004)
#define EXIF_IFD_TAG_SUBSECTIME				(0x9290)
#define EXIF_IFD_TAG_SUBSECTIME_ORIGNAL		(0x9291)
#define EXIF_IFD_TAG_SUBSECTIME_DIGITIZED	(0x9292)
#define EXIF_IFD_TAG_UNIQUE_IMAGE_ID		(0xA420)

/*
	Table 5 in specification
*/
#define EXIF_IFD_TAG_EXPOSURE_TIME			(0x829A)
#define EXIF_IFD_TAG_FNUMBER				(0x829D)

namespace iv {

class CExif {
	const char *GetTagName(int tag);

	class CExifTag
	{
	public:
		CExifTag(short tag, short type, int count, int voffset)
		{
		   m_tag = tag;
		   m_type = type;
		   m_count = count;
		   m_voffset = voffset;
		}

	private:
		short m_tag;
		short m_type;
		int m_count;
		int m_voffset;
	};

	class CIFD {
		CIFD() : count(0) {};
		~CIFD() {};
	public:

	private:
		int count;		// Entry count

		byte *data;
		int len;
	};

	inline int GetBytes(int T) {
		static char Tsize[] = { 1 /* Inavalid */, 1, 1, 2, 4, 8, 1, 1, 2, 4, 8, 4, 8 };

		return Tsize[T];
	};

private:
	int ParseUSerComment(byte *buf, int len)
	{
		// ID(8) | Comment(Any)
		m_usercomment.assign((const char *)buf + 8, len - 8);
		return 0;
	}

	int Get16(byte *buffer) {
		if ( blendian == true )
		{
			return buffer[0] | (buffer[1] << 8);
		}

		return buffer[1] | (buffer[0] << 8);
	};

	int Get32(byte *buffer) {
		if ( blendian == true )
		{
			return buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
		}

		return (buffer[0]  << 24) | (buffer[1]  << 16) | (buffer[2]  << 8) | buffer[3];
	};

/*
	| IFD DATA | Value of IFD  |
	|----------------------|
	buffer                              len

	We cannot get end offset of 'Value of IFD'
*/
	int ParseIFD(byte *buffer, int len, byte *offsetbase, int lvl);

public:
	CExif() {
		verbose = false;
	};

	bool Parse(byte *buffer, int len, int offset);

	const char *GetUserComment() {
		return m_usercomment.c_str();
	};

private:
	std::string m_usercomment;

	bool blendian;		// Is little endinan?

	long header_offset;
	byte *header;

// For inplace edit.
	bool bFoundExifIFD;
	bool bFoundUserComment;

	struct {
		long offset;
		int len;
		int adjust;		// Adjusted length.
	} UserComment;

	bool verbose;
// IDF list
	std::list<CIFD *> m_ifdlist;

};

};

