#ifndef ITEM_H
#define ITEM_H

#include <string>

namespace content {
    
    // Media Item
    struct Item {
        std::string id;
        std::string title;
        std::string sortTitle;
        std::string volume;    // can leave NULL
        std::string issue;
        std::string cover;
        float progress;         // value or 0
        bool isCollection = false;
    };

}

#endif