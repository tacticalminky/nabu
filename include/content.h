#ifndef CONTENT_H
#define CONTENT_H

#include <cppcms/view.h>

#include "items.h"
#include "login_form.h"

namespace content {

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
    struct Login : public Master {
        LoginForm login;
    };
    struct PageNotFound : public Master {};
    struct Forbidden : public Master {};
    
    // Settings views
    struct Settings : public Master {
        bool hasAdmin;
    };
    struct User : public Settings {};
    struct Account : public Settings {};
    struct General : public Settings {};
    struct AccountManagement : public Settings {};
    struct MediaManagement : public Settings {};
    struct Meintenance : public Settings {};
}

#endif