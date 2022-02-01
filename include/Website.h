#ifndef WEBSITE_H
#define WEBSITE_H

#include <cppcms/application.h>
#include <cppcms/service.h>

#include "content.h"

class Website : public cppcms::application {
public:
    Website(cppcms::service &srv);

private:
    bool ini(content::Master &cnt);
    void library();
    void upnext();
    void collection(std::string id);
    void import();
    void help();
    void login();
    bool settingsView(content::Settings &cnt);
    void user();
    void account();
    void general();
    void accountManagement();
    void mediaManagement();
    void meintenance();
    void forbidden();
    virtual void main(std::string url);
}; // class Website

#endif