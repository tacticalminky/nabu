import Library from "./views/Library.js";
import Settings from "./views/Settings.js";
import Help from "./views/Help.js";
import Import from "./views/Import.js";

const pathToRegex = path => new RegExp("^" + path.replace(/\//g, "\\/").replace(/:\w+/g, "(.+)") + "$");

const getParams = match => {
    const values = match.result.slice(1);
    const keys = Array.from(match.route.path.matchAll(/:(\w+)/g)).map(result => result[1]);
    return Object.fromEntries(keys.map((key, i) => {
        return [key, values[i]];
    }));
};

const navigateTo = url => {
    history.pushState(null, null, url);
    router();
}

const router = async () => {
    const routes = [
        { path: "/media/:view", view: Library },
        { path: "/settings/:privalage/:group", view: Settings },
        { path: "/help", view: Help },
        { path: "/import", view: Import }
    ];

    // Test routes for potential match
    const potentialMatches = routes.map(route => {
        return {
            route: route,
            result: location.pathname.match(pathToRegex(route.path))
        };
    });

    let match = potentialMatches.find(potentialMatches => potentialMatches.result !== null);

    if (!match) {
        match = {
            route: routes[3], // Help page - PageNotFound
            result: true
        }
    }

    console.log(match.route);
    const view = new match.route.view(getParams(match));

    document.querySelector("#main").innerHTML = await view.getHtml();
};

window.addEventListener("popstate", router);

document.addEventListener("DOMContentLoaded", () => {
    document.body.addEventListener("click", e => {
        // removes page refress if has this atribute
        if (e.target.matches("[data-link]")) {
            e.preventDefault();
            navigateTo(e.target.href);
        }
    });
    router();
});