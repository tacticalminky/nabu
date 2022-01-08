#ifndef CONTENT_H
#define CONTENT_H

#include <cppcms/view.h>

namespace content {
    // Media Item
    struct Item {
        std::string id;
        std::string title;
        std::string sortTitle;
        std::string cover;      // set a default value for NULL
        std::string volume;    // can leave NULL
        std::string issue;
        float progress;         // value or 0
        bool isCollection = false;
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
        std::string collection_id;
        std::string collection_title;
    };

    // Misc views
    struct Import : public Master {};
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