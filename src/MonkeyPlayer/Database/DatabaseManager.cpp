// DatabaseManager.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Manages database interaction

#include "DatabaseManager.h"
#include "d3dApp.h"
#include "d3dUtil.h"
#include "Settings.h"
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

DatabaseManager* DatabaseManager::mInstance = NULL;

// used for synchronization
CCriticalSection DatabaseManager::mCritSection;

DatabaseManager::DatabaseManager()
{
	string dbPath;
#ifdef _DEBUG
	dbPath = Settings::instance()->getStringValue("database_file", "monkeyDB.mdb");
#else
	TCHAR appPath[1024];
	Settings::getAppDataPath(appPath, "monkeyDB.mdb"); 
	dbPath = appPath;
#endif
	
	int error = sqlite3_open(dbPath.c_str(), &mDB);

	if (error)
	{
		MessageBox(0, "Error Opening database file", "Error", 0);
		sqlite3_close(mDB);
		PostQuitMessage(1);
	}
	else
	{
		string query = "PRAGMA foreign_keys = ON";

		char** resTable;
		int numRows, numCols;
		char *errorMsg;

		bool foundTable = false;
		int error = sqlite3_get_table(mDB, query.c_str(), &resTable, &numRows, &numCols, &errorMsg);
		sqlite3_free_table(resTable);

		testAndValidateDatabase(); 
	}
	mInTransaction = false;
}

DatabaseManager::~DatabaseManager()
{
	sqlite3_close(mDB);
}
void DatabaseManager::testAndValidateDatabase()
{
	CSingleLock lock(&mCritSection, true);

	string query = "SELECT NAME FROM SQLITE_MASTER WHERE TYPE='table' ORDER BY NAME";
	char** resTable;
	int numRows, numCols;
	char *errorMsg;

	bool foundTable = false;
	int error = sqlite3_get_table(mDB, query.c_str(), &resTable, &numRows, &numCols, &errorMsg);

	if (!error)
	{
		for (int i = 1; i <= numRows; i++)
		{
			for (int j = 0; j < numCols; j++)
			{
				if (strcmp(resTable[i * numCols + j], "TRACKS") == 0)
				{
					foundTable = true;
				}
			}
		}
	}
	sqlite3_free_table(resTable);
	lock.Unlock();

	if (!foundTable)
	{
		createDatabase();
	}
}

