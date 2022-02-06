/*
This file is part of Nabu.

Nabu is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Nabu is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License along with Nabu. If not, see <https://www.gnu.org/licenses/>.
*/

/**
 * @author  Andrew Mink
 * @date    6 February 2022
 */

#include <cppcms/json.h>

#include <mupdf/classes.h>
#include <sqlite3.h>
#include <filesystem>

#include "database.h"
#include "services.h"
#include "content.h"

namespace fs = std::filesystem;

namespace services {

    namespace database {
        sqlite3 *db;
        char *zErrMsg = 0;
        std::string docFilePath;
        std::string transferVal;
        
        /**
         * Opens the database with the given filepath
         * 
         * @param filepath the full file path, including the name, to the database file
         */
        void open(std::string const &filepath) {
            if (db) {
                sqlite3_close(db);
            }
            int res = sqlite3_open(filepath.c_str(), &db);
            if (res) {
                throw std::invalid_argument("Database failed to open: " + std::string(sqlite3_errmsg(db)));
            }
            Log("Database opened");
        } // open()

        /**
         * callback() is a generic callback for sql commands that does nothing.
         * All callback functions take the same paramaters
         * 
         * @param data provided in the 4th arg of sqlite3_exec()
         * @param argc number of columns in row
         * @param argv array of strings representing fields in the row
         * @param azColName array of strings representing column names
         */
        static int callback(void* /* data */, int /* argc */, char ** /* argv */, char ** /* azColName */) {
            return 0;
        } // callback()
        
        /**
         * getCallback() gets a single value from the database and sets it to transferVal
         * 
         * @param argv 0->any value to transfer
         */
        static int getCallback(void* /* data */, int argc, char **argv, char ** /* azColName */) {
            if (argc != 1) {
                Log("getCallback() `database query should contain 1 argument, but has: " + std::to_string(argc));
                throw std::invalid_argument("Database query should contain 1 argument, but has: " + std::to_string(argc));
            }
            transferVal = std::string(argv[0]);
            return 0;
        } // getCallback()

        /**
         * mediaCallback() creates a content::Item for each entry returned by the sql requests and then
         * populates glb_media with the Item
         * 
         * @param argv 0->media_id, 1->title, 2->sort_title, 3->volume_num, 4->issue_num
         */
        static int mediaCallback(void* /* data */, int argc, char **argv, char ** /* azColName */) {
            if (argc != 5) {
                Log("mediaCallback() database query should contain 5 arguments, but has: " + std::to_string(argc));
                throw std::invalid_argument("Database query should contain 5 arguments, but has: " + std::to_string(argc));
            }
            content::Item item;
            item.id             = std::string(argv[0]);
            item.title          = std::string(argv[1]);
            item.sortTitle      = std::string(argv[2]);
            item.volume         = ((argv[3]) ? std::string(argv[3]) : "" );
            item.issue          = ((argv[4]) ? std::string(argv[4]) : "" );
            item.progress       = 0; // get progress or 0
            item.isCollection   = false;

            addToMediaList(item);            
            return 0;
        } // mediaCallback()

        /**
         * collectionCallback() creates a content::Item for each entry returned by the sql requests and
         * the populates glb_media with the Item
         * 
         * @param argv 0->collection_id, 1->title, 2->sort_title, 3->number_vol, 4->number_iss, 5->cover
         */
        static int collectionCallback(void* /* data */, int argc, char **argv, char ** /* azColName */) {
            if (argc != 6) {
                Log("collectionCallback() database query should contain 6 arguments, but has: " + std::to_string(argc));
                throw std::invalid_argument("Database query should contain 6 arguments, but has: " + std::to_string(argc));
            }
            content::Item item;
            item.id             = std::string(argv[0]);
            item.title          = std::string(argv[1]);
            item.sortTitle      = std::string(argv[2]);
            item.volume         = ((argv[3]) ? std::string(argv[3]) : "" );
            item.issue          = ((argv[4]) ? std::string(argv[4]) : "" );
            item.cover          = std::string(argv[5]);
            item.progress       = 0; // get progress or 0
            item.isCollection   = true;

            addToMediaList(item);            
            return 0;
        } // collectionCallback()
        
