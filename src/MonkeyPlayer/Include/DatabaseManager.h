// DatabaseManager.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Manages database interaction
// This is a singleton

#include <string>
#include <map>
#include <sqlite3.h>

#include "d3dUtil.h"
#include "DatabaseStructs.h"

#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

class DatabaseManager
{
public:
	static DatabaseManager* instance();
	~DatabaseManager();

	static void shutdown();

	void beginTransaction();
	void endTransaction();
	// genres
	void getGenre(int id, Genre* genre);
	void getGenre(string title, Genre *genre);
	void addGenre(Genre &genre);

	// tracks
	void getTrack(int id, Track* track);
	void getTrack(string &filename, Track* track);
	vector<Track*> getTracks(Album& album);
	map<string, Track*> getAllTracks();
	void addTrack(Track &track);
	bool modifyTrack(Track &track);

	// albums
	void getAlbum(int id, Album* album);
	void getAlbum(string title, int year, Album* album);
	void addAlbum(Album &album);
	vector<Album*> getAllAlbums();

	// directories
	void addDir(std::string &path);
	bool dirExists(std::string &path);
	vector<std::string> getAllDirs();

	// DB defaults
	DBDefault getDefault(string name);
	DBDefault getDefault(string name, string defaultStrValue, int defaultIntValue);
	void addDefault(DBDefault &def); // if name already exists, it will be overwritten!
	void modifyDefault(DBDefault &def);  // if name doesn't exist, it's created

private:
	DatabaseManager();
	void testAndValidateDatabase();
	void createDatabase();
	int addRow(const char *query);
	std::string getStringColumn(sqlite3_stmt* stmt, int column);
	int getIntColumn(sqlite3_stmt* stmt, int column);
	long getLongColumn(sqlite3_stmt* stmt, int column);
	double getDoubleColumn(sqlite3_stmt* stmt, int column);
	bool getBoolColumn(sqlite3_stmt* stmt, int column);
	int addRow(sqlite3_stmt* stmt);
	void doQuery(const char *query);
	static std::string sanitize(std::string &input);
	static DatabaseManager* mInstance;
	sqlite3* mDB;
	bool mInTransaction;

	// synchronization
	static CCriticalSection mCritSection;
};

#endif