void DatabaseManager::createDatabase()
{
	CSingleLock lock(&mCritSection, true);
	char** resTable;
	int numRows, numCols;
	char *errorMsg;
	
	string query = "DROP TABLE PLAYLIST_TRACKS";
	int error = sqlite3_get_table(mDB, query.c_str(), &resTable, &numRows, &numCols, &errorMsg);
	sqlite3_free_table(resTable);
	
	query = "DROP TABLE PLAYLISTS";
	error = sqlite3_get_table(mDB, query.c_str(), &resTable, &numRows, &numCols, &errorMsg);
	sqlite3_free_table(resTable);

	query = "DROP TABLE TRACK_GENRES";
	error = sqlite3_get_table(mDB, query.c_str(), &resTable, &numRows, &numCols, &errorMsg);
	sqlite3_free_table(resTable);
	
	query = "DROP TABLE TRACKS";
	error = sqlite3_get_table(mDB, query.c_str(), &resTable, &numRows, &numCols, &errorMsg);
	sqlite3_free_table(resTable);
	
	query = "DROP TABLE ALBUMS";
	error = sqlite3_get_table(mDB, query.c_str(), &resTable, &numRows, &numCols, &errorMsg);
	sqlite3_free_table(resTable);
	
	query = "DROP TABLE GENRES";
	error = sqlite3_get_table(mDB, query.c_str(), &resTable, &numRows, &numCols, &errorMsg);
	sqlite3_free_table(resTable);

	query = "DROP TABLE DB_DEFAULTS";
	error = sqlite3_get_table(mDB, query.c_str(), &resTable, &numRows, &numCols, &errorMsg);
	sqlite3_free_table(resTable);

	query = "DROP TABLE MUSIC_DIRECTORIES";
	error = sqlite3_get_table(mDB, query.c_str(), &resTable, &numRows, &numCols, &errorMsg);
	sqlite3_free_table(resTable);

	query = "CREATE TABLE GENRES (ID INTEGER PRIMARY KEY, TITLE VARCHAR(100), STANDARD_ID INTEGER)";
	error = sqlite3_get_table(mDB, query.c_str(), &resTable, &numRows, &numCols, &errorMsg);
	sqlite3_free_table(resTable);
	if (error) MessageBox(0, errorMsg, "Error creating database table: GENRES", 0);


	query = "CREATE TABLE ALBUMS (ID INTEGER PRIMARY KEY, " 
		"NUM_TRACKS INTEGER, " 
		"TITLE VARCHAR(100), " 
		"YEAR INTEGER,"
		"ARTIST VARCHAR(250))";
	error = sqlite3_get_table(mDB, query.c_str(), &resTable, &numRows, &numCols, &errorMsg);
	sqlite3_free_table(resTable);
	if (error) MessageBox(0, errorMsg, "Error creating database table: ALBUMS", 0);

	query = "CREATE TABLE TRACKS (ID INTEGER PRIMARY KEY, "
		"FILENAME VARCHAR(255), "
		"TITLE VARCHAR(255), "
		"ARTIST VARCHAR(255), "
		"TRACK_NUMBER INTEGER, "
		"ALBUM INTEGER, "
		"LENGTH INTEGER, "
		"DATE_ADDED BIGINT, "
		"IGNORED TINYINT, "
		"GENRE INT, "
		"DATE_USED BIGINT, "
		"NUM_PLAYED INT, "
		"FOREIGN KEY(ALBUM) REFERENCES ALBUMS(ID))";
		"FOREIGN KEY(GENRE) REFERENCES GENRES(ID))";
	error = sqlite3_get_table(mDB, query.c_str(), &resTable, &numRows, &numCols, &errorMsg);
	sqlite3_free_table(resTable);
	if (error) MessageBox(0, errorMsg, "Error creating database table: TRACKS", 0);

	query = "CREATE TABLE PLAYLISTS (ID INTEGER PRIMARY KEY, "
		"NAME VARCHAR(255), "
		"FILENAME VARCHAR(255))";
	error |= sqlite3_get_table(mDB, query.c_str(), &resTable, &numRows, &numCols, &errorMsg);
	sqlite3_free_table(resTable);
	if (error) MessageBox(0, errorMsg, "Error creating database table: PLAYLISTS", 0);

	query = "CREATE TABLE PLAYLIST_TRACKS (ID INTEGER PRIMARY KEY, "
		"TRACK INTEGER, "
		"PLAYLIST INTEGER, "
		"T_INDEX INTEGER, "
		"FOREIGN KEY(TRACK) REFERENCES TRACKS(ID), "
		"FOREIGN KEY(PLAYLIST) REFERENCES PLAYLISTS(ID))";
	error = sqlite3_get_table(mDB, query.c_str(), &resTable, &numRows, &numCols, &errorMsg);
	sqlite3_free_table(resTable);
	if (error) MessageBox(0, errorMsg, "Error creating database table: PLAYLIST_TRACKS", 0);


	query = "CREATE TABLE MUSIC_DIRECTORIES (ID INTEGER PRIMARY KEY, "
		"PATH VARCHAR(255))";
	error |= sqlite3_get_table(mDB, query.c_str(), &resTable, &numRows, &numCols, &errorMsg);
	sqlite3_free_table(resTable);
	if (error) MessageBox(0, errorMsg, "Error creating database table: PLAYLISTS", 0);

	query = "CREATE TABLE DB_DEFAULTS (ID INTEGER PRIMARY KEY, "
		"NAME VARCHAR(255), "
		"STR_VALUE VARCHAR(255), "
		"INT_VALUE INTEGER)";
	error |= sqlite3_get_table(mDB, query.c_str(), &resTable, &numRows, &numCols, &errorMsg);
	sqlite3_free_table(resTable);
	if (error) MessageBox(0, errorMsg, "Error creating database table: DB_DEFAULTS", 0);

	addGenre(Genre(DatabaseStructs::DEF_EMPTY_GENRE, 0));
	lock.Unlock();
}
void DatabaseManager::shutdown()
{
	if (mInstance != NULL)
	{
		delete mInstance;
		mInstance = NULL;
	}
}

