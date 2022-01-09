/**
 * @author  TacticalMinky
 * @version development
 * @date    8 January 2022
 * 
 * This file is everything, I tried the parts of the services namespace to each be in their own file,
 * but I was unabale to get it to run after compiling. I am still pretty new at making programs, so
 * it was probably something I was doing wrong. Because of that, I made a table of contents that I will
 * try to keep updated. I don't know why I am typing this, it is in a private repo, so no one will see
 * it but whatever.
 *  
 *                                          TABLE OF CONTENTS
 * page         Name
 *-----------------------------------------------------------------------------------------------------
 *  35      namespace services
 *  41          namespace database
 *  254         class ReadingRPC
 *  453         class ImportRPC
 *  479         class SettingsRPC
 *  491     class WebSite
 *  673     main()
 */

#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <cppcms/service.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/rpc_json.h>

#include <mupdf/classes.h>
#include <sqlite3.h>

#include "views/content.h"

namespace services {
    std::vector<content::Item> glb_media;
    
    /**
     * The database namespace contains the calls to the database and the processing of the imediate returns
     */
    namespace database {
        sqlite3 *db;
        char *zErrMsg = 0;
        std::string docFilePath;

        /**
         * callback() is a generic callback for sql commands that does nothing.
         * All callback functions take the same paramaters
         * 
         * @param data provided in the 4th arg of sqlite3_exec()
         * @param argc number of columns in row
         * @param argv array of strings representing fields in the row
         * @param azColName array of strings representing column names
         */
        static int callback(void* data, int argc, char **argv, char **azColName) {
            return 0;
        } // callback()
        
        /**
         * mediaCallback() creates a content::Item for each entry returned by the sql requests and then
         * populates glb_media with the Item
         * 
         * @param argv 0->media_id, 1->title, 2->sort_title, 3->volume_num, 4->issue_num
         */
        static int mediaCallback(void* data, int argc, char **argv, char **azColName) {
            if (argc != 5) {
                throw std::invalid_argument("Database query should contain 5 arguments, but has: " + argc);
            }
            content::Item item;
            item.id             = std::string(argv[0]);
            item.title          = std::string(argv[1]);
            item.sortTitle      = std::string(argv[2]);
            item.cover          = "vagabond-v01.jpg"; // -> id.png
            item.volume         = ((argv[3]) ? std::string(argv[3]) : "" );
            item.issue          = ((argv[4]) ? std::string(argv[4]) : "" );
            item.progress       = 0; // get progress or 0
            item.isCollection   = false;

            glb_media.push_back(item);            
            return 0;
        } // mediaCallback()

        /**
         * collectionCallback() creates a content::Item for each entry returned by the sql requests and
         * the populates glb_media with the Item
         * 
         * @param argv 0->collection_id, 1->title, 2->sort_title, 3->number_vol, 4->number_iss
         */
        static int collectionCallback(void* data, int argc, char **argv, char **azColName) {
            if (argc != 5) {
                throw std::invalid_argument("Database query should contain 5 arguments, but has: " + argc);
            }
            content::Item item;
            item.id             = std::string(argv[0]);
            item.title          = std::string(argv[1]);
            item.sortTitle      = std::string(argv[2]);
            item.cover          = "vagabond-v01.jpg"; // -> id.png
            item.volume         = ((argv[3]) ? std::string(argv[3]) : "" );
            item.issue          = std::string(argv[4]);
            item.progress       = 0; // get progress or 0
            item.isCollection   = true;

            glb_media.push_back(item);            
            return 0;
        } // collectionCallback()
        
        /**
         * walkDirectoriesCallback() recursivly calls itself as it walks backwards through a directroy path in
         * the directory database. Once the root directory has been reached, the path starts to be built with
         * the name of each directory on docFilePath.
         *
         * @param argv 0->parent_id, 1->name
         */
        static int walkDirectoriesCallback(void* data, int argc, char **argv, char **azColName) {
            if (argc != 2) {
                throw std::invalid_argument("Database query should contain 2 arguments, but has: " + argc);
            }

            if (argv[0]) {
                std::cout << "Walking past " << std::string(argv[1]) << std::endl;
                std::string sql;
                sql = "SELECT directories.parent_id, directories.name FROM directories WHERE directory_id =" + std::string(argv[0]) + ";";
                int res = sqlite3_exec(db, sql.c_str(), walkDirectoriesCallback, data, &zErrMsg);
                if (res != SQLITE_OK) {
                    throw std::runtime_error("SQL error:\n" + std::string(zErrMsg));
                }
            }
            std::cout << "Currently at " << std::string(argv[1]) << std::endl;
            docFilePath.append(argv[1]);
            docFilePath.append("/");

            std::cout << docFilePath << std::endl;
            return 0;
        } // walkDirectoriesCallback()

