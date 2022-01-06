#include <cppcms/url_dispatcher.h>
#include <cppcms/url_mapper.h>
#include <cppcms/rpc_json.h>

namespace apps {

/**
 * 
 */
class JSONService : public cppcms::rpc::json_rpc_server {
public:
    JSONService(cppcms::service &srv) : cppcms::rpc::json_rpc_server(srv) {
        std::cout << "*************************** JSONService binding ***************************" << std::endl;
            
        bind("test",cppcms::rpc::json_method(&JSONService::test,this),method_role);
    }

    void test(int num) {
        std::cout << "We got it: " << num << std::endl;
        return_result(num);
    }
};

} // end of namespace apps