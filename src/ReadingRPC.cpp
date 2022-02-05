/*
This file is part of Nabu.

Nabu is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Nabu is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License along with Nabu. If not, see <https://www.gnu.org/licenses/>.
*/

/**
 * @author  Andrew Mink
 * @date    24 January 2022
 * 
 * ReadingRPC handles all of the calls related to viewing a book. 
 * Including:
 *      Loading pages for the book and sending html for the client to view
 * ##   Updating book and collection progress
 */

#include <cppcms/service.h>
#include <cppcms/rpc_json.h>

#include <mupdf/classes.h>
#include <filesystem>

#include "rpc.h"
#include "services.h"
#include "database.h"

namespace fs = std::filesystem;

namespace services {

    /**
     * Get the page chunk size from config.json and bind the functions to the applicaion
     */
    ReadingRPC::ReadingRPC(cppcms::service &srv) : cppcms::rpc::json_rpc_server(srv) {
        Log("Binding reading controls");
        
        pageChunkSizeForwards = settings().get<int>("app.settings.admin.page_chunk_size.forward");
        pageChunkSizeBackwards = settings().get<int>("app.settings.admin.page_chunk_size.backward");
            
        bind("loadInit",cppcms::rpc::json_method(&ReadingRPC::loadInit,this),method_role);
        bind("loadForwards",cppcms::rpc::json_method(&ReadingRPC::loadForwards,this),method_role);
        bind("loadBackwards",cppcms::rpc::json_method(&ReadingRPC::loadBackwards,this),method_role);
    }

    /**
     * Every called book is sent here with an id so that it can be processed to return a book
     *    1. The book is loaded from the location in the database with the given id and start page which is grabbed from the database
     *          1a) the start page keeps track of the loading progress so that the book can load in chunks
     *          1b) the book is cached so that it is not continuously opened and closed
     * ## 2. Checks to see if the book is reflowable, continueing this step if it is
     *          2a) adjusts the reflowable book to the user's preferences
     *    3. Opens the start page and saves a png of the in the tmp directory 
     *    4. Creates an image html tag with the address of the stored page and adds it to the returnHTML
     *    5. Repeat steps 3-4 for every page within the pageChunkSizeForwards limit
     *    6. Return the html with other information in a json formate for the client to use to view the book
     *
     * @param setId id of the book that is to be viewed
     */
    void ReadingRPC::loadInit(std::string const &setId) {
        if (!database::bookExists(setId)) {
            Log("No book exits with id: " + setId);
            return_error("Given id is not valid");
            return;
        }
        id = setId;
             
        std::string file =  getMediaPath() + database::fetchFile(id);
        doc = new mupdf::Document(file.c_str());
        myMatrix = mupdf::Matrix();
        float zoom = getZoom();
        myMatrix.scale(zoom / 100, zoom / 100);
        myColor = mupdf::device_rgb();
        pageCount = doc->count_pages();
                
        Log("Page Count: " + std::to_string(pageCount) + "\nFilePath: " + file.c_str());

        currentPage = database::getProgress(session()["username"], id);
        firstPageLoaded = currentPage;
        int endPage;
        if (firstPageLoaded + pageChunkSizeForwards < pageCount) {
            endPage = firstPageLoaded + pageChunkSizeForwards;
        } else {
            endPage = pageCount;
        }

        std::string returnHTML;
        for(int pageNum = firstPageLoaded; pageNum < endPage; pageNum++) {
            returnHTML.append(generateImg(pageNum));
        }

        lastPageLoaded = endPage;
            
        cppcms::json::value json;
        json["html"] = returnHTML;
        json["firstLoaded"] = firstPageLoaded;
        json["lastLoaded"] = endPage;
        json["pageCount"] = pageCount;
        json["isDoubleView"] = false;
        json["forwardsChunk"] = pageChunkSizeForwards;
        json["backwardsChunk"] = pageChunkSizeBackwards;
        return_result(json);
    } // loadInit()

    /**
     * Checks if there is an open book or not and the validity of the paramaters, then loads a pageChunkSizeForwards
     * the same way loadInit does, and finaly return the html with the new first and last loaded pages
     *  
     * @param current the current page sent from the client
     * @param start the last page loaded by the client to be checked against the last page of loaded by the server
     */
    void ReadingRPC::loadForwards(int const &current, int const &start) {
        if (!doc) {
            return_error("No document is open");
            return;
        }
        if (current < 0 || current >= pageCount || current > start || start >= pageCount) {
            return_error("Page range OUT_OF_BOUNDS");
            return;
        }
        if (start != lastPageLoaded) {
            return_error("Page range missing");
            return;
        }
        currentPage = current;
            
        int endPage;
        if (lastPageLoaded + pageChunkSizeForwards < pageCount) {
            endPage = lastPageLoaded + pageChunkSizeForwards;
        } else {
            endPage = pageCount;
        }
            
        std::string returnHTML;
        for(int pageNum = lastPageLoaded; pageNum < endPage; pageNum++) {
            returnHTML.append(generateImg(pageNum));
        }
            
        lastPageLoaded = endPage;
            
        cppcms::json::value json;
        json["html"] = returnHTML;
        json["firstLoaded"] = firstPageLoaded;
        json["lastLoaded"] = endPage;
        return_result(json);
    } // loadForwards()

    /**
     * Checks if there is an open book or not and the validity of the paramaters, then loads a pageChunkSizeBackwards
     * the same way loadInit does, and finaly return the html with the new first and last loaded pages
     *  
     * @param current the current page sent from the client
     * @param start the first page loaded by the client to be checked against the first page of loaded by the server
     */
    void ReadingRPC::loadBackwards(int const &current, int const &start) {
        if (!doc) {
            return_error("No document is open");
            return;
        }
        if (current < 0 || current < start) {
            return_error("Page range OUT_OF_BOUNDS");
            return;
        }
        if (start != firstPageLoaded) {
            return_error("Page range missing");
            return;
        }
        currentPage = current;
            
        int endPage;
        if (firstPageLoaded - pageChunkSizeBackwards < 0) {
            endPage = 0;
        } else {
            endPage = firstPageLoaded - pageChunkSizeBackwards;
        }
            
        std::string returnHTML;
        for(int pageNum = endPage; pageNum < firstPageLoaded; pageNum++) { 
            returnHTML.append(generateImg(pageNum));
        }

        firstPageLoaded = endPage;

        cppcms::json::value json;
        json["html"] = returnHTML;
        json["firstLoaded"] = endPage;
        json["lastLoaded"] = lastPageLoaded;
        return_result(json);
    } // loadBackwards()

    /**
     * generateImg() generates the page located at the pageNumber into an image
     * 
     * @param fileName name of the page file
     */
    std::string ReadingRPC::generateImg(int const &pageNumber) {
        if (pageNumber >= pageCount) {
            throw std::invalid_argument("Page " + std::to_string(pageNumber) + " is out of bounds for the book");
        }
        std::string filename = id + "-" + std::to_string(pageNumber) + ".png";
        fs::path image(getPagesPath() + filename);
        Log("Creating image from page: " + image.string());
                
        if(!fs::exists(image)) {
            doc->new_pixmap_from_page_number(pageNumber, myMatrix, myColor, 0)
                .save_pixmap_as_png(image.c_str());
        }
             
        return "<img class='myPages' src='/pages/" + filename + "'>";
    }

} // namespace services