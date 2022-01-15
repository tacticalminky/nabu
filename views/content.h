#ifndef CONTENT_H
#define CONTENT_H

#include <cppcms/view.h>
#include <cppcms/form.h>

namespace content {
    // Media Item
    struct Item {
        std::string id;
        std::string title;
        std::string sortTitle;
        std::string volume;    // can leave NULL
        std::string issue;
        float progress;         // value or 0
        bool isCollection = false;
    };
    
    // Import Item
    struct ImportItem {
        std::string title;    // default filename w/o extensions
        std::string sortTitle;    // default title w/o the
        int volNum = 0;
        int issNum = 0;
        std::string isbn;
        std::string date;
        std::string author;
        std::string illistrator;
        std::string publisher;
        std::string generes;  // make into a select with multiple or checkbox dorpdown
        std::string type;   // book, comic, or manga
        std::string collection;   // select with new option
        std::string file;
    };

    // Login Form Info
    struct LoginForm : public cppcms::form {
        cppcms::widgets::text username;
        cppcms::widgets::password password;
        cppcms::widgets::submit submit;

        LoginForm() {
            username.message("Username:");
            username.non_empty();
            add(username);

            password.message("Password:");
            password.non_empty();
            add(password);

            submit.message("Login");
            add(submit);
        }
    };
    
    // Base for all the views
    struct Master : public cppcms::base_content {
        std::string title;
    };

    // Library and reading views
    struct Library : public Master {
        std::vector<Item> media;
    };
    struct UpNext : public Library {};
    struct Collection : public Library {
        std::vector<Item> books;
        std::string collectionTitle;
    };

    // Misc views
    struct Import : public Master {
        std::vector<ImportItem> imports;
    };
    struct Help : public Master {};
    struct Login : public Master {};
    struct PageNotFound : public Master {};
    
    // Settings views
    struct Settings : public Master {};
    struct User : public Settings {};
    struct Account : public Settings {};
    struct General : public Settings {};
    struct AccountManagement : public Settings {};
    struct MediaManagement : public Settings {};
    struct Meintenance : public Settings {};
}

#endif