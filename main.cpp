/**
 * 
 */

#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <cppcms/service.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/rpc_json.h>

#include <mupdf/classes.h>

#include "views/content.h"

namespace services {

    /**
     * 
     */
    class ReadingRPC : public cppcms::rpc::json_rpc_server {
    private:
        int pageChunkSizeForwards, pageChunkSizeBackwards; // number of pages to be loaded per call defined in config.json
        int pageCount, firstPageLoaded, lastPageLoaded, currentPage;
        std::string id;
        mupdf::Document* doc;
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
         *    1. The book is loaded from the location in the database with the given id and start page
         *          1a) the start page keeps track of the loading progress so that the book can load in chunks
         * ##       1b) the book is cached so that it is not continuously opened and closed
         * ## 2. Check to see if the book is reflowable, continue this step if it is
         *          2a) adjust the reflowable book to the user's preferences
         *    3. Create an image with an adress for each page in the book and make it into an html img
         *          => <img id="page-NUMBER" src='FILEPATH' loading='lazy'>
         *    4. Return the html with whether or not the book has been full loaded
         */
        void loadInit(std::string const &setId) {
            if (false) { // check id validity
                return_error("Given id is not valid");
                return;
            }
            id = setId;
            
            std::string file =  settings().get<std::string>("app.paths.media") + "input.pdf"; // file from database
            doc = new mupdf::Document(file.c_str());
            myMatrix = mupdf::Matrix();
            myColor = mupdf::device_rgb();
            pageCount = doc->count_pages(); // check against database and throw error and drop doc if diffrent
                
            std::cout << "Page Count: " << pageCount << std::endl << "FilePath: " << file.c_str() << std::endl;

            currentPage = 6; // grab from database make sure between zero and last page => if null or last page make 0
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

                returnHTML.append(generateImgTag(pageNum, fileName));
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
         * 
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

                returnHTML.append(generateImgTag(pageNum, fileName));
            }
            
            lastPageLoaded = endPage;
            
            cppcms::json::value json;
            json["html"] = returnHTML;
            json["firstLoaded"] = firstPageLoaded;
            json["lastLoaded"] = endPage;
            return_result(json);
        } // loadForwards()

        /**
         * 
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

                returnHTML.append(generateImgTag(pageNum, fileName));
            }

            firstPageLoaded = endPage;

            cppcms::json::value json;
            json["html"] = returnHTML;
            json["firstLoaded"] = endPage;
            json["lastLoaded"] = lastPageLoaded;
            return_result(json);
        } // loadBackwards

        std::string generateImgTag(int const pageNum, std::string const fileName) {
           return "<img id='page-" + std::to_string(pageNum)+ "' class='myPages' src='/pages/" + fileName + "' loading='lazy'>";
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

} // end of namespace services

/**
 * 
 */
class WebSite : public cppcms::application {
public:
    /**
     * Attaches services, assigns url mapping, and then sets the root of the webpage
     */
    WebSite(cppcms::service &srv) : cppcms::application(srv) {
        attach(new services::ReadingRPC(srv),"/reading-rpc(/(\\d+)?)?",0);
        // attach(new service::importRPC(srv),"/import-rpc(/(\\d+)?)?",0);

        dispatcher().assign("/",&WebSite::library,this);
        mapper().assign("library","/");
        
        dispatcher().assign("/upnext",&WebSite::upnext,this);
        mapper().assign("upnext","/upnext");
        
        dispatcher().assign("/collection/(\\w+)",&WebSite::collection,this,1);
        mapper().assign("collection","/collection/{1}");

        dispatcher().assign("/read",&WebSite::read,this);
        mapper().assign("read","/read");

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
     */
    void ini(content::Master &cnt) {
        cnt.title = "Nabu";
    }

    /**
     * The following renders the Library and Reading views
     * 
     * The Library view is constructed by
     */
    void library() {
        content::Library cnt;
        ini(cnt);
        // for every entry in database without a collection, add an item to the list
        content::Item item;
        item.title = "Title";
        item.id = "0";
        item.cover = "vegabond-v01.jpg";    // (NOT NULL) ? value : default;
        item.file = "/media/input.cbz";     // Deal with spaces in file name or find another way
        // item.volumes = "1";  Leave NULL if blank
        item.progress = 0.0; // current page / total pages to 1 decimal
        cnt.media.push_back(item);
        // do the same for every collection with isCollection = true
        // sorts the list based on title
        std::stable_sort(cnt.media.begin(), cnt.media.end(), [](content::Item c1, content::Item c2) {
            return c1.title.compare(c2.title) > 0;
        });
        render("library", cnt);
    }
    void upnext() {
        content::UpNext cnt;
        ini(cnt);
        render("upnext", cnt);
    }
    void collection(std::string id_) {
        content::Collection cnt;
        ini(cnt);
        cnt.collection_id = id_; // title of a coresponding collection
        // for every item in the collection add to list, volumes should be 2+
        render("collection", cnt);
    }
    void read() {
        content::Read cnt;
        ini(cnt);
        render("read", cnt);
    }
    
    /**
     * The following renders the Import, Help, and Login views
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
     * The following renders the various Settings views
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
