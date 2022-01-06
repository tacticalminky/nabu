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

    class JsonService : public cppcms::rpc::json_rpc_server {
    public:
        JsonService(cppcms::service &srv) : cppcms::rpc::json_rpc_server(srv) {
    	   std::cout << "******************** Attaching JsonService ********************" << std::endl;
           bind("read",cppcms::rpc::json_method(&JsonService::read,this),method_role);
        }

        /**
         * Every called book is sent here with an id so that it can be processed to return a book
         *    1. The book is loaded from the location in the database with the given id
         *    2. Check to see if the book is reflowable, continue this step if it is
         *          2a) adjust the reflowable book to the user's preferences
         *    3. Create an image with an adress for each page in the book and make it into an html img
         *          => <img src='FILEPATH' alt='Page: NUMBER' loading='lazy'>
         *    4. Return the html
         */
        void read(std::string const id) {
    	    std::cout << "*********************** Building BookImg **********************" << std::endl;
            std::string returnHTML;
            std::string file =  settings().get<std::string>("app.media") + "input.pdf"; // file from database

            mupdf::Document doc = mupdf::Document(file.c_str());
            int pageCount = doc.count_pages(); // check against database and throw error and drop doc if diffrent
            std::cout << "Page Count: " << pageCount << std::endl;

            mupdf::Matrix myMatrix = mupdf::Matrix();
            mupdf::Colorspace myColor = mupdf::device_rgb();
            
            for(int pageNum = 0; pageNum < pageCount; pageNum++) {
                std::string fileName = id + "-" + std::to_string(pageNum) + ".png";
                std::string filepath = settings().get<std::string>("app.tmp") + fileName;
                
                std::cout << "Filepath: " << filepath << std::endl;
                
    		    std::cout << "*********************** Creating Pixmap ***********************" << std::endl;
                mupdf::Pixmap page = doc.new_pixmap_from_page_number(pageNum, myMatrix, myColor, 0);
      		    std::cout << "******************* Creating PNG from Pixmap ******************" << std::endl;
		        page.save_pixmap_as_png(filepath.c_str());

                returnHTML.append("<img src='/pages/" + fileName + "' alt='Page: " + std::to_string(pageNum) + "' loading='lazy'>");
            }

            return_result(returnHTML);
        }
    };

} // end of namespace services

/**
 * 
 */
class WebSite : public cppcms::application {
public:
    /**
     * 
     */
    WebSite(cppcms::service &srv) : cppcms::application(srv) {
    	std::cout << "*********************** Building WebSite **********************" << std::endl;

        // attaches JsonService to the application
        attach(new services::JsonService(srv), "/rpc(/(\\d+)?)?", 0);

        // assigns the url mapping to the application with the build method
        dispatcher().assign("/",&WebSite::library,this);
        mapper().assign("library","/");
        
        dispatcher().assign("/upnext",&WebSite::upnext,this);
        mapper().assign("upnext","/upnext");
        
        dispatcher().assign("/collection/(\\w+)",&WebSite::collection,this,1);
        mapper().assign("collection","/collection/{1}");

        dispatcher().assign("/read/(\\w+)",&WebSite::read,this,1);
        mapper().assign("read","/read/{1}");

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
        
        dispatcher().assign("/settings/acount-management",&WebSite::account_management,this);
        mapper().assign("account_management","/settings/acount-management");
        
        dispatcher().assign("/settings/media-management",&WebSite::media_management,this);
        mapper().assign("media_management","/settings/media-management");
        
        dispatcher().assign("/settings/meintenance",&WebSite::meintenance,this);
        mapper().assign("meintenance","/settings/meintenance");

        mapper().root("");
    }

private:
    /**
     * Master view
     */
    void ini(content::Master &cnt) {
        cnt.title = "Nabu";
    }

    /**
     * Library and reading views 
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
    void read(std::string id) {
        content::Read cnt;
        ini(cnt);
        cnt.book_id = id;  // id coresponding to a book
        render("read", cnt);
    }
    
    /**
     * Import, Help, and Login views
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
     * Various Settings Views
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
    void account_management() {
        content::AccountManagement cnt;
        ini(cnt);
        render("account_management", cnt);
    }
    void media_management() {
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
     * overrides 404 page
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
 * 
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
