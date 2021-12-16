import AbstactView from "./AbstactView.js";

export default class extends AbstactView {
    constructor() {
        super();
        this.setTitle("Settings");
    }

    async getHtml() {
        return `
            <nav class="settings-nav">
                <a href="/settings/page1" data-link>Option 1</a>
                <a href="#page2" data-link>Option 2</a>
                <a href="#page3" data-link>Option 3</a>
            </nav>
            <div class="settings-content">
                <p>Expand Settings Navbar with links down here and remove the navbar</p>
            </div>
        `;
    }
}