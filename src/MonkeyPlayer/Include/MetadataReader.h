// MetadataReader.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// gets metadata from music files

#include <attachedpictureframe.h>
#include <tag.h>
#include <fileref.h>
#include <id3v2tag.h>
#include <mpegfile.h>

#include <string.h>

#include "DatabaseStructs.h"

#ifndef METADATA_READER_H
#define METADATA_READER_H

using namespace TagLib;

struct AlbumArt
{
	static enum ALBUM_ART_MIME { UNKNOWN, PNG, JPG };

	AlbumArt(ALBUM_ART_MIME m, const char *d, int len) :
      mimeType(m), data(d), length(len) {}
		
	~AlbumArt()
	{
		if (data != NULL)
		{
			delete data;
		}
	}
	ALBUM_ART_MIME mimeType;
   const char* data;
	int length;
};

class MetadataReader
{
public:
	static const int GUID_LENGTH;
	
	static void getTrackInfo(const char* filename, Track* t, Album* a, Genre *g);
	static AlbumArt* getAlbumArt(const ID3v2::Tag *tag);
	static AlbumArt* getAlbumArt(const char* file);
	static void setAlbumArt(ID3v2::Tag *tag, const AlbumArt &albumArt);

private:
	static bool getAlbumGUID(const char* filename, GUID &guid);
	static void toString(GUID & guid, char* guidStr);
};

#endif