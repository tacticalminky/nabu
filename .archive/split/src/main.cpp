/*
This file is part of Nabu.

Nabu is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Nabu is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License along with Nabu. If not, see <https://www.gnu.org/licenses/>.
*/

/**
 * @author  Andrew Mink
 * @version alpha
 * @date    23 January 2022
 * 
 * WebSite is the main application that the other applications attach to and run from. WebSite handles
 * the construction of the website and url dispatching.
 */

#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <cppcms/service.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/http_response.h>
#include <cppcms/rpc_json.h>
#include <cppcms/form.h>

#include <mupdf/classes.h>
#include <sqlite3.h>
#include <filesystem>

#include "views/content.h"
#include "services/rpc.h"
#include "services/database.h"

namespace fs = std::filesystem;

class WebSite : public cppcms::application {
public:
    /**
     * Attaches services, assigns url mapping, and then sets the root of the webpage
     */
    WebSite(cppcms::service &srv) : cppcms::application(srv) {
        services::database::open(settings().get<std::string>("app.settings.admin.paths.db"));
        services::setMediaPath(settings().get<std::string>("app.settings.admin.paths.media"));
        services::coverPath = settings().get<std::string>("app.settings.admin.paths.covers");
        services::importPath = settings().get<std::string>("app.settings.admin.paths.import");
        services::pagesPath = settings().get<std::string>("app.settings.admin.paths.tmp");
        
        attach(new services::ReadingRPC(srv),"/reading-rpc(/(\\d+)?)?",0);
        attach(new services::DataRPC(srv),"/data-rpc(/(\\d+)?)?",0);

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

        dispatcher().assign("/403",&WebSite::forbidden,this);
        mapper().assign("forbidden","/403");

        mapper().root("");
    } // WebSite()

private:
    /**
     * ini() is called by most views. Sets the sitewide title and redirects to the login page when no
     * user is loged on.
     * 
     * @param cnt any content view dirived from master
     * @return true if user is set
     */
    bool ini(content::Master &cnt) {
        cnt.title = settings().get<std::string>("app.title");
        if (!session().is_set("username")) {
            response().set_redirect_header("/login");
            return false;
        }
        return true;
    }

    /**
     * library() renders the Library view. The content is created by a call to database::getMedia(), which
     * rebuild glb_media before it is copied onto cnt.media and sorted by title. Then the view is rendered
     * and sent to the client.
     */
    void library() {
        content::Library cnt;
        if (!ini(cnt)) {
            return;
        }
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
        if (!ini(cnt)) {
            return; 
        }
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
        if (!ini(cnt)) {
            return;
        }
        // adjust what is being searched -> refresh removes the name
        for (content::Item item : services::glb_media) {
            if (id == item.id) {
                cnt.collectionTitle = item.title;
                break;
            }
        }
        services::database::getCollectionMedia(id);
        cnt.books = services::glb_media;
        render("collection", cnt);
    }
    
    /**
     * import() builds the import view from files in the import directory
     */
    void import() {
        content::Import cnt;
        if (!ini(cnt)) {
            return;
        }
        std::cout << "Building import view" << std::endl; // make recursive_directory_iterator when I can handle it
        for (const auto& entry : fs::directory_iterator(services::importPath)) {
            if (entry.is_directory()) {
                continue;
            }

            fs::path file = entry.path();
            std::string ext = file.extension();
            if (ext != ".pdf" && ext != ".epub" && ext != ".cbz") {
                continue;
            }

            content::ImportItem item;
            item.file = file.filename();
            item.title = file.stem();
            item.sortTitle = file.stem(); // handle a beginning the
            cnt.imports.push_back(item);
        }
        render("import", cnt);
    }

    /**
     * The following renders the Help page
     */
    void help() {
        content::Help cnt;
        cnt.title = settings().get<std::string>("app.title");
        render("help", cnt);
    }
    
    /**
     * login() renders the login page and hands login form submission with session creation. Also acts
     * as a method to logout.
     */
    void login() {
        content::Login cnt;
        cnt.title = settings().get<std::string>("app.title");
        if (request().request_method() == "POST" && session().is_set("prelogin")) {
            cnt.login.load(context());
            if (cnt.login.validate() && services::database::validateLogin(cnt.login.username.value(), cnt.login.password.value())) {
                session().reset_session();
                session().erase("prelogin");
                session().set("username", cnt.login.username.value());
                // grab user preferences
                cnt.login.clear();
                response().set_redirect_header("/");
                return;
            }
        }
        session().reset_session();
        session().erase("username");
        session().set("prelogin","");
        render("login", cnt);
    }

    /**
     * settingsView() handles what settings are available for the user 
     * 
     * @param cnt any content view dirived from settings
     * @return false if user cannot navigate to the page, and true otherwise
     */
    bool settingsView(content::Settings &cnt) {
        std::cout << "Attempting to view settings" << std::endl;
        if (!ini(cnt)) {
            return false;
        }
        cnt.hasAdmin = false;
        std::string perms = services::database::getPermissions(session()["username"]);
        perms = "user";
        if (perms != "user" && perms != "admin") {
            response().set_redirect_header("/403");
            return false;
        }
        if ((std::string(typeid(cnt).name()) == "N7content4UserE") || (std::string(typeid(cnt).name()) == "N7content7AccountE")) {
            if (perms == "admin") {
                cnt.hasAdmin = true;
            }
            return true;
        }
        if (perms != "admin") {
            response().set_redirect_header("/403");
            return false;
        }
        cnt.hasAdmin = true;
        return true;
    }

    /**
     * The following render the various Settings views
     * 
     * Check privileges of session -> only show user and account unless they have admin privileges
     * -> redirect to user if no admin privileges
     */
    void user() {
        content::User cnt;
        if (!settingsView(cnt)) {
            return;
        }
        render("user", cnt);
    }
    void account() {
        content::Account cnt;
        if (!settingsView(cnt)) {
            return;
        }
        render("account", cnt);
    }
    void general() {
        content::General cnt;
        if (!settingsView(cnt)) {
            return;
        }
        render("general", cnt);
    }
    void accountManagement() {
        content::AccountManagement cnt;
        if(!settingsView(cnt)) {
            return;
        }
        render("account_management", cnt);
    }
    void mediaManagement() {
        content::MediaManagement cnt;
        if (!settingsView(cnt)) {
            return;
        }
        render("media_management", cnt);
    }
    void meintenance() {
        content::Meintenance cnt;
        if (!settingsView(cnt)) {
            return;
        }
        render("meintenance", cnt);
    }

    /**
     * forbidden() renders the 403 FORBIDDEN page
     */
    void forbidden() {
        response().status(cppcms::http::response::forbidden);
        content::Forbidden cnt;
        cnt.title = settings().get<std::string>("app.title");
        render("forbidden", cnt);
    }

    /**
     * Sets the override for the 404 page of the website
     */
    virtual void main(std::string url) {
        if (!dispatcher().dispatch(url)) {
            response().status(cppcms::http::response::not_found);
            content::PageNotFound cnt;
            cnt.title = settings().get<std::string>("app.title");
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
