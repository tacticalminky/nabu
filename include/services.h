#ifndef SERVICES_H
#define SERVICES_H

#include <string>
#include "content.h"

namespace services {

    void setLogfile(std::string const &file);
    void Log(std::string message);
    void closeLog();

    std::string getMediaPath();
    void setMediaPath(std::string const &path);
    
    std::string getCoverPath();
    void setCoverPath(std::string const &path);
    
    std::string getImportPath();
    void setImportPath(std::string const &path);
    
    std::string getPagesPath();
    void setPagesPath(std::string const &path);
    
    std::vector<content::Item> getMediaList();
    void addToMediaList(content::Item &item);
    void clearMediaList();

    float getZoom();
    void setZoom(float const value);
}

#endif