void DatabaseManager::beginTransaction()
{
	if (mInTransaction)
	{
		endTransaction();
	}
	sqlite3_exec(mDB, "BEGIN TRANSACTION", NULL, NULL, NULL);
	mInTransaction = true;
}
void DatabaseManager::endTransaction()
{
	if (mInTransaction)
	{
		sqlite3_exec(mDB, "COMMIT TRANSACTION", NULL, NULL, NULL);
		mInTransaction = false;
	}
}

void DatabaseManager::getGenre(int id, Genre* genre)
{
	CSingleLock lock(&mCritSection, true);

	genre->Id = -1;
	sqlite3_stmt* stmt = NULL;

	sqlite3_prepare_v2(mDB, "SELECT ID, TITLE, STANDARD_ID FROM GENRES WHERE ID = ?", -1, &stmt, NULL);
	sqlite3_bind_int(stmt, 1, id);

	int result = sqlite3_step(stmt); 

	if ( result == SQLITE_ROW)
	{
		genre->Id = getIntColumn(stmt, 0);
		genre->Title = getStringColumn(stmt, 1);
		genre->StandardId = getIntColumn(stmt, 2);
	}
	sqlite3_finalize(stmt);
	lock.Unlock();
}
void DatabaseManager::getGenre(string title, Genre* genre)
{
	CSingleLock lock(&mCritSection, true);
	genre->Id = DatabaseStructs::INVALID_ID;
	sqlite3_stmt* stmt = NULL;

	sqlite3_prepare_v2(mDB, "SELECT ID, TITLE, STANDARD_ID FROM GENRES WHERE TITLE = ?", -1, &stmt, NULL);
	sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_STATIC);

	int result = sqlite3_step(stmt); 

	if ( result == SQLITE_ROW)
	{
		genre->Id = getIntColumn(stmt, 0);
		genre->Title = getStringColumn(stmt, 1);
		genre->StandardId = getIntColumn(stmt, 2);
	}
	sqlite3_finalize(stmt);
	lock.Unlock();
}
// this will fill in the id of new (or existing) genre 
void DatabaseManager::addGenre(Genre &genre)
{
	Genre existing;
	getGenre(genre.Title, &existing);
	
	// only add if it doesn't exist
	if (existing.Id == DatabaseStructs::INVALID_ID)
	{
		CSingleLock lock(&mCritSection, true);
		sqlite3_stmt* stmt = NULL;

		sqlite3_prepare_v2(mDB, "INSERT INTO GENRES (TITLE, STANDARD_ID) VALUES (?, ?)", -1, &stmt, NULL);
		sqlite3_bind_text(stmt, 1, genre.Title.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 2, genre.StandardId);

		int index = addRow(stmt); 

		if (index != DatabaseStructs::INVALID_ID)
		{
			genre.Id = index;
		}
		sqlite3_finalize(stmt);
		lock.Unlock();
	}
	else 
	{
		genre.Id = existing.Id;
	}
}

void DatabaseManager::getTrack(int id, Track* track)
{
	CSingleLock lock(&mCritSection, true);
	track->Id = DatabaseStructs::INVALID_ID;
	sqlite3_stmt* stmt = NULL;

	sqlite3_prepare_v2(mDB, "SELECT ID, FILENAME, TITLE, ARTIST, TRACK_NUMBER, ALBUM, LENGTH, DATE_ADDED, "
		"IGNORED, GENRE, DATE_USED, NUM_PLAYED FROM TRACKS WHERE ID = ?", -1, &stmt, NULL);
	sqlite3_bind_int(stmt, 1, id);

	int result = sqlite3_step(stmt); 

	if ( result == SQLITE_ROW)
	{
		track->Id = getIntColumn(stmt, 0);
		track->Filename = getStringColumn(stmt, 1);
		track->Title = getStringColumn(stmt, 2);
		track->Artist = getStringColumn(stmt, 3);
		track->TrackNumber = getIntColumn(stmt, 4);
		track->AlbumId = getIntColumn(stmt, 5);
		track->Length = getIntColumn(stmt, 6);
		track->DateAdded = getLongColumn(stmt, 7);
		track->Ignored = getBoolColumn(stmt, 8);
		track->Genre = getIntColumn(stmt, 9);
		track->DateUsed = getLongColumn(stmt, 10);
		track->NumPlayed = getIntColumn(stmt, 11);
	}
	sqlite3_finalize(stmt);
	lock.Unlock();
}

