// MetadataReader.cpp
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// gets metadata from music files
//

#include <fstream>
#include <privateframe.h>
#include <sstream>
#include <time.h>

#include "DatabaseManager.h"
#include "FileManager.h"
#include "MetadataReader.h"

using namespace std;
using namespace TagLib;

const int MetadataReader::GUID_LENGTH = 39;

std::string getString(TagLib::String s)
{
	if (s == TagLib::String::null)
	{
		return DatabaseStructs::DEF_EMPTY_STRING;
	}
	return std::string(s.toCString());
}
std::string getGenre(TagLib::String s)
{
	if (s == TagLib::String::null)
	{
		return DatabaseStructs::DEF_EMPTY_GENRE;
	}
	return std::string(s.toCString());
}
int getInt(TagLib::uint ui)
{
	if (ui == 0)
	{
		return DatabaseStructs::INVALID_ID;
	}
	return (int)ui;
}
int getYear(TagLib::uint ui)
{
	if (ui == 0)
	{
		return DatabaseStructs::DEF_EMPTY_YEAR;
	}
	return (int)ui;
}
int getDate(TagLib::uint ui)
{
	if (ui == 0)
	{
		return DatabaseStructs::DEF_EMPTY_DATE;
	}
	return (int)ui;
}
AlbumArt* MetadataReader::getAlbumArt(const ID3v2::Tag *tag)
{
	ID3v2::FrameList frames = tag->frameList("APIC");

	if (!frames.isEmpty())
	{
		ID3v2::AttachedPictureFrame *frame =
        static_cast<ID3v2::AttachedPictureFrame *>(frames.front());

		if (frame != NULL)
		{
			std::string mime = frame->mimeType().toCString();
			transform(mime.begin(), mime.end(), mime.begin(), ::toupper);
			AlbumArt::ALBUM_ART_MIME mimeType = AlbumArt::UNKNOWN;
			if (mime.find("JPG") != string::npos || mime.find("JPEG") != string::npos)
			{
				mimeType = AlbumArt::JPG;
			}
			else if (mime.find("PNG") != string::npos)
			{
				mimeType = AlbumArt::PNG;
			}
			
			if (mimeType != AlbumArt::UNKNOWN)
			{
				const char* raw = frame->picture().data();
				unsigned int rawSize = frame->picture().size();
				char* rawCopy = snew char[rawSize];
				memcpy(rawCopy, raw, rawSize);
				
				return snew AlbumArt(mimeType, rawCopy, rawSize);
			}
		}

	}
	 return NULL;
}

AlbumArt* MetadataReader::getAlbumArt(const char* file)
{
	GUID guid;
	ZeroMemory(&guid, sizeof(GUID));

	MPEG::File f(file);

	ID3v2::Tag *tag = f.ID3v2Tag();
	AlbumArt* art = NULL;
	if(tag)
	{
		art = getAlbumArt(tag);
	}

	if (art == NULL && getAlbumGUID(file, guid))
	{
		string dir = FileManager::getContainingDirectory(file);
		char guidStr[GUID_LENGTH];
		toString(guid, guidStr);

		string fullPath = dir + "\\AlbumArt_" + guidStr + "";
		string fullPathL = fullPath + "_Large.jpg";
		string fullPathS = fullPath + "_Small.jpg";
		try
		{
			ifstream infile(fullPathL.c_str(), ios::binary);
			if (!infile.good())
			{
				infile.close();
				infile.open(fullPathS.c_str(), ios::binary);
			}
			if (infile.good())
			{
				infile.seekg(0, ios::end);
				int length = infile.tellg();

				infile.seekg(0, ios::beg);
				char* data = snew char[length];
				infile.read(data, length);

				art = snew AlbumArt(AlbumArt::JPG, data, length);
				infile.close();
			}

		}
		catch (...)
		{
			art = NULL;
		}

	}

	return art;
}
void MetadataReader::setAlbumArt(ID3v2::Tag *tag, const AlbumArt &albumArt)
{
    ID3v2::FrameList frames = tag->frameList("APIC");
    ID3v2::AttachedPictureFrame *frame = 0;

    if(frames.isEmpty())
    {
        frame = snew TagLib::ID3v2::AttachedPictureFrame;
        tag->addFrame(frame);
    }
    else
    {
        frame = static_cast<ID3v2::AttachedPictureFrame *>(frames.front());
    }

    frame->setPicture(ByteVector(albumArt.data));
	 if (albumArt.mimeType == AlbumArt::JPG)
	 {
		 frame->setMimeType(String("image/jpeg"));
	 }
	 else if (albumArt.mimeType == AlbumArt::PNG)
	 {
		 frame->setMimeType(String("image/png"));
	 }
}

void MetadataReader::getTrackInfo(const char *filename, Track* t, Album* a, Genre* g)
{
	TagLib::FileRef f(filename);
	if (!f.isNull() && f.tag())
	{
		// TRACK
		t->Title = getString(f.tag()->title());
		t->Artist = getString(f.tag()->artist());;
		t->TrackNumber = f.tag()->track();
		if (f.audioProperties() != NULL)
		{
			t->Length = f.audioProperties()->length();
		}
		t->DateAdded = (long)time(NULL);
		t->Ignored = false;

		t->DateUsed = DatabaseStructs::DEF_EMPTY_DATE;
		t->NumPlayed = 0;

		// ALBUM
		a->Title = getString(f.tag()->album());
		a->Year = getYear(f.tag()->year());
		a->NumTracks = DatabaseStructs::INVALID_ID;
		a->Artist = t->Artist;

		// GENRE
		g->Title = getGenre(f.tag()->genre());
		g->StandardId = 0;
	}
}

