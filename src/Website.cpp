/*
This file is part of Nabu.

Nabu is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Nabu is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License along with Nabu. If not, see <https://www.gnu.org/licenses/>.
*/

/**
 * @author  Andrew Mink
 * @date    6 February 2022
 * 
 * Website is the main application that the other applications attach to and run from. WebSite handles
 * the construction of the website and url dispatching.
 */

#include <cppcms/application.h>
#include <cppcms/service.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/http_response.h>
#include <cppcms/form.h>

#include <filesystem>

#include "Website.h"
#include "content.h"
#include "services.h"
#include "rpc.h"
#include "database.h"

namespace fs = std::filesystem;

/**
 * Attaches services, assigns url mapping, and then sets the root of the webpage
 */
Website::Website(cppcms::service &srv) : cppcms::application(srv) {
    services::setLogfile(settings().get<std::string>("app.settings.admin.logfile"));
    services::database::open(settings().get<std::string>("app.settings.admin.paths.db"));
    services::setMediaPath(settings().get<std::string>("app.settings.admin.paths.media"));
    services::setCoverPath(settings().get<std::string>("app.settings.admin.paths.covers"));
    services::setImportPath(settings().get<std::string>("app.settings.admin.paths.import"));
    services::setPagesPath(settings().get<std::string>("app.settings.admin.paths.tmp"));

    fs::create_directories(services::getPagesPath());

    services::Log("Launching website");
    
    attach(new services::ReadingRPC(srv),"/reading-rpc(/(\\d+)?)?",0);
    attach(new services::DataRPC(srv),"/data-rpc(/(\\d+)?)?",0);

    dispatcher().assign("/",&Website::library,this);
    mapper().assign("library","/");
    
    dispatcher().assign("/upnext",&Website::upnext,this);
    mapper().assign("upnext","/upnext");
    
    dispatcher().assign("/collection/(\\w+)",&Website::collection,this,1);
    mapper().assign("collection","/collection/{1}");

    dispatcher().assign("/import",&Website::import,this);
    mapper().assign("import","/import");
    
    dispatcher().assign("/help",&Website::help,this);
    mapper().assign("help","/help");
    
    dispatcher().assign("/login",&Website::login,this);
    mapper().assign("login","/login");
    
    dispatcher().assign("/settings/user",&Website::user,this);
    mapper().assign("user","/settings/user");
    
    dispatcher().assign("/settings/account",&Website::account,this);
    mapper().assign("account","/settings/account");
    
    dispatcher().assign("/settings/general",&Website::general,this);
    mapper().assign("general","/settings/general");
    
    dispatcher().assign("/settings/acount-management",&Website::accountManagement,this);
    mapper().assign("account_management","/settings/acount-management");
    
    dispatcher().assign("/settings/media-management",&Website::mediaManagement,this);
    mapper().assign("media_management","/settings/media-management");
    
    dispatcher().assign("/settings/meintenance",&Website::meintenance,this);
    mapper().assign("meintenance","/settings/meintenance");

    dispatcher().assign("/403",&Website::forbidden,this);
    mapper().assign("forbidden","/403");

    mapper().root("");
} // Website()

/**
 * ini() is called by most views. Sets the sitewide title and redirects to the login page when no
 * user is loged on.
 * 
 * @param cnt any content view dirived from master
 * @return true if user is set
 */
bool Website::ini(content::Master &cnt) {
    cnt.title = settings().get<std::string>("app.title");
    if (!session().is_set("username")) {
        response().set_redirect_header("/login");
        return false;
    }
    return true;
}

/**
 * library() renders the Library view. The content is created by a call to database::getMedia(), which
 * rebuilds the media list before it is copied onto cnt.media and sorted by title. Then the view is rendered
 * and sent to the client.
 */
void Website::library() {
    content::Library cnt;
    if (!ini(cnt)) {
        return;
    }
    services::database::getMedia();
    cnt.media = services::getMediaList();
    std::stable_sort(cnt.media.begin(), cnt.media.end(), [](content::Item c1, content::Item c2) {
        return c1.sortTitle.compare(c2.sortTitle) < 0;
    });
    render("library", cnt);
}

/**
 * upnext() retnders the UpNext view.
 */
void Website::upnext() {
    content::UpNext cnt;
    if (!ini(cnt)) {
        return; 
    }
    render("upnext", cnt);
}

/**
 * collection() renders the Collection view. The content is created by a call to database::getCollectionMedia(),
 * which rebuilds the media list before it is copied onto cnt.books and sorted by volume. Then the view is rendered
 * and sent to the client.
 * 
 *  @param id id of the collection to be viewed
 */
