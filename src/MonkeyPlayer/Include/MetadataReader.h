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

namespace MonkeyPlayer
{
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
		enum FILE_TYPE { MP3, MP4, WMA, UNKNOWN };
		static const string WMA_FILE_EXT;
		static const string MP3_FILE_EXT;
		static const string MP4_FILE_EXT;

		static const int HEADER_LENGTH;
		
		static void getTrackInfo(const char* filename, Track* t, Album* a, Genre *g);
		static bool getTrackInfoMP3(const char* filename, Track* t, Album* a, Genre *g);
		static bool getTrackInfoMP4(const char* filename, Track* t, Album* a, Genre *g);
		static bool getTrackInfoWMA(const char* filename, Track* t, Album* a, Genre *g);
		static bool getTrackInfoWMA(const char *filename, Track* t, Album* a, Genre* g, GUID &guidOut, bool guidOnly);
		static string readWMAString(char **ptr, int length);
		static AlbumArt* getAlbumArt(const ID3v2::Tag *tag);
		static AlbumArt* getAlbumArt(const char* file);
		static void setAlbumArt(ID3v2::Tag *tag, const AlbumArt &albumArt);
		static FILE_TYPE getFileType(const char* file);
		static bool getAlbumGUID(const char* filename, GUID &guid);


	private:
		static bool getAlbumGUIDMP3(const char* filename, GUID &guid);
		static bool getAlbumGUIDWMA(const char* filename, GUID &guid);
		static void toString(GUID & guid, char* guidStr);
	};
}
#endif