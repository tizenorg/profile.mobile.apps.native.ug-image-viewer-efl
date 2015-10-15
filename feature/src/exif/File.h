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
*/#pragma once

#include <stdio.h>
#include <stdlib.h>

class CFile {
public:
	enum {
		eSet = SEEK_SET,
		eCur = SEEK_CUR,
		eEnd = SEEK_END,
	};

	enum {
		eModeRead,
		eModeWrite,
		eModeRW,
	};

	CFile() : fp(NULL), fname(NULL), offset(0) {};
	~CFile() {
		Close();
	};

	bool Open(const char *fname, int mode = eModeRead) {
		const char *szMode[] = {
			"rb", "wb", "rb+"
		};

		fp = fopen(fname, szMode[mode]);

		if ( fp == NULL )
		{
			return false;
		}

		fseek(fp, 0L, SEEK_END);
		fsize = ftell(fp);

// Reset to head
		fseek(fp, 0L, SEEK_SET);

		this->fname = strdup(fname);
		return true;
	};

	bool Close() {
		if ( fp )
			fclose(fp);
		fp = NULL;

		if ( fname)
			free(fname);
		fname = NULL;

		return true;
	};

	int GetChar() const {
		if ( fp == NULL)
		{
			return EOF;
		}

		return fgetc(fp);
	};

	int Seek(long offset, int whence ) {
		return fseek(fp, offset, whence);
	};

	long Tell() {
		return ftell(fp);
	};

	int Read(void *buffer, int n) {
		return fread(buffer, 1, n, fp);
	};

	int SetBookmark(long offset) {

		return -1;
	};

	int GetBookmark(int bookmark)
	{
		return -1;
	};

	int Write(void *buffer, int n) {
		return fwrite(buffer, n, 1, fp);
	};

	bool IsLoaded() {
		return fp == NULL ? false : true;
	};

	size_t GetSize() {
		if ( fp == NULL )
		{
			return 0;
		}

		return fsize;
	};

	const char *GetFilename() {
		return fname;
	}

private:
	FILE *fp;

	char *fname;

	int offset;

	size_t fsize;	// File size

};