        /**
         * walkDirectoriesCallback() recursivly calls itself as it walks backwards through a directroy path in
         * the directory database. Once the root directory has been reached, the path starts to be built with
         * the name of each directory on docFilePath.
         *
         * @param argv 0->parent_id, 1->name
         */
        static int walkDirectoriesCallback(void* /* data */, int argc, char **argv, char ** /* azColName */) {
            if (argc != 2) {
                Log("walkDirectoriesCallback() database query should contain 2 arguments, but has: " + std::to_string(argc));
                throw std::invalid_argument("Database query should contain 2 arguments, but has: " + std::to_string(argc));
            }

            if (argv[0]) {
                std::string sql;
                sql = "SELECT directories.parent_id, directories.name FROM directories WHERE directory_id =" + std::string(argv[0]) + ";";
                int res = sqlite3_exec(db, sql.c_str(), walkDirectoriesCallback, 0, &zErrMsg);
                if (res != SQLITE_OK) {
                    Log("SQL error: \n" + std::string(zErrMsg));
                    throw std::runtime_error("SQL error:\n" + std::string(zErrMsg));
                }
            }
            docFilePath.append(argv[1]);
            docFilePath.append("/");

            return 0;
        } // walkDirectoriesCallback()

        /**
         * fetchFileCallback() starts the call to walkDirectoriesCallback with the directory_id that the file is
         * contained in and once the filepath is built, the filename is appended to the end of the path to finish
         * off docFilePath for the book.
         * 
         * @param argv 0->file_loc, 1->filename
         */
        static int fetchFileCallback(void* /* data */, int argc, char **argv, char ** /* azColName */) {
            if (argc != 2) {
                Log("fetchFileCallback() database query should contain 2 arguments, but has: " + std::to_string(argc));
                throw std::invalid_argument("Database query should contain 2 arguments, but has: " + std::to_string(argc));
            }
            Log("Fetching path to '" + std::string(argv[1]) + "' from the database");
            
            std::string sql;
            sql = "SELECT directories.parent_id, directories.name FROM directories WHERE directory_id =" + std::string(argv[0]) + ";";
            int res = sqlite3_exec(db, sql.c_str(), walkDirectoriesCallback, 0, &zErrMsg);
            if (res != SQLITE_OK) {
                Log("SQL error: \n" + std::string(zErrMsg));
                throw std::runtime_error("SQL error:\n" + std::string(zErrMsg));
            }

            docFilePath.append(argv[1]);
            return 0;
        } // fetchFileCallback()

        /**
         * getMedia() clears glb_media before making a sql request for media that don't belong to a collection
         * and for all of the collections. The media request is sent to mediaCallback() and te collection request
         * is sent to collectionCallback() to handle the retrieved data and repopulate glb_media.
         */
        void getMedia() {
            Log("Getting books from database not in a collection");
            clearMediaList();
            std::string sql;
            sql = "SELECT media.media_id, media.title, media.sort_title, media.volume_num, media.issue_num FROM media WHERE collection_id IS NULL;";
            int res = sqlite3_exec(db, sql.c_str(), mediaCallback, 0, &zErrMsg);
            if (res != SQLITE_OK) {
                Log("SQL error: \n" + std::string(zErrMsg));
                throw std::runtime_error("SQL error: \n" + std::string(zErrMsg));
            }
            
            Log("Getting collections");
            sql = "SELECT collections.collection_id, collections.title, collections.sort_title, collections.number_vol, collections.number_iss, collections.cover FROM collections;";
            res = sqlite3_exec(db, sql.c_str(), collectionCallback, 0, &zErrMsg);
            if (res != SQLITE_OK) {
                Log("SQL error: \n" + std::string(zErrMsg));
                throw std::runtime_error("SQL error: \n" + std::string(zErrMsg));
            }
        } // getMedia()