        /**
         * fetchFileCallback() starts the call to walkDirectoriesCallback with the directory_id that the file is
         * contained in and once the filepath is built, the filename is appended to the end of the path to finish
         * off docFilePath for the book.
         * 
         * @param argv 0->file_loc, 1->filename
         */
        static int fetchFileCallback(void* data, int argc, char **argv, char **azColName) {
            if (argc != 2) {
                throw std::invalid_argument("Database query should contain 2 arguments, but has: " + argc);
            }
            std::cout << "Fetching path to '" << std::string(argv[1]) << "' from the database" << std::endl;
            
            std::string sql;
            sql = "SELECT directories.parent_id, directories.name FROM directories WHERE directory_id =" + std::string(argv[0]) + ";";
            int res = sqlite3_exec(db, sql.c_str(), walkDirectoriesCallback, data, &zErrMsg);
            if (res != SQLITE_OK) {
                throw std::runtime_error("SQL error:\n" + std::string(zErrMsg));
            }

            docFilePath.append(argv[1]);

            std::cout << docFilePath << std::endl;
            return 0;
        } // fetchFileCallback()
        
        /**
         * Opens the database with the given filepath
         * 
         * @param filepath the full file path, including the name, to the database file
         */
        void open(std::string filepath) {
            if (db) {
                sqlite3_close(db);
            }
            int res = sqlite3_open(filepath.c_str(), &db);
            if (res) {
                throw std::invalid_argument("Database failed to open: " + std::string(sqlite3_errmsg(db)));
            }
            std::cout << "Database opened" << std::endl;
        } // open()

