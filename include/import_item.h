#ifndef IMPORT_ITEM_H
#define IMPORT_ITEM_H

#include <string>

namespace content {
   
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