void DatabaseManager::getTrack(string &filename, Track* track)
{
	CSingleLock lock(&mCritSection, true);
	track->Id = DatabaseStructs::INVALID_ID;
	sqlite3_stmt* stmt = NULL;

	sqlite3_prepare_v2(mDB, "SELECT ID, FILENAME, TITLE, ARTIST, TRACK_NUMBER, ALBUM, LENGTH, DATE_ADDED, "
		"IGNORED, GENRE, DATE_USED, NUM_PLAYED FROM "
		"TRACKS WHERE FILENAME = ?", -1, &stmt, NULL);
	sqlite3_bind_text(stmt, 1, filename.c_str(), -1, SQLITE_STATIC);

	int result = sqlite3_step(stmt); 

	if ( result == SQLITE_ROW)
	{
		track->Id = getIntColumn(stmt, 0);
		track->Filename = getStringColumn(stmt, 1);
		track->Title = getStringColumn(stmt, 2);
		track->Artist = getStringColumn(stmt, 3);
		track->TrackNumber = getIntColumn(stmt, 4);
		track->AlbumId = getIntColumn(stmt, 5);
		track->Length = getIntColumn(stmt, 6);
		track->DateAdded = getLongColumn(stmt, 7);
		track->Ignored = getBoolColumn(stmt, 8);
		track->Genre = getIntColumn(stmt, 9);
		track->DateUsed = getLongColumn(stmt, 10);
		track->NumPlayed = getIntColumn(stmt, 11);
	}
	sqlite3_finalize(stmt);
	lock.Unlock();
}

