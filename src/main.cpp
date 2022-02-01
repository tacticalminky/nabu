/*
This file is part of Nabu.

Nabu is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Nabu is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License along with Nabu. If not, see <https://www.gnu.org/licenses/>.
*/

/**
 * @author  Andrew Mink
 * @version alpha
 * @date    31 January 2022
 * 
 * 
 */

#include <cppcms/application.h>
#include <cppcms/applications_pool.h>

#include "Website.h"
#include "services.h"

/**
 * Starts and runs the Website application
 */
int main(int argc, char ** argv) {
    try {
        cppcms::service srv(argc,argv);
        srv.applications_pool().mount(cppcms::create_pool<Website>());
        srv.run();
    } catch(std::exception const &e) {
        services::Log(e.what());
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
