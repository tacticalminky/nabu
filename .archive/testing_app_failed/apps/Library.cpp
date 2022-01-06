/**
 * 
 */

#include <cppcms/application.h>
#include <cppcms/service.h>
#include <cppcms/url_dispatcher.h>

#include "../views/content.h"

namespace apps {

/**
 * 
 */
class Library : public cppcms::application {
public:
    /**
     * 
     */
    Library(cppcms::service &srv) : cppcms::application(srv) {
        std::cout << "****************************** Library binding ******************************" << std::endl;
        
        dispatcher().assign("",&Library::library,this);
        mapper().assign("library","");
        
        dispatcher().assign("upnext",&Library::upnext,this);
        mapper().assign("upnext","upnext");
        
        dispatcher().assign("collection/(\\w+)",&Library::collection,this,1);
        mapper().assign("collection","collection/{1}");

        dispatcher().assign("read/(\\w+)",&Library::read,this,1);
        mapper().assign("read","read/{1}");

        dispatcher().assign("import",&Library::import,this);
        mapper().assign("import","import");
        
        dispatcher().assign("help",&Library::help,this);
        mapper().assign("help","help");
        
        dispatcher().assign("login",&Library::login,this);
        mapper().assign("login","login");
    }

private:

    /**
     * Library and reading views 
     */
    void library() {
        content::Library cnt;
        // for every entry in database without a collection, add an item to the list
        // for every entry in database without a collection, add an item to the list
        content::Item item;
        item.title = "Title";
        item.id = "0";
        item.cover = "vegabond-v01.jpg";    // (NOT NULL) ? value : default;
        item.file = "/media/input.cbz";     // Deal with spaces in file name or find another way
        // item.volumes = "1";  Leave NULL if blank
        item.progress = 0;
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
        render("upnext", cnt);
    }
    void collection(std::string id_) {
        content::Collection cnt;
        cnt.collection_id = id_; // title of a coresponding collection
        // for every item in the collection add to list, volumes should be 2+
        render("collection", cnt);
    }
    void read(std::string id) {
        content::Read cnt;
        cnt.book_id = id;  // id coresponding to a book
        render("read", cnt);
    }
    
    /**
     * Import, Help, and Login views
     */
    void import() {
        content::Import cnt;
        render("import", cnt);
    }
    void help() {
        content::Help cnt;
        render("help", cnt);
    }
    void login() {
        content::Login cnt;
        render("login", cnt);
    }
};

} // end of namespace apps