bool MetadataReader::getAlbumGUID(const char* filename, GUID &guid)
{
	const int HEADER_LENGTH = 10;
	const int TAG_INDICATOR_LENGTH = 3;
	const string TAG_TYPE = "ID3";
	const int TAG_MAJOR = 3;
	const int TAG_MINOR = 0;
	const int HEADER_SIZE_BEGIN = 6;
	const int HEADER_SIZE_END = 9;
	const int EXTENDED_HEADER_POS = 5;
	const int EXTENDED_HEADER_SIZE_LENGTH = 4;
	const int FRAME_HEADER_LENGTH = 10;
	const int FRAME_ID_LENGTH = 4;
	const string FRAME_MEDIA_PLAYER_OWNER1 = "WM/WMCollectionID";
//	const string FRAME_MEDIA_PLAYER_OWNER2 = "WM/WMCollectionGroupID"; // this apparently screws up multi-cd albums
	const string FRAME_MEDIA_PLAYER_ID = "PRIV";
	const int FRAME_SIZE_BEGIN = 4;
	const int FRAME_SIZE_END = 7;
	

	bool retValue = false;
	try
	{
		// read in file
		ifstream infile(filename, ios::binary);

		infile.seekg(0, ios::beg);

		char header[HEADER_LENGTH];
		infile.read(header, HEADER_LENGTH);

		string tagType(TAG_INDICATOR_LENGTH, '0');
		for (int i = 0; i < TAG_INDICATOR_LENGTH; i++)
		{
			tagType[i] = header[i];
		}
		int majorVersion = (int)header[TAG_INDICATOR_LENGTH];
		int minorVersion = (int)header[TAG_INDICATOR_LENGTH + 1];
		if (tagType == TAG_TYPE && 
			majorVersion == TAG_MAJOR && minorVersion == TAG_MINOR)
		{
			// get extended header bit
			bool extHeader = (header[EXTENDED_HEADER_POS] & 0x0010000);

			// get size of extended header
			int extHeaderSize = 0;
			if (extHeader)
			{
				char extHeaderData[EXTENDED_HEADER_SIZE_LENGTH];
				infile.seekg(HEADER_LENGTH, ios::beg);
				infile.read(extHeaderData, EXTENDED_HEADER_SIZE_LENGTH);
				for (int i = 0; i <= EXTENDED_HEADER_SIZE_LENGTH; i++)
				{
					extHeaderSize = extHeaderSize << 8;
					extHeaderSize |= (unsigned char)extHeaderData[i];
				}
				extHeaderSize += EXTENDED_HEADER_SIZE_LENGTH;

			}
			// get size of tag.
			// 4 bytes with the most significant bit ignored
			int tagSize = 0;
			for (int i = HEADER_SIZE_BEGIN; i <= HEADER_SIZE_END; i++)
			{
				tagSize = tagSize << 7;
				tagSize |= (header[i] & 0x7f);
			}
			// tagSize includes extended header
			tagSize += HEADER_LENGTH;

			// read tags
			int currPos = HEADER_LENGTH + extHeaderSize;
			while (currPos < tagSize)
			{
				infile.seekg(currPos, ios::beg);

				char frameHeader[FRAME_HEADER_LENGTH];
				infile.read(frameHeader, FRAME_HEADER_LENGTH);
				
				// frame id
				string frameType(FRAME_ID_LENGTH, '0');
				for (int i = 0; i < FRAME_ID_LENGTH; i++)
				{
					frameType[i] = frameHeader[i];
				}
				// frame length
				int frameLength = 0;
				for (int i = FRAME_SIZE_BEGIN; i <= FRAME_SIZE_END; i++)
				{
					frameLength = frameLength << 8;
					frameLength |= (unsigned char)frameHeader[i];
				}
				if (frameType == FRAME_MEDIA_PLAYER_ID)
				{
					char* frameData = snew char[frameLength];
					infile.read(frameData, frameLength);
					string owner = frameData;
					if (owner == FRAME_MEDIA_PLAYER_OWNER1)// || owner == FRAME_MEDIA_PLAYER_OWNER2)
					{
						memcpy(&guid, frameData + owner.length() + 1, sizeof(GUID));
						retValue = true;
					}
					delete[] frameData;
				}

				currPos += FRAME_HEADER_LENGTH + frameLength;
			}
		}
		infile.close();
	}
	catch (...)
	{
		retValue = false;
	}

	return retValue;
}

void MetadataReader::toString(GUID & guid, char* guidStr)
{
	sprintf_s(guidStr, GUID_LENGTH, "{%08lX-%04hX-%04hX-%02hX%02hX-%02hX%02hX%02hX%02hX%02hX%02hX}",
		guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], 
		guid.Data4[5], guid.Data4[6], guid.Data4[7]);
}
