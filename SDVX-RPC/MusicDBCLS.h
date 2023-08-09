#ifndef MUSIC_DBCLS_H
#define MUSIC_DBCLS_H

#include <string>
#include <vector>
#include "sqlite/sqlite3.h"

struct MusicInfo {
    int musicID;
    std::string musicTitle;
    std::string musicArtist;
};

class MusicDBCLS {
public:
    MusicDBCLS();
    MusicDBCLS(const std::string& databasePath);
    ~MusicDBCLS();
    MusicDBCLS& operator=(const MusicDBCLS& other);

    bool openDatabase();
    bool closeDatabase();
    bool getMusicInfoByID(int musicID, MusicInfo& musicInfo);

private:
    std::string databasePath_;
    sqlite3* database_;
};

#endif