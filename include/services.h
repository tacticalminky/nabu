#ifndef SERVICES_H
#define SERVICES_H

#include <string>
#include "content.h"

namespace services {

    void setLogfile(std::string const &file);
    void Log(std::string message);

    std::string getMediaPath();
    std::string getCoverPath();
    std::string getImportPath();
    std::string getPagesPath();
    std::vector<content::Item> getMediaList();
    
    void setMediaPath(std::string const &path);
    void setCoverPath(std::string const &path);
    void setImportPath(std::string const &path);
    void setPagesPath(std::string const &path);
    void addToMediaList(content::Item &item);
    void clearMediaList();

}

#endif