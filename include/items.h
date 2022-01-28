#ifndef ITEMS_H
#define ITEMS_H

#include <string>

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

}

#endif