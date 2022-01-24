/*
This file is part of Nabu.

Nabu is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Nabu is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License along with Nabu. If not, see <https://www.gnu.org/licenses/>.
*/

/**
 * @author  Andrew Mink
 * @date    24 January 2022
 */

#include <string>
#include "../views/content.h"

#include "services.h"

namespace services {
    
    std::string mediaPath;
    std::string getMediaPath() {
        return mediaPath;
    };
    void setMediaPath(std::string const &path) {
        mediaPath = path;
    };
    
    std::string coverPath;
    std::string getCoverPath() {
        return coverPath;
    };
    void setCoverPath(std::string const &path) {
        coverPath = path;
    };
    
    std::string importPath;
    std::string getImportPath() {
        return importPath;
    };
    void setImportPath(std::string const &path) {
        importPath = path;
    };
    
    std::string pagesPath;
    std::string getPagesPath() {
        return pagesPath;
    };
    void setPagesPath(std::string const &path) {
        pagesPath = path;
    };

    std::vector<content::Item> glb_media;
    std::vector<content::Item> getMediaList() {
        return glb_media;
    };
    void addToMediaList(content::Item &item) {
        glb_media.push_back(item);
    };
    void clearMediaList() {
        glb_media.clear();
    }

} // namespace services