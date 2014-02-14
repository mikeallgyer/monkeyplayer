//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Definitions for structures returned by DatabaseStructs

#ifndef DATABASE_STRUCTS_H
#define DATABASE_STRUCTS_H

#include <string>
#include <vector>

using namespace std;

namespace DatabaseStructs
{
	const std::string DEF_EMPTY_STRING = "UNKNOWN_VALUE";
	const std::string DEF_EMPTY_GENRE = "Unknown Genre";
	const std::string DEF_EMPTY_ARTIST = "Unknown Artist";
	const std::string DEF_EMPTY_ALBUM = "Unknown Album";
	const int DEF_EMPTY_YEAR = 1900;
	const long DEF_EMPTY_DATE = 0;
	const int INVALID_ID = -1;
	const std::string VARIOUS_ARTIST = "Various";
}
struct Genre
{
	Genre() : Id(DatabaseStructs::INVALID_ID), Title(DatabaseStructs::DEF_EMPTY_STRING), StandardId(0) {}
	Genre(string title, int stdId) : Id(DatabaseStructs::INVALID_ID), Title(title), StandardId(stdId) {}
	Genre(int id, string title, int stdId) : Id(id), Title(title), StandardId(stdId) {}
	Genre(const Genre& g)
	{
		Id = g.Id;
		Title = g.Title;
	}

	int Id;
	string Title;
	int StandardId;
};
struct Album
{
	Album() : Id(DatabaseStructs::INVALID_ID), Title(DatabaseStructs::DEF_EMPTY_ALBUM), 
		Year(0), Artist(DatabaseStructs::DEF_EMPTY_ARTIST), VirtualArtist(DatabaseStructs::DEF_EMPTY_ARTIST) {}
	Album(int numTracks, string title, int year, string artist, string virtualArtist)
		: Id(DatabaseStructs::INVALID_ID), NumTracks(numTracks), Title(title), 
		Year(DatabaseStructs::DEF_EMPTY_YEAR), Artist(artist), VirtualArtist(virtualArtist) {}
	Album(int id, int numTracks, string title, int year, string artist, string virtualArtist)
		: Id(id), NumTracks(numTracks), Title(title), Year(year), Artist(artist), VirtualArtist(virtualArtist) {}

	Album(const Album& a)
	{
		Id = a.Id;
		NumTracks = a.NumTracks;
		Title = a.Title;
		Year = a.Year;
		Artist = a.Artist;
		VirtualArtist = a.VirtualArtist;
	}

	int Id;
	int NumTracks;
	string Title;
	int Year;
	string Artist;
	string VirtualArtist;
};

struct Track
{
	Track() : Id(DatabaseStructs::INVALID_ID), Filename(DatabaseStructs::DEF_EMPTY_STRING), Title(DatabaseStructs::DEF_EMPTY_STRING), Artist(DatabaseStructs::DEF_EMPTY_STRING), 
		VirtualArtist(DatabaseStructs::DEF_EMPTY_ARTIST), TrackNumber(DatabaseStructs::INVALID_ID), AlbumId(DatabaseStructs::INVALID_ID), Length(1), 
		DateAdded(DatabaseStructs::INVALID_ID), Ignored(false), Genre(0), DateUsed(DatabaseStructs::INVALID_ID), NumPlayed(0) {}
	Track(int id, string filename, string title, string artist, string virtualArtist, int trackNumber, int albumId, 
		int length, int dateAdded, bool ignored, int genre, int dateUsed, int numPlayed) 
		: Id(id), Filename(filename), Title(title), Artist(artist), VirtualArtist(virtualArtist), TrackNumber(trackNumber), AlbumId(albumId), 
		Length(length), DateAdded(dateAdded), Ignored(ignored), Genre(genre), DateUsed(dateUsed), NumPlayed(numPlayed) {}
	Track(string filename, string title, string artist, string virtualArtist, int trackNumber, int albumId,
		int length, int dateAdded, bool ignored, int genre, int dateUsed, int numPlayed)
		: Id(DatabaseStructs::INVALID_ID), Filename(filename), Title(title), Artist(artist), VirtualArtist(virtualArtist), TrackNumber(trackNumber), AlbumId(albumId), 
		Length(length), DateAdded(dateAdded), Ignored(ignored), Genre(genre), DateUsed(dateUsed), NumPlayed(numPlayed) {}
	Track(const Track& t)
	{
		Id = t.Id;
		Filename = t.Filename;
		Title = t.Title;
		Artist = t.Artist;
		VirtualArtist = t.VirtualArtist;
		TrackNumber = t.TrackNumber;
		AlbumId = t.AlbumId;
		Length = t.Length;
		DateAdded = t.DateAdded;
		Ignored = t.Ignored;
		Genre = t.Genre;
		DateUsed = t.DateUsed;
		NumPlayed = t.NumPlayed;
	}
	void setTrackInfo(Track& t)
	{
		Id = t.Id;
		Filename = t.Filename;
		Title = t.Title;
		Artist = t.Artist;
		VirtualArtist = t.Artist;
		TrackNumber = t.TrackNumber;
		AlbumId = t.AlbumId;
		Length = t.Length;
		DateAdded = t.DateAdded;
		Ignored = t.Ignored;
		Genre = t.Genre;
		DateUsed = t.DateUsed;
		NumPlayed = t.NumPlayed;
	}

