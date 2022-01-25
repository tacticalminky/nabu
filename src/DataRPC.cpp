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
 * DataRPC handles all the call related to importing books
 */

#include <cppcms/service.h>
#include <cppcms/rpc_json.h>

#include "rpc.h"
#include "database.h"

namespace services {

    DataRPC::DataRPC(cppcms::service &srv) : cppcms::rpc::json_rpc_server(srv) {
        std::cout << "Binding data controls" << std::endl;
        bind("import",cppcms::rpc::json_method(&DataRPC::import,this),method_role);
    }

    /**
     * import() 
     */
    void DataRPC::import(cppcms::json::value json) {
        std::cout << "import() called" << std::endl;
        json.save(std::cout,cppcms::json::readable); // prints json
        try {
            database::import(json);
            return_result("Successfully added to the database");
        } catch (std::exception const &e) {
            std::cout << e.what() << std::endl;
            return_error("Error: " + std::string(e.what()));
            return;
        }
    } // import()

} // namespace services