vector<Track*> DatabaseManager::getTracks(Album &album)
{
	CSingleLock lock(&mCritSection, true);
	sqlite3_stmt* stmt = NULL;

	sqlite3_prepare_v2(mDB, "SELECT ID, FILENAME, TITLE, ARTIST, TRACK_NUMBER, ALBUM, LENGTH, DATE_ADDED, "
		"IGNORED, GENRE, DATE_USED, NUM_PLAYED FROM TRACKS WHERE ALBUM=? ORDER BY TRACK_NUMBER", -1, &stmt, NULL);
	sqlite3_bind_int(stmt, 1, album.Id);

	vector<Track*> tracks;
	int result = sqlite3_step(stmt); 

	while (result == SQLITE_ROW)
	{
		Track* track = snew Track();
		track->Id = getIntColumn(stmt, 0);
		track->Filename = getStringColumn(stmt, 1);
		track->Title = getStringColumn(stmt, 2);
		track->Artist = getStringColumn(stmt, 3);
		track->TrackNumber = getIntColumn(stmt, 4);
		track->AlbumId = getIntColumn(stmt, 5);
		track->Length = getIntColumn(stmt, 6);
		track->DateAdded = getLongColumn(stmt, 7);
		track->Ignored = getBoolColumn(stmt, 8);
		track->Genre = getIntColumn(stmt, 9);
		track->DateUsed = getLongColumn(stmt, 10);
		track->NumPlayed = getIntColumn(stmt, 11);

		tracks.push_back(track);

		result = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	lock.Unlock();
	return tracks;
}

map<string, Track*> DatabaseManager::getAllTracks()
{
	CSingleLock lock(&mCritSection, true);
	sqlite3_stmt* stmt = NULL;

	sqlite3_prepare_v2(mDB, "SELECT ID, FILENAME, TITLE, ARTIST, TRACK_NUMBER, ALBUM, LENGTH, DATE_ADDED, "
		"IGNORED, GENRE, DATE_USED, NUM_PLAYED FROM TRACKS", -1, &stmt, NULL);

	map<string, Track*> tracks;
	int result = sqlite3_step(stmt); 

	while (result == SQLITE_ROW)
	{
		Track* track = snew Track();
		track->Id = getIntColumn(stmt, 0);
		track->Filename = getStringColumn(stmt, 1);
		track->Title = getStringColumn(stmt, 2);
		track->Artist = getStringColumn(stmt, 3);
		track->TrackNumber = getIntColumn(stmt, 4);
		track->AlbumId = getIntColumn(stmt, 5);
		track->Length = getIntColumn(stmt, 6);
		track->DateAdded = getLongColumn(stmt, 7);
		track->Ignored = getBoolColumn(stmt, 8);
		track->Genre = getIntColumn(stmt, 9);
		track->DateUsed = getLongColumn(stmt, 10);
		track->NumPlayed = getIntColumn(stmt, 11);

		tracks[track->Filename] = track;

		result = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	lock.Unlock();
	return tracks;
}

// this will fill in the id of new (or existing) track
void DatabaseManager::addTrack(Track& track)
{
	Track existing;
	getTrack(track.Filename, &existing);
	
	// only add if it doesn't exist
	if (existing.Id == DatabaseStructs::INVALID_ID)
	{
		CSingleLock lock(&mCritSection, true);
		sqlite3_stmt* stmt = NULL;

		sqlite3_prepare_v2(mDB, "INSERT INTO TRACKS (FILENAME, TITLE, ARTIST, TRACK_NUMBER, ALBUM, LENGTH, "
			"DATE_ADDED, IGNORED, GENRE, DATE_USED, NUM_PLAYED) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", -1, &stmt, NULL);
		
		sqlite3_bind_text(stmt, 1, track.Filename.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, track.Title.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 3, track.Artist.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 4, track.TrackNumber);
		if (track.AlbumId <= -1)
		{
			sqlite3_bind_null(stmt, 5);
		}
		else
		{
			sqlite3_bind_int(stmt, 5, track.AlbumId);
		}
		sqlite3_bind_int64(stmt, 6, track.Length);
		sqlite3_bind_int64(stmt, 7, track.DateAdded);
		sqlite3_bind_int(stmt, 8, track.Ignored ? 1: 0);
		if (track.Genre <= DatabaseStructs::INVALID_ID)
		{
			sqlite3_bind_null(stmt, 9);
		}
		else
		{
			sqlite3_bind_int(stmt, 9, track.Genre);
		}
		sqlite3_bind_int64(stmt, 10, track.DateUsed);
		sqlite3_bind_int(stmt, 11, track.NumPlayed);

		int index = addRow(stmt);
		sqlite3_finalize(stmt);

		lock.Unlock();

		if (index != DatabaseStructs::INVALID_ID)
		{
			track.Id = index;
		}
	}
	else 
	{
		track.Id = existing.Id;
		track.Filename = existing.Filename;
		track.Title = existing.Title;
		track.Artist = existing.Artist;
		track.TrackNumber = existing.TrackNumber;
		track.AlbumId = existing.AlbumId;
		track.Length = existing.Length;
	}
}
// tries to update according to id, then by path.
bool DatabaseManager::modifyTrack(Track& track)
{
	int trackId = track.Id;
	if (trackId == DatabaseStructs::INVALID_ID)
	{
		Track existing;
		getTrack(track.Filename, &existing);
		trackId = existing.Id;
	}	
	// only update if it exists
	if (trackId != DatabaseStructs::INVALID_ID)
	{
		CSingleLock lock(&mCritSection, true);
		sqlite3_stmt* stmt = NULL;

		sqlite3_prepare_v2(mDB, "UPDATE TRACKS SET FILENAME=?, TITLE=?, "
				"ARTIST=?, TRACK_NUMBER=?, ALBUM=?, LENGTH=?, DATE_ADDED=?, "
				"IGNORED=?, GENRE=?, DATE_USED=?, NUM_PLAYED=? WHERE ID=?", -1, &stmt, NULL);
		
		sqlite3_bind_text(stmt, 1, track.Filename.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, track.Title.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 3, track.Artist.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 4, track.TrackNumber);
		if (track.AlbumId <= DatabaseStructs::INVALID_ID) 
		{
			sqlite3_bind_null(stmt, 5);
		}
		else
		{
			sqlite3_bind_int(stmt, 5, track.AlbumId);
		}
		sqlite3_bind_int(stmt, 6, track.Length);
		sqlite3_bind_int64(stmt, 7, track.DateAdded);
		sqlite3_bind_int(stmt, 8, track.Ignored ? 1 : 0);
		if (track.Genre <= DatabaseStructs::INVALID_ID) 
		{
			sqlite3_bind_null(stmt, 9);
		}
		else
		{
			sqlite3_bind_int(stmt, 9, track.Genre);
		}
		sqlite3_bind_int64(stmt, 10, track.DateUsed);
		sqlite3_bind_int(stmt, 11, track.NumPlayed);
		sqlite3_bind_int(stmt, 12, trackId);
		int result = sqlite3_step(stmt);

		sqlite3_finalize(stmt);
		lock.Unlock();
		
		if (result != SQLITE_DONE && result != SQLITE_ROW && result != SQLITE_OK)
		{
			return false;
		}
	}
	else
	{
		return false;
	}
	return true;
}
void DatabaseManager::getAlbum(int id, Album* album)
{
	album->Id = DatabaseStructs::INVALID_ID;
	CSingleLock lock(&mCritSection, true);
	album->Id = DatabaseStructs::INVALID_ID;
	sqlite3_stmt* stmt = NULL;

	sqlite3_prepare_v2(mDB, "SELECT NUM_TRACKS, TITLE, YEAR, ARTIST FROM ALBUMS WHERE ID=? ORDER BY ARTIST, TITLE", -1, &stmt, NULL);
	sqlite3_bind_int(stmt, 1, id);

	int result = sqlite3_step(stmt); 

	if ( result == SQLITE_ROW)
	{
		album->Id = id;
		album->NumTracks = getIntColumn(stmt, 0);
		album->Title = getStringColumn(stmt, 1);
		album->Year = getIntColumn(stmt, 2);
		album->Artist = getStringColumn(stmt, 3);
	}
	sqlite3_finalize(stmt);
	lock.Unlock();
}

void DatabaseManager::getAlbum(string title, int year, Album* album)
{
	CSingleLock lock(&mCritSection, true);
	album->Id = DatabaseStructs::INVALID_ID;
	sqlite3_stmt* stmt = NULL;

	sqlite3_prepare_v2(mDB, "SELECT ID, NUM_TRACKS, ARTIST FROM ALBUMS WHERE TITLE=? AND YEAR=?", -1, &stmt, NULL);
	sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 2, year);

	int result = sqlite3_step(stmt); 

	if ( result == SQLITE_ROW)
	{
		album->Id = getIntColumn(stmt, 0);
		album->NumTracks = getIntColumn(stmt, 1);
		album->Title = title;
		album->Year = year;
		album->Artist = getStringColumn(stmt, 2);
	}
	sqlite3_finalize(stmt);
	lock.Unlock();
}
// this will fill in the id of new (or existing) album
void DatabaseManager::addAlbum(Album &album)
{
	Album existing;
	getAlbum(album.Title, album.Year, &existing);
	
	// only add if it doesn't exist
	if (existing.Id == DatabaseStructs::INVALID_ID)
	{
		CSingleLock lock(&mCritSection, true);
		sqlite3_stmt* stmt = NULL;

		sqlite3_prepare_v2(mDB, "INSERT INTO ALBUMS (NUM_TRACKS, TITLE, YEAR, ARTIST) VALUES (?, ?, ?, ?)", -1, &stmt, NULL);
		sqlite3_bind_int(stmt, 1, album.NumTracks);
		sqlite3_bind_text(stmt, 2, album.Title.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 3, album.Year);
		sqlite3_bind_text(stmt, 4, album.Artist.c_str(), -1, SQLITE_STATIC);

		int index = addRow(stmt);
		sqlite3_finalize(stmt);

		lock.Unlock();

		if (index != DatabaseStructs::INVALID_ID)
		{
			album.Id = index;
		}
	}
	else 
	{
		album.Id = existing.Id;
		album.NumTracks = existing.NumTracks;
		album.Title = existing.Title;
		album.Year = existing.Year;
		album.Artist = existing.Artist;
	}
}

vector<Album*> DatabaseManager::getAllAlbums()
{
	CSingleLock lock(&mCritSection, true);
	sqlite3_stmt* stmt = NULL;

	sqlite3_prepare_v2(mDB, "SELECT ID, NUM_TRACKS, TITLE, YEAR, ARTIST FROM ALBUMS ORDER BY ARTIST", -1, &stmt, NULL);

	vector<Album*> albums;
	int result = sqlite3_step(stmt); 

	while (result == SQLITE_ROW)
	{
		Album* album = snew Album();
		album->Id = getIntColumn(stmt, 0);
		album->NumTracks = getIntColumn(stmt, 1);
		album->Title = getStringColumn(stmt, 2);
		album->Year = getIntColumn(stmt, 3);
		album->Artist = getStringColumn(stmt, 4);
		
		albums.push_back(album);

		result = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	lock.Unlock();
	return albums;
}

void DatabaseManager::addDir(std::string &path)
{
	// only add if it doesn't exist
	if (!dirExists(path))
	{
		CSingleLock lock(&mCritSection, true);
		sqlite3_stmt* stmt = NULL;

		sqlite3_prepare_v2(mDB, "INSERT INTO MUSIC_DIRECTORIES (PATH) VALUES (?)", -1, &stmt, NULL);
		sqlite3_bind_text(stmt, 1, path.c_str(), -1, SQLITE_STATIC);

		int index = addRow(stmt);
		sqlite3_finalize(stmt);

		lock.Unlock();
	}
}
bool DatabaseManager::dirExists(std::string &path)
{
	CSingleLock lock1(&mCritSection, true);
	sqlite3_stmt* stmt1 = NULL;

	sqlite3_prepare_v2(mDB, "SELECT PATH FROM MUSIC_DIRECTORIES WHERE PATH=?", -1, &stmt1, NULL);
	sqlite3_bind_text(stmt1, 1, path.c_str(), -1, SQLITE_STATIC);

	int result = sqlite3_step(stmt1); 

	bool exists = false;
	if ( result == SQLITE_ROW)
	{
		exists = true;
	}
	sqlite3_finalize(stmt1);
	lock1.Unlock();

	return exists;
}

vector<std::string> DatabaseManager::getAllDirs()
{
	CSingleLock lock(&mCritSection, true);
	sqlite3_stmt* stmt = NULL;

	sqlite3_prepare_v2(mDB, "SELECT PATH FROM MUSIC_DIRECTORIES", -1, &stmt, NULL);

	vector<std::string> paths;
	int result = sqlite3_step(stmt); 

	while (result == SQLITE_ROW)
	{
		paths.push_back(getStringColumn(stmt, 0));

		result = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
	lock.Unlock();
	return paths;
}

DBDefault DatabaseManager::getDefault(string name)
{
	CSingleLock lock(&mCritSection, true);
	DBDefault d;
	sqlite3_stmt* stmt = NULL;

	sqlite3_prepare_v2(mDB, "SELECT ID, NAME, STR_VALUE, INT_VALUE FROM "
		"DB_DEFAULTS WHERE NAME = ?", -1, &stmt, NULL);

	sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);

	int result = sqlite3_step(stmt); 

	if (result == SQLITE_ROW)
	{
		d.Id = getIntColumn(stmt, 0);
		d.Name = getStringColumn(stmt, 1);
		d.StrVal= getStringColumn(stmt, 2);
		d.IntVal = getIntColumn(stmt, 3);
	}
	sqlite3_finalize(stmt);
	lock.Unlock();
	return d;
}
DBDefault DatabaseManager::getDefault(string name, string defaultStrValue, int defaultIntValue)
{
	DBDefault def = getDefault(name);

	// add if doesn't exist
	if (def.Id == DatabaseStructs::INVALID_ID)
	{
		def.Name = name;
		def.StrVal = defaultStrValue;
		def.IntVal = defaultIntValue;
		addDefault(def);
	}
	return def;
}

// if name already exists, it will be overwritten!
void DatabaseManager::addDefault(DBDefault &def)
{
	DBDefault existing = getDefault(def.Name);
	
	CSingleLock lock(&mCritSection, true);

	// only add if it doesn't exist
	if (existing.Id == DatabaseStructs::INVALID_ID)
	{
		sqlite3_stmt* stmt = NULL;

		sqlite3_prepare_v2(mDB, "INSERT INTO DB_DEFAULTS (NAME, STR_VALUE, INT_VALUE) "
			"VALUES (?, ?, ?)", -1, &stmt, NULL);

		sqlite3_bind_text(stmt, 1, def.Name.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, def.StrVal.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 3, def.IntVal);

		int index = addRow(stmt);
		sqlite3_finalize(stmt);

		if (index != DatabaseStructs::INVALID_ID)
		{
			def.Id = index;
		}
	}
	else // modify existing
	{
		sqlite3_stmt* stmt = NULL;

		sqlite3_prepare_v2(mDB, "UPDATE DB_DEFAULTS SET STR_VALUE=?, INT_VALUE=? where id=?", -1, &stmt, NULL);

		sqlite3_bind_text(stmt, 1, def.StrVal.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 2, def.IntVal);
		sqlite3_bind_int(stmt, 3, existing.Id);

		sqlite3_step(stmt);
		sqlite3_finalize(stmt);

		def.Id = existing.Id;
	}
	lock.Unlock();

}
// if name doesn't exist, it's created
void DatabaseManager::modifyDefault(DBDefault &def)
{
	addDefault(def);
}

int DatabaseManager::addRow(sqlite3_stmt* stmt)
{
	int id = DatabaseStructs::INVALID_ID;
	int result = sqlite3_step(stmt); 

	if ( result == SQLITE_DONE)
	{
		id = (int)sqlite3_last_insert_rowid(mDB);
	}
	return id;
}
std::string DatabaseManager::getStringColumn(sqlite3_stmt* stmt, int column)
{
	std::string retVal = "";
	const unsigned char* data = sqlite3_column_text(stmt, column);
	if (data != NULL)
	{
		retVal = std::string(reinterpret_cast<const char*>(data));
	}
	return retVal;
}
int DatabaseManager::getIntColumn(sqlite3_stmt* stmt, int column)
{
	int retVal = DatabaseStructs::INVALID_ID;
	const unsigned char* data = sqlite3_column_text(stmt, column);
	if (data != NULL)
	{
		retVal = sqlite3_column_int(stmt, column);
	}
	return retVal;
}
long DatabaseManager::getLongColumn(sqlite3_stmt* stmt, int column)
{
	long retVal = -1;
	const unsigned char* data = sqlite3_column_text(stmt, column);
	if (data != NULL)
	{
		retVal = (long)sqlite3_column_int64(stmt, column);
	}
	return retVal;
}
double DatabaseManager::getDoubleColumn(sqlite3_stmt* stmt, int column)
{
	double retVal = -1;
	const unsigned char* data = sqlite3_column_text(stmt, column);
	if (data != NULL)
	{
		retVal = sqlite3_column_double(stmt, column);
	}
	return retVal;
}

bool DatabaseManager::getBoolColumn(sqlite3_stmt* stmt, int column)
{
	bool retVal = false;
	const unsigned char* data = sqlite3_column_text(stmt, column);
	if (data != NULL)
	{
		int num = sqlite3_column_int(stmt, column);
		retVal = (num != 0);
	}
	return retVal;
}

void DatabaseManager::doQuery(const char *query)
{
	char** resTable;
	int numRows, numCols;
	char *errorMsg;

	int error = sqlite3_get_table(mDB, query, &resTable, &numRows, &numCols, &errorMsg);

	sqlite3_free_table(resTable);
}
DatabaseManager* DatabaseManager::instance()
{
	if (mInstance == NULL)
	{
		mInstance = snew DatabaseManager();
	}
	return mInstance;
}