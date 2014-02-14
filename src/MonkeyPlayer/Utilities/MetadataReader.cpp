// MetadataReader.cpp
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// gets metadata from music files
//

#include <fstream>
#include <mp4file.h>
#include <privateframe.h>
#include <sstream>
#include <time.h>

#include "DatabaseManager.h"
#include "FileManager.h"
#include "MetadataReader.h"

using namespace std;
using namespace TagLib;
using namespace MonkeyPlayer;

const int MetadataReader::HEADER_LENGTH = 39;
const string MetadataReader::WMA_FILE_EXT = "WMA";
const string MetadataReader::MP3_FILE_EXT = "MP3";
const string MetadataReader::MP4_FILE_EXT = "M4A";

/*static*/ std::string getString(TagLib::String s)
{
	if (s == TagLib::String::null)
	{
		return DatabaseStructs::DEF_EMPTY_STRING;
	}
	return std::string(s.toCString());
}
/*static*/ std::string getGenre(TagLib::String s)
{
	if (s == TagLib::String::null)
	{
		return DatabaseStructs::DEF_EMPTY_GENRE;
	}
	return std::string(s.toCString());
}
/*static*/ int getInt(TagLib::uint ui)
{
	if (ui == 0)
	{
		return DatabaseStructs::INVALID_ID;
	}
	return (int)ui;
}
/*static*/ int getYear(TagLib::uint ui)
{
	if (ui == 0)
	{
		return DatabaseStructs::DEF_EMPTY_YEAR;
	}
	return (int)ui;
}
/*static*/ int getDate(TagLib::uint ui)
{
	if (ui == 0)
	{
		return DatabaseStructs::DEF_EMPTY_DATE;
	}
	return (int)ui;
}
/*static*/ AlbumArt* MetadataReader::getAlbumArt(const ID3v2::Tag *tag)
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
				unsigned int rawSize = frame->picture().size();
				char* rawCopy = snew char[rawSize];
				memcpy(rawCopy, frame->picture().data(), rawSize);
				return snew AlbumArt(mimeType, rawCopy, rawSize);
			}
		}

	}
	 return NULL;
}

/*static*/ AlbumArt* MetadataReader::getAlbumArt(const char* file)
{
	GUID guid;
	ZeroMemory(&guid, sizeof(GUID));
	AlbumArt* art = NULL;

	MPEG::File f(file);

	ID3v2::Tag *tag = f.ID3v2Tag();
	if(tag)
	{
		art = getAlbumArt(tag);
	}
	if (art == NULL && getAlbumGUID(file, guid))
	{
		string dir = FileManager::getContainingDirectory(file);
		char guidStr[HEADER_LENGTH];
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
/*static*/ void MetadataReader::setAlbumArt(ID3v2::Tag *tag, const AlbumArt &albumArt)
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

/*static*/ void MetadataReader::getTrackInfo(const char *filename, Track* t, Album* a, Genre* g)
{
	MetadataReader::FILE_TYPE ftype = getFileType(filename);
	string sansPath = FileManager::getFileName(filename);

	// TRACK
	t->Artist = DatabaseStructs::DEF_EMPTY_ARTIST;
	t->VirtualArtist = DatabaseStructs::DEF_EMPTY_ARTIST;
	t->DateAdded = DatabaseStructs::DEF_EMPTY_DATE;
	t->Title = sansPath;
	t->Genre = DatabaseStructs::INVALID_ID;
	t->TrackNumber = 0;
	t->Length = 1;
	t->Filename = filename;

	// ALBUM
	a->Title = DatabaseStructs::DEF_EMPTY_ALBUM;
	a->Year = DatabaseStructs::DEF_EMPTY_YEAR;
	a->NumTracks = 1;
	a->Artist = DatabaseStructs::DEF_EMPTY_ARTIST;
	a->VirtualArtist = DatabaseStructs::DEF_EMPTY_ARTIST;

	// GENRE
	g->Title = DatabaseStructs::DEF_EMPTY_GENRE;
	g->StandardId = 0;

	bool success = false;
	if (ftype == MonkeyPlayer::MetadataReader::MP3)
	{
		success = getTrackInfoMP3(filename, t, a, g);
	}
	else if (ftype == MonkeyPlayer::MetadataReader::MP4)
	{
		success = getTrackInfoMP4(filename, t, a, g);
	}
	else if (ftype == MonkeyPlayer::MetadataReader::WMA)
	{
		success = getTrackInfoWMA(filename, t, a, g);
	}

	if (success)
	{
		string virtualArtist = t->Artist;
		if (virtualArtist.size() > 4 && FileManager::toUpper(virtualArtist.substr(0, 4)) == "THE ")
		{
			virtualArtist = virtualArtist.substr(4);
		}
		else if (virtualArtist.size() > 3 && FileManager::toUpper(virtualArtist.substr(0, 2)) == "A ")
		{
			virtualArtist = virtualArtist.substr(2);
		}
		else if (virtualArtist.size() > 2 && FileManager::toUpper(virtualArtist.substr(0, 3)) == "AN ")
		{
			virtualArtist = virtualArtist.substr(3);
		}
		a->VirtualArtist = virtualArtist;
		t->VirtualArtist = virtualArtist;
	}
	if (!success)
	{
	}

}
/*static*/ bool MetadataReader::getTrackInfoMP3(const char *filename, Track* t, Album* a, Genre* g)
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
		return true;
	}
	return false;
}

