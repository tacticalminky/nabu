#ifndef LOGIN_FORM_H
#define LOGIN_FORM_H

#include <cppcms/form.h>

namespace content {
    struct LoginForm : public cppcms::form {
        cppcms::widgets::text username;
        cppcms::widgets::password password;
        cppcms::widgets::submit submit;

        LoginForm() {
            username.message("Username:");
            username.non_empty();
            add(username);

            password.message("Password:");
            password.non_empty();
            add(password);

            submit.message("Login");
            add(submit);
        }
    };

}

#endif