	int Id;
	string Filename;
	string Title;
	string Artist;
	string VirtualArtist;
	int TrackNumber;
	int AlbumId;
	int Length;
	int DateAdded;
	bool Ignored;
	int Genre;
	int DateUsed;
	int NumPlayed;
};

struct TrackExtended
{
	Track* t;
	Genre* g;
	Album* a;
};
struct AlbumWithTracks
{
	Album* album;
	vector<Track*> tracks;
};
struct ArtistWithTracks
{
	string artist;
	vector<Track*> tracks;
};

struct Playlist
{
	Playlist() : Id(DatabaseStructs::INVALID_ID), Name(DatabaseStructs::DEF_EMPTY_STRING), Filename(DatabaseStructs::DEF_EMPTY_STRING) {}
	Playlist(string name, string filename) : Id(DatabaseStructs::INVALID_ID), Name(name), Filename(filename) {}
	Playlist(int id, string name, string filename) : Id(id), Name(name), Filename(filename) {}
	Playlist(const Playlist& p)
	{
		Id = p.Id;
		Name = p.Name;
		Filename = p.Filename;
	}

	int Id;
	string Name;
	string Filename;
};

struct PlaylistTrack
{
	PlaylistTrack() : Id(DatabaseStructs::INVALID_ID), TrackId(DatabaseStructs::INVALID_ID), PlaylistId(DatabaseStructs::INVALID_ID), T_Index(DatabaseStructs::INVALID_ID) {}
	PlaylistTrack(int trackId, int playlistId, int index) : Id(DatabaseStructs::INVALID_ID), TrackId(trackId), PlaylistId(playlistId), T_Index(index) {}
	PlaylistTrack(int id, int trackId, int playlistId, int index) : Id(id), TrackId(trackId), PlaylistId(playlistId), T_Index(index) {}
	PlaylistTrack(const PlaylistTrack& p)
	{
		Id = p.Id;
		TrackId = p.TrackId;
		PlaylistId = p.PlaylistId;
		T_Index = p.T_Index;
	}

	int Id;
	int TrackId;
	int PlaylistId;
	int T_Index;
};

struct DBDefault
{
	DBDefault() : Id(DatabaseStructs::INVALID_ID), Name(DatabaseStructs::DEF_EMPTY_STRING), StrVal(DatabaseStructs::DEF_EMPTY_STRING), IntVal(DatabaseStructs::INVALID_ID) {}
	DBDefault(string name, string strVal, int intVal) : Id(DatabaseStructs::INVALID_ID), Name(name), StrVal(strVal), IntVal(intVal) {}
	DBDefault(int id, string name, string strVal, int intVal) : Id(id), Name(name), StrVal(strVal), IntVal(intVal) {}
	DBDefault(const DBDefault &d)
	{
		Id = d.Id;
		Name = d.Name;
		StrVal = d.StrVal;
		IntVal = d.IntVal;
	}

	int Id;
	string Name;
	string StrVal;
	int IntVal;

};

#endif
