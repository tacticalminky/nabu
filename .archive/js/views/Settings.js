import AbstactView from "./AbstactView.js";

export default class extends AbstactView {
    constructor(params) {
        super(params);
        this.setTitle("Settings");
    }

    async getHtml() {
        return `
            <nav class="settings-nav">
                <a href="/settings/user/user" data-link>User</a>
                <a href="/settings/user/account" data-link>Account</a>
                <a href="/settings/admin/general" data-link>General</a>
            </nav>
            <div class="settings-content">${this.getContent(this.params)}</div>
        `;
    }

    getContent(params) {
        if (params.privalage === "user") {
            if (params.group === "user") {
                return `
                    <h2>Options Group 1</h2>
                    <h3>Option 1</h3>
                    <p>Description for Option 1</p>
                    <h3>Option 2</h3>
                    <p>Description for Option 2</p>
                `;
            }
            if (params.group === "account") {
                return ``;
            }
        }
        if (params.privalage === "admin") {
            if (params.group === "general") {
                return `
                    <h2>General Admin Settings</h2>
                    <p>
                        Hide admin settings from general users. Check from server for which settings are viewable.<br>
                        For each admin settings verify user privilages on server. <br>
                        Throw a 403 Forbidden error 
                    </p>
                `;
            }
            if (params.group === "account-management") {
                return ``;
            }
            if (params.group === "media-management") {
                return ``;
            }
            if (params.group === "meintenance") {
                return ``;
            }
        }
        // 404 Error
        throw "PageNotFound";
    }
}