        /**
         * getCollectionMedia() clears glb_media before making a sql request for the infomation of all books belonging
         * to a given collection. The request is sent to mediaCallback() for handling the information and rebuilding
         * glb_media.
         * 
         * @param id the id of the collection where the requested books belong
         */
        void getCollectionMedia(std::string const &collection_id) {
            Log("Getting books in the collection with id: " + collection_id);
            clearMediaList();
            std::string sql;
            sql = "SELECT media.media_id, media.title, media.sort_title, media.volume_num, media.issue_num FROM media WHERE collection_id='" + collection_id + "' ORDER BY media.issue_num DESC;";
            int res = sqlite3_exec(db, sql.c_str(), mediaCallback, 0, &zErrMsg);
            if (res != SQLITE_OK) {
                Log("SQL error: \n" + std::string(zErrMsg));
                throw std::runtime_error("SQL error: \n" + std::string(zErrMsg));
            }
        } // getCollectionMedia()

        /**
         * fetchFile() assigns docFilePath to '/' and then calls for the filename and parent directory with
         * fetchFileCallback where the rest of the filepath is constructed onto docFilePath before it is returned
         * 
         * @param media_id the id of the book whose file path is wanted
         * @return the finilaized docFilePath
         */
        std::string fetchFile(std::string const &media_id) {
            Log("Getting the file for the book with id: " + media_id);
            docFilePath = "/";
            std::string sql;
            sql = "SELECT media.file_loc, media.filename FROM media WHERE media_id=" + media_id + ";";
            int res = sqlite3_exec(db, sql.c_str(), fetchFileCallback, 0, &zErrMsg);
            if (res != SQLITE_OK) {
                Log("SQL error: \n" + std::string(zErrMsg));
                throw std::runtime_error("SQL error: \n" + std::string(zErrMsg));
            }

            return docFilePath;
        }

        /**
         * getDirectoryID() grabs the id for a given directory
         * 
         * @param name the name of the 
         * @return the id of the directory
         */
        std::string getDirectoryID(std::string const &name) {
            Log("Getting directory_id for " + name);
            transferVal.clear();

            std::string sql;
            sql = "SELECT directories.directory_id FROM directories WHERE name='" + name + "';";
            int res = sqlite3_exec(db, sql.c_str(), getCallback, 0, &zErrMsg);
            if (res != SQLITE_OK) {
                Log("SQL error: \n" + std::string(zErrMsg));
                throw std::runtime_error("SQL error: \n" + std::string(zErrMsg));
            }
            
            return transferVal;
        } // getDirectoryID()

        /**
         * addCollectionToDatabase() adds a new collection to the database
         *  
         * @param name the name of the collection
         * @return the id of the collection
         */
        std::string addCollectionToDatabase(std::string const &name) {
            Log("Making collection for " + name);

            std::string sortTitle = "";
            for (const auto& chr : name) {
                sortTitle += (char) tolower(chr);
            }
            if (name.find("the ") == 0) {
                sortTitle.erase(0,4);
            }

            std::hash<std::string> str_hash;
            size_t hashVal = str_hash(name);
            Log("Collection ID: " + std::to_string(hashVal));

            std::string sql = "INSERT INTO collections (collection_id, title, sort_title, number_vol, cover)" \
                "VALUES ('" + std::to_string(hashVal) + "', '" + name + "', '" + sortTitle + "', 1, 'default.png');";
            int res = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
            if (res != SQLITE_OK) {
                Log("SQL error: \n" + std::string(zErrMsg));
                throw std::runtime_error("SQL error: \n" + std::string(zErrMsg));
            }

            return std::to_string(hashVal);
        } // addCollectionToDatabase()

        /**
         * getCollectionVolumeCount() gets the number of volumes in the collection
         * 
         * @param name the collection title 
         * @return the number of volumes 
         */
        int getCollectionVolumeCount(std::string const &name) {
            Log("Getting number_vol for " + name);
            transferVal.clear();

            std::string sql = "SELECT collections.number_vol FROM collections WHERE title='" + name + "';";
            int res = sqlite3_exec(db, sql.c_str(), getCallback, 0, &zErrMsg);
            if (res != SQLITE_OK) {
                Log("SQL error: \n" + std::string(zErrMsg));
                throw std::runtime_error("SQL error: \n" + std::string(zErrMsg));
            }

            return std::stoi(transferVal);
        }