        /**
         * getMedia() clears glb_media before making a sql request for media that don't belong to a collection
         * and for all of the collections. The media request is sent to mediaCallback() and te collection request
         * is sent to collectionCallback() to handle the retrieved data and repopulate glb_media.
         */
        void getMedia() {
            std::cout << "Getting books from database not in a collection" << std::endl;
            glb_media.clear();
            std::string sql;
            sql = "SELECT media.media_id, media.title, media.sort_title, media.volume_num, media.issue_num FROM media WHERE collection_id IS NULL;";
            int res = sqlite3_exec(db, sql.c_str(), mediaCallback, 0, &zErrMsg);
            if (res != SQLITE_OK) {
                throw std::runtime_error("SQL error: \n" + std::string(zErrMsg));
            }
            
            std::cout << "Getting collections" << std::endl;
            sql = "SELECT collections.collection_id, collections.title, collections.sort_title, collections.number_vol, collections.number_iss FROM collections;";
            res = sqlite3_exec(db, sql.c_str(), collectionCallback, 0, &zErrMsg);
            if (res != SQLITE_OK) {
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
        void getCollectionMedia(std::string id) {
            std::cout << "Getting books in the collection" << std::endl;
            glb_media.clear();
            std::string sql;
            sql = "SELECT media.media_id, media.title, media.sort_title, media.volume_num, media.issue_num FROM media WHERE collection_id=" + id + ";";
            int res = sqlite3_exec(db, sql.c_str(), mediaCallback, 0, &zErrMsg);
            if (res != SQLITE_OK) {
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
        std::string fetchFile(std::string media_id) {
            std::cout << "Getting the file for the book" << std::endl;
            docFilePath = "/";
            std:: string sql;
            sql = "SELECT media.file_loc, media.filename FROM media WHERE media_id=" + media_id + ";";
            int res = sqlite3_exec(db, sql.c_str(), fetchFileCallback, 0, &zErrMsg);
            if (res != SQLITE_OK) {
                throw std::runtime_error("SQL error: \n" + std::string(zErrMsg));
            }

            return docFilePath;
        }

        /**
         * close() closes the database
         */
        void close() {
            sqlite3_close(db);
            std::cout << "Closed database" << std::endl;
        } // close()
    }; // inline database namespace

    /**
     * ReadingRPC handles all of the calls related to viewing a book. 
     * Including:
     *      Loading pages for the book and sending html for the client to view
     * ##   Updating book and collection progress
     */
    class ReadingRPC : public cppcms::rpc::json_rpc_server {
    private:
        int pageChunkSizeForwards, pageChunkSizeBackwards; // number of pages to be loaded per call defined in config.json
        int pageCount, firstPageLoaded, lastPageLoaded, currentPage;
        std::string id;
        mupdf::Document *doc;
        mupdf::Matrix myMatrix;
        mupdf::Colorspace myColor;
    
    public:
        /**
         * Get the page chunk size from config.json and bind the functions to the applicaion
         */
        ReadingRPC(cppcms::service &srv) : cppcms::rpc::json_rpc_server(srv) {
            pageChunkSizeForwards = settings().get<int>("app.page_chunk_size.forward");
            pageChunkSizeBackwards = settings().get<int>("app.page_chunk_size.backward");
            
            bind("loadInit",cppcms::rpc::json_method(&ReadingRPC::loadInit,this),method_role);
            bind("loadForwards",cppcms::rpc::json_method(&ReadingRPC::loadForwards,this),method_role);
            bind("loadBackwards",cppcms::rpc::json_method(&ReadingRPC::loadBackwards,this),method_role);
        }

    private:
        /**
         * Every called book is sent here with an id so that it can be processed to return a book
         *    1. The book is loaded from the location in the database with the given id and start page which is grabbed from the database
         *          1a) the start page keeps track of the loading progress so that the book can load in chunks
         *          1b) the book is cached so that it is not continuously opened and closed
         * ## 2. Checks to see if the book is reflowable, continueing this step if it is
         *          2a) adjusts the reflowable book to the user's preferences
         *    3. Opens the start page and saves a png of the in the tmp directory 
         *    4. Creates an image html tag with the address of the stored page and adds it to the returnHTML
         *    5. Repeat steps 3-4 for every page within the pageChunkSizeForwards limit
         *    6. Return the html with other information in a json formate for the client to use to view the book
         *
         * @param setId id of the book that is to be viewed
         */
        void loadInit(std::string const &setId) {
            if (false) { // check id validity
                return_error("Given id is not valid");
                return;
            }
            id = setId;
             
            std::string file =  settings().get<std::string>("app.paths.media") + database::fetchFile(id);
            doc = new mupdf::Document(file.c_str());
            myMatrix = mupdf::Matrix();
            myColor = mupdf::device_rgb();
            pageCount = doc->count_pages(); // check against database and throw error and drop doc if diffrent
                
            std::cout << "Page Count: " << pageCount << std::endl << "FilePath: " << file.c_str() << std::endl;

            currentPage = 0; // grab from database make sure between zero and last page => if null or last page make 0
            firstPageLoaded = currentPage;
            int endPage;
            if (firstPageLoaded + pageChunkSizeForwards < pageCount) {
                endPage = firstPageLoaded + pageChunkSizeForwards;
            } else {
                endPage = pageCount;
            }

            std::string returnHTML;
            for(int pageNum = firstPageLoaded; pageNum < endPage; pageNum++) {
                std::string fileName = id + "-" + std::to_string(pageNum) + ".png";
                std::string filepath = settings().get<std::string>("app.paths.tmp") + fileName;
                std::cout << filepath << std::endl;
                
                doc->new_pixmap_from_page_number(pageNum, myMatrix, myColor, 0)
                    .save_pixmap_as_png(filepath.c_str());

                returnHTML.append(generateImgTag(fileName));
            }

            lastPageLoaded = endPage;
            
            cppcms::json::value json;
            json["html"] = returnHTML;
            json["firstLoaded"] = firstPageLoaded;
            json["lastLoaded"] = endPage;
            json["pageCount"] = pageCount;
            json["isDoubleView"] = false;
            json["forwardsChunk"] = pageChunkSizeForwards;
            json["backwardsChunk"] = pageChunkSizeBackwards;
            return_result(json);
        } // loadInit()

        /**
         * Checks if there is an open book or not and the validity of the paramaters, then loads a pageChunkSizeForwards
         * the same way loadInit does, and finaly return the html with the new first and last loaded pages
         *  
         * @param current the current page sent from the client
         * @param start the last page loaded by the client to be checked against the last page of loaded by the server
         */
        void loadForwards(int const &current, int const &start) {
            if (!doc) {
                return_error("No document is open");
                return;
            }
            if (current < 0 || current >= pageCount || current > start || start >= pageCount) {
                return_error("Page range OUT_OF_BOUNDS");
                return;
            }
            if (start != lastPageLoaded) {
                return_error("Page range missing");
                return;
            }
            currentPage = current;
            
            int endPage;
            if (lastPageLoaded + pageChunkSizeForwards < pageCount) {
                endPage = lastPageLoaded + pageChunkSizeForwards;
            } else {
                endPage = pageCount;
            }
            
            std::string returnHTML;
            for(int pageNum = lastPageLoaded; pageNum < endPage; pageNum++) {
                std::string fileName = id + "-" + std::to_string(pageNum) + ".png";
                std::string filepath = settings().get<std::string>("app.paths.tmp") + fileName;
                std::cout << filepath << std::endl;
                
                doc->new_pixmap_from_page_number(pageNum, myMatrix, myColor, 0)
                    .save_pixmap_as_png(filepath.c_str());

                returnHTML.append(generateImgTag(fileName));
            }
            
            lastPageLoaded = endPage;
            
            cppcms::json::value json;
            json["html"] = returnHTML;
            json["firstLoaded"] = firstPageLoaded;
            json["lastLoaded"] = endPage;
            return_result(json);
        } // loadForwards()

        /**
         * Checks if there is an open book or not and the validity of the paramaters, then loads a pageChunkSizeBackwards
         * the same way loadInit does, and finaly return the html with the new first and last loaded pages
         *  
         * @param current the current page sent from the client
         * @param start the first page loaded by the client to be checked against the first page of loaded by the server
         */
        void loadBackwards(int const &current, int const &start) {
            if (!doc) {
                return_error("No document is open");
                return;
            }
            if (current < 0 || current < start) {
                return_error("Page range OUT_OF_BOUNDS");
                return;
            }
            if (start != firstPageLoaded) {
                return_error("Page range missing");
                return;
            }
            currentPage = current;
            
            int endPage;
            if (firstPageLoaded - pageChunkSizeBackwards < 0) {
                endPage = 0;
            } else {
                endPage = firstPageLoaded - pageChunkSizeBackwards;
            }
            
            std::string returnHTML;
            for(int pageNum = endPage; pageNum < firstPageLoaded; pageNum++) {
                std::string fileName = id + "-" + std::to_string(pageNum) + ".png";
                std::string filepath = settings().get<std::string>("app.paths.tmp") + fileName;
                std::cout << filepath << std::endl;
                
                doc->new_pixmap_from_page_number(pageNum, myMatrix, myColor, 0)
                    .save_pixmap_as_png(filepath.c_str());

                returnHTML.append(generateImgTag(fileName));
            }

            firstPageLoaded = endPage;

            cppcms::json::value json;
            json["html"] = returnHTML;
            json["firstLoaded"] = endPage;
            json["lastLoaded"] = lastPageLoaded;
            return_result(json);
        } // loadBackwards

        /**
         * Acts as a generic <img> tag generater for the other 3 load functions
         * 
         * @param fileName name of the page file
         */
        std::string generateImgTag(std::string const fileName) {
           return "<img class='myPages' src='/pages/" + fileName + "'>";
        }
    }; // ReadRPC class

    /**
     * 
     */
    class ImportRPC : public cppcms::rpc::json_rpc_server {
    public:
        ImportRPC(cppcms::service &srv) : cppcms::rpc::json_rpc_server(srv) {
            bind("getImports",cppcms::rpc::json_method(&ImportRPC::getImports,this),method_role);
            bind("import",cppcms::rpc::json_method(&ImportRPC::import,this),notification_role);
        }

    private:
        /**
         * Graps the data for the new files to be imported
         */
        void getImports() {}

        /**
         * Takes in information about a file and notifies the system to record the new files' info into the database
         * and move the file to the media folder with a possible rename
         * Each call only handles one file at a time
         * 
         * oldFileName, newFileName, title, ...etc 
         */
        void import() {}
    }; // ImportRPC class
    
    /**
     * 
     */
    class SettingsRPC : public cppcms::rpc::json_rpc_server {
    public:
        SettingsRPC(cppcms::service &srv) : cppcms::rpc::json_rpc_server(srv) {}

    private:
    }; // SettingsRPC class
    
} // namespace services

/**
 * 
 */
class WebSite : public cppcms::application {
public:
    /**
     * Attaches services, assigns url mapping, and then sets the root of the webpage
     */
    WebSite(cppcms::service &srv) : cppcms::application(srv) {
        services::database::open(settings().get<std::string>("app.paths.db"));
        
        attach(new services::ReadingRPC(srv),"/reading-rpc(/(\\d+)?)?",0);
        // attach(new service::ImportRPC(srv),"/import-rpc(/(\\d+)?)?",0);
        // attach(new service::SettingsRPC(srv),"/settings-rpc(/(\\d+)?)?",0);

        dispatcher().assign("/",&WebSite::library,this);
        mapper().assign("library","/");
        
        dispatcher().assign("/upnext",&WebSite::upnext,this);
        mapper().assign("upnext","/upnext");
        
        dispatcher().assign("/collection/(\\w+)",&WebSite::collection,this,1);
        mapper().assign("collection","/collection/{1}");

        dispatcher().assign("/import",&WebSite::import,this);
        mapper().assign("import","/import");
        
        dispatcher().assign("/help",&WebSite::help,this);
        mapper().assign("help","/help");
        
        dispatcher().assign("/login",&WebSite::login,this);
        mapper().assign("login","/login");
        
        dispatcher().assign("/settings/user",&WebSite::user,this);
        mapper().assign("user","/settings/user");
        
        dispatcher().assign("/settings/account",&WebSite::account,this);
        mapper().assign("account","/settings/account");
        
        dispatcher().assign("/settings/general",&WebSite::general,this);
        mapper().assign("general","/settings/general");
        
        dispatcher().assign("/settings/acount-management",&WebSite::accountManagement,this);
        mapper().assign("account_management","/settings/acount-management");
        
        dispatcher().assign("/settings/media-management",&WebSite::mediaManagement,this);
        mapper().assign("media_management","/settings/media-management");
        
        dispatcher().assign("/settings/meintenance",&WebSite::meintenance,this);
        mapper().assign("meintenance","/settings/meintenance");

        mapper().root("");
    } // WebSite()

private:
    /**
     * Sets the sitewide title
     * @param cnt any content view dirived from master
     */
    void ini(content::Master &cnt) {
        cnt.title = "Nabu";
    }

    /**
     * library() renders the Library view. The content is created by a call to database::getMedia(), which
     * rebuild glb_media before it is copied onto cnt.media and sorted by title. Then the view is rendered
     * and sent to the client.
     */
    void library() {
        content::Library cnt;
        ini(cnt);
        services::database::getMedia();
        cnt.media = services::glb_media;
        std::stable_sort(cnt.media.begin(), cnt.media.end(), [](content::Item c1, content::Item c2) {
            return c1.sortTitle.compare(c2.sortTitle) < 0;
        });
        render("library", cnt);
    }

    /**
     * upnext() retnders the UpNext view.
     */
    void upnext() {
        content::UpNext cnt;
        ini(cnt);
        render("upnext", cnt);
    }

    /**
     * collection() renders the Collection view. The content is created by a call to database::getCollectionMedia(),
     * which rebuilds glb_media before it is copied onto cnt.books and sorted by volume. Then the view is rendered
     * and sent to the client.
     * 
     *  @param id id of the collection to be viewed
     */
    void collection(std::string id) {
        content::Collection cnt;
        ini(cnt);
        cnt.collectionId = id;
        for (content::Item item : services::glb_media) {
            if (id == item.id) {
                cnt.collectionTitle = item.title;
                cnt.collectionCover = item.cover;
                break;
            }
        }
        services::database::getCollectionMedia(id);
        cnt.books = services::glb_media;
        std::stable_sort(cnt.books.begin(), cnt.books.end(), [](content::Item c1, content::Item c2) {
            std::stringstream c1_num(c1.volume);
            int n1 = 0;
            c1_num >> n1;
            std::stringstream c2_num(c2.volume);
            int n2 = 0;
            c2_num >> n2;
            return n1 - n2;
        });
        render("collection", cnt);
    }
    
    /**
     * The following render the Import, Help, and Login views respectively
     */
    void import() {
        content::Import cnt;
        ini(cnt);
        render("import", cnt);
    }
    void help() {
        content::Help cnt;
        ini(cnt);
        render("help", cnt);
    }
    void login() {
        content::Login cnt;
        ini(cnt);
        render("login", cnt);
    }

    /**
     * The following render the various Settings views
     */
    void user() {
        content::User cnt;
        ini(cnt);
        render("user", cnt);
    }
    void account() {
        content::Account cnt;
        ini(cnt);
        render("account", cnt);
    }
    void general() {
        content::General cnt;
        ini(cnt);
        render("general", cnt);
    }
    void accountManagement() {
        content::AccountManagement cnt;
        ini(cnt);
        render("account_management", cnt);
    }
    void mediaManagement() {
        content::MediaManagement cnt;
        ini(cnt);
        render("media_management", cnt);
    }
    void meintenance() {
        content::Meintenance cnt;
        ini(cnt);
        render("meintenance", cnt);
    }

    /**
     * Sets the override for the 404 page of the website
     */
    virtual void main(std::string url) {
        if (!dispatcher().dispatch(url)) {
            response().status(cppcms::http::response::not_found);
            content::PageNotFound cnt;
            render("page_not_found", cnt);
        }
    }
};

/**
 * Starts and runs the application
 */
int main(int argc, char ** argv) {
    try {
        cppcms::service srv(argc,argv);
        srv.applications_pool().mount(cppcms::create_pool<WebSite>());
        srv.run();
    } catch(std::exception const &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
