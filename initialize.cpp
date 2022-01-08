#include <sqlite3.h>
#include <iostream>

static int callback(void* data, int argc, char **argv, char **azColName) {
    std::cout << (const char*)data << std::endl;
    for (int i = 0; i < argc; i++) {
        std::cout << std::string(azColName[i]) << " = " << ((argv[i]) ? std::string(argv[i]) : "NULL") << std::endl;
    }
    return 0;
}

int main(int argc, char ** argv) {
    sqlite3 *db;
    char *zErrMsg = 0;
    
    int res = sqlite3_open("./testing/database/test.db", &db);
    if (res) {
        throw std::invalid_argument("Database failed to open: " + std::string(sqlite3_errmsg(db)));
    }
    std::cout << "Database opened" << std::endl;

    std::string sql;

    sql = "CREATE TABLE directories(" \
        "directory_id   INTEGER     PRIMARY KEY AUTOINCREMENT," \
        "parent_id      INT," \
        "name           TEXT        NOT NULL," \
        "FOREIGN KEY(parent_id) REFERENCES directories(directory_id) ON DELETE CASCADE );";
    res = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
    if (res != SQLITE_OK ) {
        throw std::runtime_error("SQL error: \n" + std::string(zErrMsg));
    }
    std::cout << "Created directories database table successfully" << std::endl;
            
    sql = "CREATE TABLE collection(" \
        "collection_id  INT         PRIMARY KEY," \
        "title          TEXT        UNIQUE NOT NULL," \
        "sort_title     TEXT        UNIQUE NOT NULL," \
        "authors        TEXT," \
        "illistrators   TEXT," \
        "publisher      TEXT," \
        "generes        TEXT," \
        "number_vol     INT," \
        "number_iss     INT         NOT NULL," \
        "start_date     CHAR(10)    NOT NULL," \
        "end_date       CHAR(10),"
        "file_loc       TEXT        NOT NULL );";
    res = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
    if (res != SQLITE_OK ) {
        throw std::runtime_error("SQL error: \n" + std::string(zErrMsg));
    }
    std::cout << "Created collection database table successfully" << std::endl;
            
    sql = "CREATE TABLE media(" \
        "media_id       INTEGER     PRIMARY KEY AUTOINCREMENT," \
        "title          TEXT        NOT NULL," \
        "sort_title     TEXT        NOT NULL," \
        "volume_num     INT," \
        "issue_num      INT," \
        "isbn           CHAR(18)    UNIQUE," \
        "total_pages    INT         NOT NULL," \
        "date           CHAR(10)," \
        "authors        TEXT," \
        "illistrators   TEXT," \
        "publisher      TEXT," \
        "generes        TEXT," \
        "type           TEXT        NOT NULL," \
        "collection_ID  INT," \
        "filename       TEXT        NOT NULL," \
        "file_loc       TEXT        NOT NULL," \
        "FOREIGN KEY(collection_id) REFERENCES collection(collection_id) ON DELETE SET NULL," \
        "FOREIGN KEY(file_loc) REFERENCES directories(directory_id) );"; // maybe have it CASCADE -> could be bad though
    res = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
    if (res != SQLITE_OK ) {
        throw std::runtime_error("SQL error: \n" + std::string(zErrMsg));
    }
    std::cout << "Created media database table successfully" << std::endl;
            
    sql = "CREATE TABLE users(" \
        "user_id        INTEGER     PRIMARY KEY AUTOINCREMENT," \
        "username       CHAR(16)    UNIQUE NOT NULL," \
        "password_hash  CHAR(16)    UNIQUE NOT NULL," \
        "email          TEXT        UNIQUE," \
        "privilages     CHAR(5)     NOT NULL," \
        "enabled        BOOLEAN     NOT NULL );";
    res = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
    if (res != SQLITE_OK ) {
        throw std::runtime_error("SQL error: \n" + std::string(zErrMsg));
    }
    std::cout << "Created users database table successfully" << std::endl;

    sql = "CREATE TABLE progress(" \
        "user_id        INT         PRIMARY KEY," \
        "FOREIGN KEY(user_id) REFERENCES users(user_id) ON DELETE CASCADE );";
    res = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
    if (res != SQLITE_OK ) {
        throw std::runtime_error("SQL error: \n" + std::string(zErrMsg));
    }
    std::cout << "Created progress database table successfully" << std::endl;

    // remove after root is added
    std::cout << "Populating Database" << std::endl;
    sql = "INSERT INTO directories (name)" \
        "VALUES ('media');" \
        "INSERT INTO users (username, password_hash, privilages, enabled)" \
        "VALUES ('root', '0', 'admin', 1);";
    res = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
    if (res != SQLITE_OK ) {
        throw std::runtime_error("SQL error: \n" + std::string(zErrMsg));
    }

    sql = "INSERT INTO directories (parent_id, name)" \
        "VALUES (1, 'testing');" \
        "INSERT INTO directories (parent_id, name)" \
        "VALUES (1, 'another');" \
        "INSERT INTO directories (parent_id, name)" \
        "VALUES (2, 'nesting');" \
        "INSERT INTO media (title, sort_title, total_pages, type, filename, file_loc)" \
        "VALUES ('The Beginners Guide to Bycicle Commuting', 'Beginners Guide to Bycicle Commuting', 52, 'book', 'The Beginners Guide to Bycicle Commuting.pdf', 4);" \
        "INSERT INTO media (title, sort_title, total_pages, type, filename, file_loc)" \
        "VALUES ('Bash Command Line and Shell Scripts Pocket Primer', 'Bash Command Line and Shell Scripts Pocket Primer', 411, 'book', 'Bash Command Line and Shell Scripts Pocket Primer.epub', 2);" \
        "INSERT INTO media (title, sort_title, volume_num, total_pages, type, filename, file_loc)" \
        "VALUES ('Vagabond', 'Vagabond', 1, 241, 'manga', 'Vagabond - vol01.cbz', 2);" \
        "INSERT INTO media (title, sort_title, total_pages, type, filename, file_loc)" \
        "VALUES ('Transformers Historia', 'Transformers Historia', 60, 'comic', 'Transformers Historia Oneshot.cbz', 3);";
    res = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
    if (res != SQLITE_OK ) {
        throw std::runtime_error("SQL error: \n" + std::string(zErrMsg));
    }
    std::cout << "Populated database tables successfully" << std::endl;

    sqlite3_close(db);
    std::cout << "Database closed" << std::endl;
    return 0;
}