        /**
         * updateCollectionVolumeCount() increments the volume count by 1 for a collection
         * 
         * @param name the title of the collection
         */
        void updateCollectionVolumeCount(std::string const &name) {
            Log("Updating collection volume count for " + name);
            
            int newVolCount = getCollectionVolumeCount(name) + 1;
            std::string sql = "UPDATE collections SET number_vol=" + std::to_string(newVolCount) + " WHERE title='" + name + "';";
            int res = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
            if (res != SQLITE_OK) {
                Log("SQL error: \n" + std::string(zErrMsg));
                throw std::runtime_error("SQL error: \n" + std::string(zErrMsg));
            }
        }

        /**
         * getCollectionID() is used when adding a new book to the database with a collection.
         * It checks the database for if the collection with the given name exists, if it does
         * not exist the collection is created and the id is returned. If the collection does
         * exits, the id is grabbed
         * 
         * @param name the name of the collection
         * @param isAddingBoook true if adding a book to the database, false if otherwise
         * @return the id of the collection
         */
        std::string getCollectionID(std::string const &name, bool const &isAddingBook) {
            Log("Getting collection_id for " + name);

            std::string sql;
            struct sqlite3_stmt *selectstmt;
            sql = "SELECT * FROM collections WHERE title='" + name + "';";
            int res = sqlite3_prepare_v2(db, sql.c_str(), -1, &selectstmt, NULL);
            if (res != SQLITE_OK) {
                Log("SQL error: \n" + std::string(zErrMsg));
                throw std::runtime_error("SQL error: \n" + std::string(zErrMsg));
            }
            if (sqlite3_step(selectstmt) != SQLITE_ROW) {
                if (isAddingBook) {
                    return addCollectionToDatabase(name);
                } else {
                    Log("Collection " + name + " does not exist");
                    throw std::invalid_argument("Collection " + name + " does not exist");
                }
            }
            sqlite3_finalize(selectstmt);

            if (isAddingBook) {
                updateCollectionVolumeCount(name);
            }

            transferVal.clear();
            sql = "SELECT collections.collection_id FROM collections WHERE title='" + name + "';";
            res = sqlite3_exec(db, sql.c_str(), getCallback, 0, &zErrMsg);
            if (res != SQLITE_OK) {
                Log("SQL error: \n" + std::string(zErrMsg));
                throw std::runtime_error("SQL error: \n" + std::string(zErrMsg));
            }

            return transferVal;
        } // getCollectionID()

        /**
         * getMediaID() grabs the id for the book with the given filename
         * 
         * @param filename the name of the file for the book
         * @return the id of the book
         */
        std::string getMediaID(std::string const &filename) {
            Log("Getting media_id for " + filename);
            transferVal.clear();

            std::string sql;
            sql = "SELECT media.media_id FROM media WHERE filename='" + filename + "';";
            int res = sqlite3_exec(db, sql.c_str(), getCallback, 0, &zErrMsg);
            if (res != SQLITE_OK) {
                Log("SQL error: \n" + std::string(zErrMsg));
                throw std::runtime_error("SQL error: \n" + std::string(zErrMsg));
            }
            
            return transferVal;
        }

        /**
         * bookExists() checks the inputed id with the database
         * 
         * @param media_id the id to be checked
         * @return if the book was found or not
         */
        bool bookExists(std::string const &media_id) {
            Log("Checking if book exists with id: " + media_id);
            std::string sql;
            struct sqlite3_stmt *selectstmt;
            sql = "SELECT * FROM media WHERE media_id='" + media_id + "';";
            int res = sqlite3_prepare_v2(db, sql.c_str(), -1, &selectstmt, NULL);
            if (res != SQLITE_OK) {
                Log("SQL error: \n" + std::string(zErrMsg));
                throw std::runtime_error("SQL error: \n" + std::string(zErrMsg));
            }
            if (sqlite3_step(selectstmt) != SQLITE_ROW) {
                return false;
            }
            sqlite3_finalize(selectstmt);

            return true;
        }

