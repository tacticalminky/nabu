#ifndef RPC_H
#define RPC_H

#include <cppcms/service.h>
#include <cppcms/rpc_json.h>

#include <mupdf/classes.h>

#include "../views/content.h"

namespace services {
    std::string mediaPath, coverPath, importPath, pagesPath;
    std::vector<content::Item> glb_media;

    class ReadingRPC : public cppcms::rpc::json_rpc_server {
    private:
        int pageChunkSizeForwards, pageChunkSizeBackwards; // number of pages to be loaded per call defined in config.json
        int pageCount, firstPageLoaded, lastPageLoaded, currentPage;
        std::string id;
        mupdf::Document *doc;
        mupdf::Matrix myMatrix;
        mupdf::Colorspace myColor;
    
    public:
        ReadingRPC(cppcms::service &srv);

    private:
        void loadInit(std::string const &setId);
        void loadForwards(int const &current, int const &start);
        void loadBackwards(int const &current, int const &start);
        std::string generateImgTag(std::string const &filename);
    };

    class DataRPC : public cppcms::rpc::json_rpc_server {
    public:
        DataRPC(cppcms::service &srv);

    private:
        void import(cppcms::json::value json);
    };

}

#endif