/*static*/ bool MetadataReader::getTrackInfoMP4(const char *filename, Track* t, Album* a, Genre* g)
{
/*	TagLib::MP4::File f(filename);
	//TagLib::MP4::Tag* tag = f.tag();
	//TagLib:::FileRef f(filename);
	if (f.tag())
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
		return true;
	}
*/	return false;
}
/*static*/ bool MetadataReader::getTrackInfoWMA(const char *filename, Track* t, Album* a, Genre* g)
{
	GUID guid;
	return getTrackInfoWMA(filename, t, a, g, guid, false);
}
/*static*/ bool MetadataReader::getTrackInfoWMA(const char *filename, Track* t, Album* a, Genre* g,
												GUID &guidOut, bool guidOnly)
{
	const int GUID_LENGTH = 16;
	const int SIZE_LENGTH = 8;
	const int SMALL_SIZE_LENGTH = 2;
	const int NUM_OBJ_LENGTH = 4;
	const string ART_GUID_RECORD_NAME = "WM/WMCollectionID";

	GUID contentDescGuid, extContentGuid, filePropGuid, headerExtGuid, metadataLibGuid;
	CLSIDFromString(L"{75B22633-668E-11CF-A6D9-00AA0062CE6C}",  &contentDescGuid);
	CLSIDFromString(L"{D2D0A440-E307-11D2-97F0-00A0C95EA850}",  &extContentGuid);
	CLSIDFromString(L"{8CABDCA1-A947-11CF-8EE4-00C00C205365}",  &filePropGuid);
	CLSIDFromString(L"{5FBF03B5-A92E-11CF-8EE3-00C00C205365}",  &headerExtGuid);
	CLSIDFromString(L"{44231C94-9498-49D1-A141-1D134E457054}",  &metadataLibGuid);
	
	bool retVal = false;
	ifstream infile;
	try
	{
		// read in file
		infile.open(filename, ios::binary);
		if (!infile.good())
		{
			return false;
		}
		char guidStr[HEADER_LENGTH];
		GUID guid;
		long size;
		int numObj;

		infile.seekg(0, ios::end);
		long pos = infile.tellg();

		infile.seekg(0, ios::beg);
		char *entireFile = snew char[pos];
		infile.read(entireFile, pos);
		infile.close();

		char* ptr = entireFile;
		// read guid
		memcpy(&guid, ptr, sizeof(GUID));
		ptr += GUID_LENGTH;
		toString(guid, guidStr);

		// read long
		memcpy(&size, ptr, sizeof(long));
		ptr += SIZE_LENGTH;

		// read int
		memcpy(&numObj, ptr, sizeof(int));
		ptr += NUM_OBJ_LENGTH;

		// read reserved (1-byte value)
		ptr += 1;

		// read reserved (1-byte value)
		byte sanityCheck = 0;
		memcpy(&sanityCheck, ptr, 1);
		ptr += 1;
		if (sanityCheck != 2)
		{
			string msg = filename;
			msg = "Error parsing " + msg + ". Reserved2 is incorrect.";
			Logger::instance()->write(msg);
			delete[] entireFile;
			return false;
		}
		bool artistFound = false;
		bool albumFound = false;
		bool guidFound = false;
		for (int i = 0; i < numObj; i++)
		{
			// read guid
			memcpy(&guid, ptr, sizeof(GUID));
			ptr += GUID_LENGTH;
			toString(guid, guidStr);

			long contentSize;
			// read long
			memcpy(&contentSize, ptr, sizeof(long));
			ptr += SIZE_LENGTH;
			if (!guidOnly && guid == contentDescGuid) // title, artist
			{
				short titleLen, authorLen, copyrightLen, descLen, ratingLen;
				// read short
				memcpy(&titleLen, ptr, sizeof(short));
				ptr += SMALL_SIZE_LENGTH;
				// read short
				memcpy(&authorLen, ptr, sizeof(short));
				ptr += SMALL_SIZE_LENGTH;
				// read short
				memcpy(&copyrightLen, ptr, sizeof(short));
				ptr += SMALL_SIZE_LENGTH;
				// read short
				memcpy(&descLen, ptr, sizeof(short));
				ptr += SMALL_SIZE_LENGTH;
				// read short
				memcpy(&ratingLen, ptr, sizeof(short));
				ptr += SMALL_SIZE_LENGTH;
				
				// read string
				t->Title = readWMAString(&ptr, titleLen);
				t->Artist = readWMAString(&ptr, authorLen);
				string c = readWMAString(&ptr, copyrightLen);
				string desc = readWMAString(&ptr, descLen);
				string r = readWMAString(&ptr, ratingLen);
				artistFound = true;
			}
			else if (!guidOnly && guid == extContentGuid) // year, album artist, album title, genre
			{
				short numDesc = 0; 
				// read short
				memcpy(&numDesc, ptr, sizeof(short));
				ptr += SMALL_SIZE_LENGTH;
				for (int j = 0; j <numDesc; j++)
				{
					short descNameLength, descValType, descValLength;

					// read short
					memcpy(&descNameLength, ptr, sizeof(short));
					ptr += SMALL_SIZE_LENGTH;

					// read name
					string name = readWMAString(&ptr, descNameLength);
					// read short
					memcpy(&descValType, ptr, sizeof(short));
					ptr += SMALL_SIZE_LENGTH;
					// read short
					memcpy(&descValLength, ptr, sizeof(short));
					ptr += SMALL_SIZE_LENGTH;
					if (descValType == 0)
					{
						string val = readWMAString(&ptr, descValLength);
						
						if (name == "WM/Year")
						{
							a->Year = atoi(val.c_str());
						}
						else if (name == "WM/AlbumArtist")
						{
							a->Artist = val;
						}
						else if (name == "WM/AlbumTitle")
						{
							a->Title = val;
							albumFound = true;
						}
						else if (name == "WM/Genre")
						{
							g->Title = val;
							g->StandardId = 0;
						}
						Logger::instance()->write(val);					
					}
					else if (descValType == 1)
					{
						ptr += descValLength;
					}
					else if (descValType == 2)
					{
						ptr += descValLength;
					}
					else if (descValType == 3)
					{
						if (name == "WM/TrackNumber")
						{
							int trackNo;
							// read int
							memcpy(&trackNo, ptr, sizeof(int));
							ptr += descValLength;
							t->TrackNumber = trackNo;
						}
						else
						{
							ptr += descValLength;
						}
					}
					else if (descValType == 4)
					{
						ptr += descValLength;
					}
					else if (descValType == 5)
					{
						ptr += descValLength;
					}
					else
					{
						ptr += descValLength;
					}
				}
			}
			else if (!guidOnly && guid == filePropGuid) // track time
			{
				long remaining = contentSize - (GUID_LENGTH + SIZE_LENGTH);
				GUID fileGuid;
				// read GUID
				memcpy(&fileGuid, ptr, sizeof(GUID));
				ptr += GUID_LENGTH;
				toString(fileGuid, guidStr);
				remaining -= GUID_LENGTH;

				// read long...file size
				long junk;
				memcpy(&junk, ptr, sizeof(long));
				ptr += SIZE_LENGTH;
				remaining -= SIZE_LENGTH;

				// read long...date
				memcpy(&junk, ptr, sizeof(long));
				ptr += SIZE_LENGTH;
				remaining -= SIZE_LENGTH;

				// read long...packet count
				memcpy(&junk, ptr, sizeof(long));
				ptr += SIZE_LENGTH;
				remaining -= SIZE_LENGTH;

				// read long
				unsigned long trackLen;
				memcpy(&trackLen, ptr, sizeof(long));
				ptr += SIZE_LENGTH;
				t->Length = max((int)(trackLen / 1e7), 1);
				remaining -= SIZE_LENGTH;
				ptr += remaining;
			}
			else if (guidOnly && guid == headerExtGuid) // guid
			{
				// read guid
				memcpy(&guid, ptr, sizeof(GUID));
				ptr += GUID_LENGTH;
				toString(guid, guidStr);
				long remaining = contentSize - (GUID_LENGTH + SIZE_LENGTH + GUID_LENGTH);

				short res1;
				memcpy(&res1, ptr, sizeof(short));
				ptr += SMALL_SIZE_LENGTH;

				int dataSize;
				// read int
				memcpy(&dataSize, ptr, sizeof(int));
				ptr += SIZE_LENGTH/2;

				if (dataSize > 0)
				{
					// child objects...fml!
					int index = 0; 
					while (index < dataSize)
					{
						// read guid
						memcpy(&guid, ptr + index, sizeof(GUID));
						index += GUID_LENGTH;
						toString(guid, guidStr);

						long childContentSize;
						// read long
						memcpy(&childContentSize, ptr + index, sizeof(long));
						index += SIZE_LENGTH;

						if (guid == metadataLibGuid)
						{
							short recCount;
							// read long
							memcpy(&recCount, ptr + index, sizeof(short));
							index += SMALL_SIZE_LENGTH;

							for (long rec = 0; rec < recCount; rec++)
							{
								short listIndex, streamNo, nameLen, dataType;
								int dataLen;
								string recName;
								string recVal;
								memcpy(&listIndex, ptr + index, sizeof(short));
								index += SMALL_SIZE_LENGTH;
								memcpy(&streamNo, ptr + index, sizeof(short));
								index += SMALL_SIZE_LENGTH;
								memcpy(&nameLen, ptr + index, sizeof(short));
								index += SMALL_SIZE_LENGTH;
								memcpy(&dataType, ptr + index, sizeof(short));
								index += SMALL_SIZE_LENGTH;
								memcpy(&dataLen, ptr + index, sizeof(int));
								index += SIZE_LENGTH / 2;
								char* tmpPtr = ptr + index;
								recName = readWMAString(&tmpPtr, nameLen);
								index += nameLen;

								if (dataType == 6 && recName == ART_GUID_RECORD_NAME)
								{
									memcpy(&guidOut, ptr + index, sizeof(GUID));
									guidFound = true;
								}
								index += dataLen;
							} // for recCount
						} // if metadataLibGuid
						else 
						{
							index += childContentSize - (GUID_LENGTH + SIZE_LENGTH);
						}
					} // while index < dataSize
				} // if dataSize > 0
				ptr += dataSize;
			}
			else // skip header
			{
				ptr += contentSize - (GUID_LENGTH + SIZE_LENGTH);
			}
		}
		delete[] entireFile;
		if (guidOnly)
		{
			retVal = guidFound;
			if (!retVal)
			{
				string str = "No GUID found in ";
				str += filename;
				Logger::instance()->write(str);
			}
		}
		else
		{
			retVal = albumFound && artistFound;
			if (!retVal)
			{
				string str = "Metadata not found in ";
				str += filename;
				Logger::instance()->write(str);
			}
		}
	}
	catch (...)
	{
		retVal = false;
		try
		{
			infile.close();
		}
		catch (...) {}
	}

	return retVal;
}
/*static*/ string MetadataReader::readWMAString(char** ptr, int length)
{
	if (length <= 0)
	{
		return "";
	}
	
	char *textBuf = snew char[length];

	char defChar = ' ';
	WCHAR* title = snew WCHAR[length];
	memcpy(title, *ptr, length);
	*ptr += length;
	WideCharToMultiByte(CP_ACP, 0, title, -1, textBuf, length, &defChar, NULL);
	string retVal = textBuf;
	delete title;
	delete textBuf;
	return retVal;
}
/*static*/ MetadataReader::FILE_TYPE MetadataReader::getFileType(const char* file)
{
	string filename = file;
	size_t pos = filename.find_last_of('.');
	MetadataReader::FILE_TYPE retVal = UNKNOWN;
	if (pos == string::npos || pos == filename.length() - 1)
	{
		retVal = MetadataReader::UNKNOWN;
	}
	else
	{
		string ext = filename.substr(pos + 1);
		for (size_t i = 0; i <ext.length(); i++)
		{
			ext[i] = toupper(ext[i]);
		}
		if (ext == MetadataReader::WMA_FILE_EXT)
		{
			retVal = MetadataReader::WMA;
		}
		else if (ext == MetadataReader::MP3_FILE_EXT)
		{
			retVal = MetadataReader::MP3;
		}
		else if (ext == MetadataReader::MP4_FILE_EXT)
		{
			retVal = MetadataReader::MP4;
		}
	}
	return retVal;
}

/*static*/ bool MetadataReader::getAlbumGUID(const char* filename, GUID &guid)
{
	MetadataReader::FILE_TYPE ftype = getFileType(filename);
	bool success = false;
	if (ftype == MonkeyPlayer::MetadataReader::MP3)
	{
		return getAlbumGUIDMP3(filename, guid);
	}
	else if (ftype == MonkeyPlayer::MetadataReader::WMA)
	{
		return getAlbumGUIDWMA(filename, guid);
	}
	return false;
}
/*static*/ bool MetadataReader::getAlbumGUIDMP3(const char* filename, GUID &guid)
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
/*static*/ bool MetadataReader::getAlbumGUIDWMA(const char* filename, GUID &guid)
{
	return getTrackInfoWMA(filename, NULL, NULL, NULL, guid, true);
}
/*static*/ void MetadataReader::toString(GUID & guid, char* guidStr)
{
	sprintf_s(guidStr, HEADER_LENGTH, "{%08lX-%04hX-%04hX-%02hX%02hX-%02hX%02hX%02hX%02hX%02hX%02hX}",
		guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], 
		guid.Data4[5], guid.Data4[6], guid.Data4[7]);
}
