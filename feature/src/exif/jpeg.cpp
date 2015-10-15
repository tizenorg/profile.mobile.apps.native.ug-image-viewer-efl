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

/*
Orientation.
	Load
		JFIF format --> 0

		EXIF format -> Check 0th IFD
			if found orientation : return value.
			if not : return 0.

// Impossible because MakerNote offset
//	Save
//		JFIF format.
//			Write FFD8
//			Create TIFF Header. Make 0th IFD.
//			Save SOI ~ End
//
//		EXIF format
//			if found : just replace value itself.
//			not found : add interopebility and resize 0th IFD. Adjust all offset.
//


Comment.
	Load
		JFIF format --> check JFIF comment

		EXIF format --> Find UserComment
			if found
				use this value.
			else if not found
				find JFIF comment
			else
				No Comment

// Impossible because MakerNote offset
//	Save
//		JFIF format
//			Create basic EXIF header & User comment
//
//		EXIF format.
//			Offset Modified : Next IFD, StripOffset, GPS IFD Pointer

*/

#include <string.h>	// memcmp
#include <memory.h>
#include <stdlib.h>

#include "File.h"
#include "exif.h"
#include "jpeg.h"

#include <list>
#include <string>

using namespace std;
using namespace iv;


int CJPEG::ReadAppN(CFile &file, int marker, int len)
{
	int size;

	size = len;

	MSG_DEBUG("marker=0x%02x Size=%d", marker, size);

	switch(marker)
	{
	case M_APP0:		// JFIF or JFXX
		{
			byte *buffer;

			buffer = (byte *)malloc(size);

			if (buffer == NULL) return -1;

			if (file.Read(buffer, size) != size )
			{
				MSG_ERROR("Cannot read file. size=%u", size);
				free(buffer);
				return -1;
			}

			if (memcmp(buffer, "JFIF", sizeof("JFIF")) == 0 )
			{
				int vMajor = buffer[5];
				int vMinor = buffer[6];

				int unit = buffer[7];
				int Xdensity = buffer[8] << 8 | buffer[9];
				int Ydensity = buffer[10] << 8 | buffer[11];

				int Xthumbnail = buffer[12];
				int Ythumbnail = buffer[13];

				MSG_DEBUG("JFIF v%1d.%02d Unit=%d XYDensity(%d,%d), XYThumbnail(%d,%d)", vMajor, vMinor, unit, Xdensity, Ydensity, Xthumbnail, Ythumbnail);
			}
			else if (memcmp(buffer, "JFXX", sizeof("JFXX")) == 0 )
			{
				MSG_DEBUG("JFXX");
			}
			else
			{
				MSG_DEBUG("Invalid APP0");
			}
			free(buffer);
		}

		break;
	case M_APP1:		// EXIF or XMP
		{
			// Read 6 bytes.
			char header[6];
			int bytes;

			bytes = file.Read(header, 6);

			if (bytes < 6 )
			{
				MSG_ERROR("Not a vaild EXIF");
				return -1;
			}

			int read;

			if (memcmp(header, "Exif\00", sizeof("Exif\00") - 1) == 0 )
			{
				if (m_exif != NULL)
				{
					// There is jpeg has 2 Exifs segment. we ignore that.
					MSG_WARN("M_APP1 appears two times. Ignore this");
					break;
				}

				int offset = file.Tell();

				byte *buffer = (byte *)malloc(size);

				if (buffer == NULL) return -1;

				read = file.Read(buffer, size);

				if (read != size )
				{
					MSG_ERROR("Try to read %d bytes but %d bytes read", size, read);
					free(buffer);
					break;
				}

				m_exif = new CExif;

				if (m_exif->Parse(buffer, size, offset) == false )
				{
					MSG_ERROR("EXIF Parse Error. Size=%d bytes offset=%d",size, offset );
				}

				free(buffer);
			}
			else if (memcmp(header, "http:", sizeof("http:") -1 ) == 0 )
			{
				MSG_WARN("XMP parser is not implemented");
				// XMP
			}

		}

		break;
	default:
		MSG_DEBUG("Unknown marker(0x%02X)", marker);
		break;
	}

	return 0;
}

