#ifndef DATABASE_H
#define DATABASE_H

#include <cppcms/json.h>
#include "database.h"

namespace services {

    namespace database {
        void open(std::string const &filepath);
        void getMedia();
        void getCollectionMedia(std::string const &collection_id);
        std::string fetchFile(std::string const &media_id);
        void import(cppcms::json::value json);
        bool validateLogin(std::string const &username, std::string const &password);
        std::string getPermissions(std::string const &username);
        void close();
    }

}

#endif