        /**
         * getProgress() grabs the progress of the user for a book
         * 
         * @param username the name of the user
         * @param media_id id of the book
         * @return the progress
         */
        int getProgress(std::string const &username, std::string const &media_id) {
            // get the page count with session
            // not implemented yet
            return 0;

            Log("Getting progress for " + username + " with book id: " + media_id);
            transferVal.clear();
            
            std::string sql;
            sql = "SELECT progress.progress FROM progress WHERE ;";
            int res = sqlite3_exec(db, sql.c_str(), getCallback, 0, &zErrMsg);
            if (res != SQLITE_OK) {
                Log("SQL error: \n" + std::string(zErrMsg));
                throw std::runtime_error("SQL error: \n" + std::string(zErrMsg));
            }
            // check if null or at max page count
            return std::stoi(transferVal);
        }

        /**
         * moveFile()
         * 
         * @param newDirectoryPath the path to the new file starting after /media/
         * @param oldFilePath the path to the old file relative to import
         * @return the new file path
         */
        fs::path moveFile(std::string const &newDirectoryPath, fs::path const &oldFilePath, fs::path const &newFilename) {
            Log("Creating new directory path for /media/" + newDirectoryPath + newFilename.string());
            fs::path newDirPathRelativeToMedia("media/" + newDirectoryPath);
            fs::path newPath(getMediaPath());
            fs::path prevPart;
            for (const auto& part : newDirPathRelativeToMedia) {
                newPath.append(part.c_str());
                if (fs::create_directory(newPath)) {
                    Log("Created directory " + part.string() + ": adding it to the database");
                    std::string sqlTMP;
                    sqlTMP = "INSERT INTO directories (parent_id, name)" \
                        "VALUES (" + getDirectoryID(prevPart.c_str()) + ",'" + part.c_str() + "');";
                    int resTMP = sqlite3_exec(db, sqlTMP.c_str(), callback, 0, &zErrMsg);
                    if (resTMP != SQLITE_OK ) {
                        Log("SQL error: \n" + std::string(zErrMsg));
                        throw std::runtime_error("SQL error: \n" + std::string(zErrMsg));
                    }
                }
                prevPart = part;
            }
            newPath.append(newFilename.string());

            Log("Moving: " + newFilename.string());
            fs::rename(oldFilePath, newPath);

            return newPath;
        }