int CJPEG::ReadSegment(CFile &file)
{
// SOI - APP1 - (APP2) - DQT - DHT - (DRI) - SOF - SOS - ... - EOI - Sound&Image Data
	int f = file.GetChar();

	if (f != 0xFF || file.GetChar() != M_SOI)
	{
		MSG_DEBUG("Not a valid jpeg file");
		return -1;
	}

	m_seglist.push_back(CSegment(M_SOI, 0, 0, "SOI") );

	int marker;
	int len;

	for(int a = file.GetChar(); a == 0xFF; a = file.GetChar())
	{
		marker = file.GetChar();

//			MSG_DEBUG("Found Segment 0x%02X", marker);
		len = readLength(file) - 2;		// JPEG Len include 2byte for size itself

		if ((marker & 0xF0 ) == 0xE0 )
		{
			char buf[10];

			snprintf(buf, 10, "APP%d", marker & 0x0F);

			int offset = file.Tell();

			MSG_DEBUG("Found %s(0x%02X) - Off(%d) Len(%d)", buf, marker, offset, len);

			m_seglist.push_back(CSegment(marker, len, offset, buf ));

//  TODO : Read whole section.
			if (marker == M_APP0 || marker == M_APP1)
			{
				if (ReadAppN(file, marker, len) < 0 )
				{
					MSG_ERROR("Parse Error. but ignore. %s", file.GetFilename());
				}
			}

			if (offset > -1) {
				file.Seek(offset, CFile::eSet);		// Reset
			}
			file.Seek(len, CFile::eCur);		// skip to next segment. - 2 is segment header
			continue;
		}

		switch(marker) {
		case M_DHT:
			m_seglist.push_back(CSegment(marker, len, file.Tell(),"DHT") );
			break;
		case M_DQT:
			m_seglist.push_back(CSegment(marker, len, file.Tell(),"DQT") );
			break;

		case M_SOS:
			m_seglist.push_back(CSegment(marker, len, file.Tell(),"SOS") );
			break;

		case M_SOF0:
			m_seglist.push_back(CSegment(marker, len, file.Tell(),"SOSF0") );
			break;

		case M_EOI:
			// Never reached here. because we cannot get compressed data len.
			m_seglist.push_back(CSegment(marker, len, file.Tell(),"EOI") );
			break;

		case M_DRI:
			m_seglist.push_back(CSegment(marker, len, file.Tell(),"DRI") );
			break;

		case M_COM:
			{
				// Read Comment & Store.
				char *buf = (char *)malloc(len);
				if (buf == NULL)
					break;
				int offset = file.Tell();

				if (buf != NULL && offset > 0) {
					file.Read(buf, len);

					file.Seek(offset, CFile::eSet);

					m_comment.assign(buf, len);

					MSG_HIGH("Found Comment in JPEG. %s", m_comment.c_str());

					m_seglist.push_back(CSegment(marker, len, file.Tell(),"COM") );
				}
				free(buf);
			}
			break;
		default:
			MSG_WARN("Unhandled Marker=0x%02X", marker);
			m_seglist.push_back(CSegment(marker, len, file.Tell(),"NOTPARSED") );
			break;
		};

		file.Seek(len, CFile::eCur);		// - 2 is segment header len
	}

// Save remainder of file.
	long offset = file.Tell() - 1;		// -1 is GetChar() in for statement.

	size_t size = file.GetSize();

	m_seglist.push_back(CSegment(0xFF, size - offset, offset, "DATA") );

	return 0;
}


bool CJPEG::WriteJPEG(const char *fname) {

	if (bModified == false )
	{
		MSG_ERROR("Jpeg does not changed. ");
		return false;
	}

	MSG_DEBUG("Write to file. %s", fname);
	string dfile;
	string sfile;

	bool bSamedest = false;

// Make tmp and Rename
	dfile = fname;

	if (strcmp( file.GetFilename(), fname ) == 0 )
	{
		MSG_ERROR("Source file & Dst file name are same.");

		bSamedest = true;
		dfile += ".tmp";
	}

	sfile = file.GetFilename();

	CFile dst;

//	dst.Open(fname, CFile::eModeWrite);
	dst.Open(dfile.c_str(), CFile::eModeWrite);

	try {

// TODO : If needed, make APP1 segment for EXIF
		for(list<CSegment>::iterator itor = m_seglist.begin(); itor != m_seglist.end(); itor++)
		{
			CSegment &seg = *itor;

	//		MSG_DEBUG("%s(0x%02X) Length=%d", (*itor).name_.c_str(), (*itor).T_, (*itor).L_);

			switch(seg.T_) {
			case M_SOI:
				{
					byte buf[2] = { 0xFF, 0xD8 };
					dst.Write(buf, 2);
				}
				break;
			case 0xFF:		// Compresssed image Data
				{
				// Read from src.
				if (file.Seek(seg.offset_, CFile::eSet) < 0 )
				{
					MSG_ERROR("Cannot seek. Offset=0x%u", seg.offset_);
					throw "Cannot parse";
				}

				byte *buffer;

				buffer = (byte *)malloc(seg.L_);
				if (buffer) {
					int cnt = file.Read(buffer, seg.L_);
					if (cnt < seg.L_ )
					{
						MSG_DEBUG("Read fail. %d, %d", cnt, seg.L_);
						free(buffer);
						throw "Invalid Src";
					}
					dst.Write(buffer, cnt);
					free(buffer);
				}
				}

				break;
			default:
				{
					byte buf[4];
					int size = seg.L_ + 2;		// +2 for size storage

					buf[0] = 0xFF;
					buf[1] = (byte)seg.T_;
					buf[2] = (0xFF00 & size) >> 8;
					buf[3] = 0x00FF & size;

					dst.Write(buf, 4);		// Tag(2), Size(2)

					if (seg.buf == NULL)
					{
	// Read from src & write to dst.
						if (file.Seek(seg.offset_, CFile::eSet) < 0 )
						{
							MSG_ERROR("Invalid offset. 0x%08x", seg.offset_);
							throw "Invalid Offset";
						}

						byte *buffer;

						buffer = (byte *)malloc(seg.L_);

						if (buffer != NULL) {
							int cnt = file.Read(buffer, seg.L_);

							if (cnt < seg.L_ )
							{
								MSG_DEBUG("Read fail. %d, %d", cnt, seg.L_);
								free(buffer);
								throw "Invalid Src";
							}

							dst.Write(buffer, cnt);

							free(buffer);
						}
					}
					else
					{
						dst.Write(seg.buf, seg.L_);
					}
				}
				break;
			}
		}

	}
	catch(...)
	{
		dst.Close();

		unlink(dfile.c_str());		// Remove tmp file

		return false;
	}

	dst.Close();

// Remove src & rename dst to src.
	if (bSamedest == true )
	{
		file.Close();
		unlink(sfile.c_str());

		if (rename (dfile.c_str(), sfile.c_str()) != 0 )
		{
			MSG_ERROR("Rename failed. from %s to %s", dfile.c_str(), sfile.c_str());
		}

	}

	return true;
}
