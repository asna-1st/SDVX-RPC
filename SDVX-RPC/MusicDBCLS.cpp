#include "MusicDBCLS.h"

MusicDBCLS::MusicDBCLS() : databasePath_(""), database_(nullptr) {}

MusicDBCLS::MusicDBCLS(const std::string& databasePath) : databasePath_(databasePath), database_(nullptr) {}

MusicDBCLS::~MusicDBCLS() {
    closeDatabase();
}

MusicDBCLS& MusicDBCLS::operator=(const MusicDBCLS& other) {
    if (this == &other) {
        return *this;
    }

    closeDatabase();

    databasePath_ = other.databasePath_;
    database_ = nullptr;

    return *this;
}

bool MusicDBCLS::openDatabase() {
    int rc = sqlite3_open(databasePath_.c_str(), &database_);
    return rc == SQLITE_OK;
}

bool MusicDBCLS::closeDatabase() {
    if (database_) {
        int rc = sqlite3_close(database_);
        database_ = nullptr;
        return rc == SQLITE_OK;
    }
    return false;
}

bool MusicDBCLS::getMusicInfoByID(int musicID, MusicInfo& musicInfo) {
    if (!database_) {
        return false;
    }

    std::string query = "SELECT musicTitle, musicArtist FROM musicDB WHERE musicID = " + std::to_string(musicID) + ";";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(database_, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return false;
    }

    musicInfo.musicID = musicID;
    musicInfo.musicTitle = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    musicInfo.musicArtist = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

    sqlite3_finalize(stmt);
    return true;
}