        /**
         * import() takes in the info for a book to be added to the database, gets a page count form the book,
         * generates a new filename, builds the new path to the file, moves the file, and then adds the book to
         * the database.
         * 
         * @param json contains the information about the book in json format
         */
        void import(cppcms::json::value json) {
            Log("Importing " + json.get<std::string>("title") + " into the database");

            // open document, get page count and make cover 
            fs::path oldFilePath(getImportPath() + json.get<std::string>("file")); // fix for recursive files
            if (!fs::exists(oldFilePath)) {
                Log("Can't find file: " + oldFilePath.string());
                throw std::runtime_error("Can't find file: " + oldFilePath.string());
            }
            mupdf::Document *doc = new mupdf::Document(oldFilePath.c_str());
            int totalPages = doc->count_pages();
            
            // build the new filename from the title and volume or issue of the book, keeping the same extension
            std::string newFilename = json.get<std::string>("title");
            if (!json.get<std::string>("volNum").empty()) {
                newFilename.append(" - vol" + json.get<std::string>("volNum"));
            } else if (!json.get<std::string>("issNum").empty()) {
                newFilename.append(" - iss" + json.get<std::string>("issNum"));
            }
            newFilename.append(oldFilePath.extension());

            fs::path newPath(moveFile(json.get<std::string>("title"), oldFilePath, newFilename));

            // Insert the book into the media database with all the new infomation
            Log("Adding to database");
            std::string sql;
            sql = "INSERT INTO media (title, sort_title, volume_num, issue_num, isbn, total_pages, date, author, illistrator, publisher, genere, type, collection_id, filename, file_loc)" \
                "VALUES ('"
                    + json.get<std::string>("title") + "','"
                    + json.get<std::string>("sortTitle") + "',"
                    + ((json.get<std::string>("volNum").empty()) ? "NULL" : "'" + json.get<std::string>("volNum") + "'" ) + ","
                    + ((json.get<std::string>("issNum").empty()) ? "NULL" : "'" + json.get<std::string>("issNum") + "'" ) + ","
                    + ((json.get<std::string>("isbn").empty()) ? "NULL" : "'" + json.get<std::string>("isbn") + "'" ) + ","
                    + std::to_string(totalPages) + ","
                    + ((json.get<std::string>("date").empty()) ? "NULL" : "'" + json.get<std::string>("date") + "'" ) + ","
                    + ((json.get<std::string>("author").empty()) ? "NULL" : "'" + json.get<std::string>("author") + "'" ) + ","
                    + ((json.get<std::string>("illistrator").empty()) ? "NULL" : "'" + json.get<std::string>("illistrator") + "'" ) + ","
                    + ((json.get<std::string>("publisher").empty()) ? "NULL" : "'" + json.get<std::string>("publisher") + "'" ) + ","
                    + ((json.get<std::string>("genere").empty()) ? "NULL" : "'" + json.get<std::string>("genere") + "'" ) + ",'"
                    + json.get<std::string>("type") + "',"
                    + ((json.get<std::string>("collection").empty()) ? "NULL" : "'" + getCollectionID(json.get<std::string>("collection"), true) + "'" ) + ",'"
                    + newFilename + "','"
                    + getDirectoryID(newPath.parent_path().filename()) +
                "');";
            int res = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
            if (res != SQLITE_OK) {
                Log("SQL error: \n" + std::string(zErrMsg));
                throw std::runtime_error("SQL error: \n" + std::string(zErrMsg));
            }

            // create the cover image
            std::string cover = getCoverPath() + getMediaID(newFilename) + ".png";
            Log("Creating cover image: " + cover);
            mupdf::Matrix ctm = mupdf::Matrix();
            doc->new_pixmap_from_page_number(0, ctm, mupdf::device_rgb(), 0)
                .save_pixmap_as_png(cover.c_str());
        } // import()

        /**
         * validateLogin() checks to see if the given credentials are valid
         * 
         * @param username the username to be validated
         * @param password the password in plain text to be validated
         * @return weither or not the user exists and is enabled
         */
        bool validateLogin(std::string const &username, std::string const &password) {
            Log("Validating user:  " + username);

            std::string sql;
            struct sqlite3_stmt *selectstmt;
            std::hash<std::string> str_hash;
            sql = "SELECT * FROM users WHERE username='" + username + "' AND password_hash='" + std::to_string(str_hash(password)) + "';";
            int res = sqlite3_prepare_v2(db, sql.c_str(), -1, &selectstmt, NULL);
            if (res != SQLITE_OK) {
                Log("SQL error: \n" + std::string(zErrMsg));
                throw std::runtime_error("SQL error: \n" + std::string(zErrMsg));
            }
            if (sqlite3_step(selectstmt) == SQLITE_ROW) {
                Log("User is valid");
                // check if user is enabled -> return false
                return true;
            }
            sqlite3_finalize(selectstmt);
            Log("User is disabled or doesn't exist");
            return false;
        }

        /**
         * getPermissions() takes a user and fetches the permissions for the user
         * 
         * @param username the username for the user
         * @return the permission of the user
         */
        std::string getPermissions(std::string const &username) {
            Log("Getting permissions for " + username);
            transferVal.clear();

            std::string sql;
            sql = "SELECT users.privileges FROM users WHERE username='" + username + "';";
            int res = sqlite3_exec(db, sql.c_str(), getCallback, 0, &zErrMsg);
            if (res != SQLITE_OK) {
                Log("SQL error: \n" + std::string(zErrMsg));
                throw std::runtime_error("SQL error: \n" + std::string(zErrMsg));
            }

            return transferVal;
        }
        
        /**
         * close() closes the database
         */
        void close() {
            sqlite3_close(db);
            Log("Closed database");
        } // close()
    
    } // namespace database
    
} // namespace services
