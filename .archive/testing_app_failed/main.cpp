/**
 * 
 */

#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <cppcms/service.h>
#include <cppcms/url_dispatcher.h>

#include "views/content.h"
#include "apps/apps.h"

class WebSite : public cppcms::application {
public:
    /**
     * 
     */
    WebSite(cppcms::service &srv) : cppcms::application(srv) {
        std::cout << "****************************** WebSite binding ******************************" << std::endl;
       
        attach( new apps::Library(srv),
            "library",
            "/{1}",
            "(/(\\w++)?)?",2);

        attach( new apps::Settings(srv),
            "settings",
            "/settings/{1}",
            "/settings/(\\w+)",1);

        attach( new apps::JSONService(srv) );
        
        std::cout << "***************************** Attached services *****************************" << std::endl;

        mapper().root(settings().get<std::string>("WebSite.root"));
        mapper().assign("user_thread","/{1}");
    }
    
    /**
     * overrides 404 page
     */
    virtual void main(std::string url) {
        if (!dispatcher().dispatch(url)) {
            response().status(cppcms::http::response::not_found);
            content::PageNotFound cnt;
            render("page_not_found", cnt);
        }
    }
};

/**
 * 
 */
int main(int argc, char ** argv) {
    try {
        cppcms::service srv(argc,argv);
        srv.applications_pool().mount(cppcms::create_pool<WebSite>());
        srv.run();
    } catch(std::exception const &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}