/*
This file is part of Nabu.

Nabu is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Nabu is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License along with Nabu. If not, see <https://www.gnu.org/licenses/>.
*/

/**
 * Initializes the database tables
 */

#include <sqlite3.h>
#include <iostream>

int main(int /* argc */, char ** /* argv */) {
    sqlite3 *db;
    char *zErrMsg = 0;
    
    int res = sqlite3_open("/appdata/database/nabu.db", &db);
    if (res) {
        throw std::invalid_argument("Database failed to open: " + std::string(sqlite3_errmsg(db)));
    }
    std::cout << "Database opened" << std::endl;

    std::string sql;

    sql = "CREATE TABLE directories(" \
        "directory_id   INTEGER     PRIMARY KEY AUTOINCREMENT," \
        "parent_id      INT," \
        "name           TEXT        UNIQUE NOT NULL," \
        "FOREIGN KEY(parent_id) REFERENCES directories(directory_id) ON DELETE CASCADE );";
    res = sqlite3_exec(db, sql.c_str(), 0, 0, &zErrMsg);
    if (res != SQLITE_OK ) {
        throw std::runtime_error("SQL error: \n" + std::string(zErrMsg));
    }
    std::cout << "Created directories database table successfully" << std::endl;
            
    sql = "CREATE TABLE collections(" \
        "collection_id  TEXT        PRIMARY KEY," \
        "title          TEXT        UNIQUE NOT NULL," \
        "sort_title     TEXT        UNIQUE NOT NULL," \
        "authors        TEXT," \
        "illistrators   TEXT," \
        "publisher      TEXT," \
        "generes        TEXT," \
        "number_vol     TINYINT," \
        "number_iss     TINYINT," \
        "start_date     CHAR(10)," \
        "end_date       CHAR(10),"\
        "cover          TEXT        NOT NULL);";
    res = sqlite3_exec(db, sql.c_str(), 0, 0, &zErrMsg);
    if (res != SQLITE_OK ) {
        throw std::runtime_error("SQL error: \n" + std::string(zErrMsg));
    }
    std::cout << "Created collections database table successfully" << std::endl;
            
    sql = "CREATE TABLE media(" \
        "media_id       INTEGER     PRIMARY KEY AUTOINCREMENT," \
        "title          TEXT        NOT NULL," \
        "sort_title     TEXT        NOT NULL," \
        "volume_num     INT," \
        "issue_num      INT," \
        "isbn           CHAR(18)    UNIQUE," \
        "total_pages    TINYINT     UNSIGNED NOT NULL," \
        "date           CHAR(10)," \
        "author         TEXT," \
        "illistrator    TEXT," \
        "publisher      TEXT," \
        "genere         TEXT," \
        "type           TEXT        NOT NULL," \
        "collection_id  TEXT," \
        "filename       TEXT        UNIQUE NOT NULL," \
        "file_loc       INT         NOT NULL," \
        "FOREIGN KEY(collection_id) REFERENCES collection(collection_id) ON DELETE SET NULL," \
        "FOREIGN KEY(file_loc) REFERENCES directories(directory_id) );"; // maybe have it CASCADE -> could be bad though
    res = sqlite3_exec(db, sql.c_str(), 0, 0, &zErrMsg);
    if (res != SQLITE_OK ) {
        throw std::runtime_error("SQL error: \n" + std::string(zErrMsg));
    }
    std::cout << "Created media database table successfully" << std::endl;
            
    sql = "CREATE TABLE users(" \
        "user_id        INTEGER     PRIMARY KEY AUTOINCREMENT," \
        "username       CHAR(16)    UNIQUE NOT NULL," \
        "password_hash  CHAR(16)    UNIQUE NOT NULL," \
        "email          TEXT        UNIQUE," \
        "privileges     CHAR(5)     NOT NULL," \
        "enabled        BOOLEAN     NOT NULL );";
    res = sqlite3_exec(db, sql.c_str(), 0, 0, &zErrMsg);
    if (res != SQLITE_OK ) {
        throw std::runtime_error("SQL error: \n" + std::string(zErrMsg));
    }
    std::cout << "Created users database table successfully" << std::endl;

    sql = "CREATE TABLE progress(" \
        "user_id        INT         PRIMARY KEY," \
        "FOREIGN KEY(user_id) REFERENCES users(user_id) ON DELETE CASCADE );";
    res = sqlite3_exec(db, sql.c_str(), 0, 0, &zErrMsg);
    if (res != SQLITE_OK ) {
        throw std::runtime_error("SQL error: \n" + std::string(zErrMsg));
    }
    std::cout << "Created progress database table successfully" << std::endl;

    std::cout << "Populating Database" << std::endl;
    std::hash<std::string> str_hash;
    sql = "INSERT INTO directories (name)" \
        "VALUES ('media');" \
        "INSERT INTO users (username, password_hash, privileges, enabled)" \
        "VALUES ('root', '" + std::to_string(str_hash("admin")) + "', 'admin', 1);";
    res = sqlite3_exec(db, sql.c_str(), 0, 0, &zErrMsg);
    if (res != SQLITE_OK ) {
        throw std::runtime_error("SQL error: \n" + std::string(zErrMsg));
    }

    std::cout << "Populated database tables successfully" << std::endl;

    sqlite3_close(db);
    std::cout << "Database closed" << std::endl;
    return 0;
}