void Website::collection(std::string id) {
    content::Collection cnt;
    if (!ini(cnt)) {
        return;
    }
    // TODO: adjust what is being searched -> refresh removes the name
    services::database::getMedia(); // temporary fix, but increases load times
    for (content::Item item : services::getMediaList()) {
        if (id == item.id) {
            cnt.collectionTitle = item.title;
            cnt.cover = item.cover;
            break;
        }
    }
    services::database::getCollectionMedia(id);
    cnt.books = services::getMediaList();
    render("collection", cnt);
}

/**
 * import() builds the import view from files in the import directory
 */
void Website::import() {
    content::Import cnt;
    if (!ini(cnt)) {
        return;
    }
    services::Log("Grabbing new files for importing");
    // TODO: make recursive_directory_iterator when I can handle it
    int count = 0;
    for (const auto& entry : fs::directory_iterator(services::getImportPath())) {
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
        cnt.imports.push_back(item);

        count++;
    }
    services::Log("Found " + std::to_string(count) + " new files");
    render("import", cnt);
}

/**
 * The following renders the Help page
 */
void Website::help() {
    content::Help cnt;
    cnt.title = settings().get<std::string>("app.title");
    render("help", cnt);
}

/**
 * login() renders the login page and hands login form submission with session creation. Also acts
 * as a method to logout.
 */
void Website::login() {
    content::Login cnt;
    cnt.title = settings().get<std::string>("app.title");
    if (request().request_method() == "POST" && session().is_set("prelogin")) {
        cnt.login.load(context());
        services::Log("Attempted login for user: " + cnt.login.username.value());
        if (cnt.login.validate() && services::database::validateLogin(cnt.login.username.value(), cnt.login.password.value())) {
            services::Log("Successful login");
            session().reset_session();
            session().erase("prelogin");
            session().set("username", cnt.login.username.value());
            // TODO: grab user preferences
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
bool Website::settingsView(content::Settings &cnt) {
    if (!ini(cnt)) {
        return false;
    }
    services::Log(session()["username"] + "is attempting to view settings");

    cnt.hasAdmin = false;
    std::string perms = services::database::getPermissions(session()["username"]);
    if (perms != "user" && perms != "admin") {
        services::Log("Access denined");
        response().set_redirect_header("/403");
        return false;
    }
    if ((std::string(typeid(cnt).name()) == "N7content4UserE") || (std::string(typeid(cnt).name()) == "N7content7AccountE")) {
        if (perms == "admin") {
            services::Log("Access granted with admin privilages");
            cnt.hasAdmin = true;
        }
        services::Log("Access granted with user privilages");
        return true;
    }
    if (perms != "admin") {
        services::Log("Access denined");
        response().set_redirect_header("/403");
        return false;
    }
    services::Log("Access granted with admin privilages");
    cnt.hasAdmin = true;
    return true;
}

/**
 * The following render the various Settings views
 * 
 * Check privileges of session -> only show user and account unless they have admin privileges
 * -> redirect to user if no admin privileges
 */
void Website::user() {
    content::User cnt;
    if (!settingsView(cnt)) {
        return;
    }
    render("user", cnt);
}
void Website::account() {
    content::Account cnt;
    if (!settingsView(cnt)) {
        return;
    }
    render("account", cnt);
}
void Website::general() {
    content::General cnt;
    if (!settingsView(cnt)) {
        return;
    }
    render("general", cnt);
}
void Website::accountManagement() {
    content::AccountManagement cnt;
    if(!settingsView(cnt)) {
        return;
    }
    render("account_management", cnt);
}
void Website::mediaManagement() {
    content::MediaManagement cnt;
    if (!settingsView(cnt)) {
        return;
    }
    render("media_management", cnt);
}
void Website::meintenance() {
    content::Meintenance cnt;
    if (!settingsView(cnt)) {
        return;
    }
    render("meintenance", cnt);
}

/**
 * forbidden() renders the 403 FORBIDDEN page
 */
void Website::forbidden() {
    response().status(cppcms::http::response::forbidden);
    content::Forbidden cnt;
    cnt.title = settings().get<std::string>("app.title");
    render("forbidden", cnt);
}

/**
 * Sets the override for the 404 page of the website
 */
void Website::main(std::string url) {
    if (!dispatcher().dispatch(url)) {
        response().status(cppcms::http::response::not_found);
        content::PageNotFound cnt;
        cnt.title = settings().get<std::string>("app.title");
        render("page_not_found", cnt);
    }
}
