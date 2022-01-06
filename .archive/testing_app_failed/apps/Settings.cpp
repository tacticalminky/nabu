/**
 * 
 */

#include <cppcms/application.h>
#include <cppcms/service.h>
#include <cppcms/url_dispatcher.h>

#include "../views/content.h"

namespace apps {

/**
 * 
 */
class Settings : public cppcms::application {
public:
    /**
     * 
     */
    Settings(cppcms::service &srv) : cppcms::application(srv) {
        std::cout << "****************************** Settings binding ******************************" << std::endl;
        
        dispatcher().assign("user",&Settings::user,this);
        mapper().assign("user","user");
        
        dispatcher().assign("account",&Settings::account,this);
        mapper().assign("account","account");
        
        dispatcher().assign("general",&Settings::general,this);
        mapper().assign("general","general");
        
        dispatcher().assign("acount-management",&Settings::account_management,this);
        mapper().assign("account_management","acount-management");
        
        dispatcher().assign("media-management",&Settings::media_management,this);
        mapper().assign("media_management","media-management");
        
        dispatcher().assign("meintenance",&Settings::meintenance,this);
        mapper().assign("meintenance","meintenance");
    }

private:

    /**
     * Various Settings Views
     */
    void user() {
        content::User cnt;
        render("user", cnt);
    }
    void account() {
        content::Account cnt;
        render("account", cnt);
    }
    void general() {
        content::General cnt;
        render("general", cnt);
    }
    void account_management() {
        content::AccountManagement cnt;
        render("account_management", cnt);
    }
    void media_management() {
        content::MediaManagement cnt;
        render("media_management", cnt);
    }
    void meintenance() {
        content::Meintenance cnt;
        render("meintenance", cnt);
    }
};

} // end of namespace apps