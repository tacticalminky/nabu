import Library from "./views/Library.js";
import UpNext from "./views/UpNext.js";
import Settings from "./views/Settings.js";
import Settings1 from "./views/Settings1.js";
import Help from "./views/Help.js";
import Import from "./views/Import.js";

const navigateTo = url => {
    history.pushState(null, null, url);
    router();
}

const router = async () => {
    const routes = [
        { path: "/", view: Library },
        { path: "/upnext", view: UpNext },
        { path: "/settings", view: Settings },
        { path: "/settings/page1", view: Settings1 },
        { path: "/help", view: Help },
        { path: "/import", view: Import }
    ];

    // Test routes for potential match
    const potentialMatches = routes.map(route => {
        return {
            route: route,
            hasMatch: location.pathname === route.path
        };
    });

    let match = potentialMatches.find(potentialMatches => potentialMatches.hasMatch);

    if (!match) {
        match = {
            route: route[0], // make a 404 page for here
            hasMatch: true
        }
    }

    const view = new match.route.view();

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