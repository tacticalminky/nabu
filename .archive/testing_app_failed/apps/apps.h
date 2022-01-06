#ifndef WEBSITE_H
#define WEBSITE_H
#pragma once

#include <cppcms/application.h>

namespace apps {
    
    struct Library : public  cppcms::application {
    public:
        Library(cppcms::service &srv) : cppcms::application(srv) {};
    };

    struct Settings : public cppcms::application {
    public:
        Settings(cppcms::service &srv) : cppcms::application(srv) {};
    };

    struct JSONService : public  cppcms::application {
    public:
        JSONService(cppcms::service &srv) : cppcms::application(srv) {};
    };
    
}

#endif