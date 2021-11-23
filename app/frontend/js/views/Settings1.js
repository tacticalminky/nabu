import AbstactView from "./AbstactView.js";

export default class extends AbstactView {
    constructor() {
        super();
        this.setTitle("Settings 1");
    }

    async getHtml() {
        return `
            <nav class="settings-nav">
                <a href="/settings/page1" data-link>Option 1</a>
                <a href="#page2" data-link>Option 2</a>
                <a href="#page3" data-link>Option 3</a>
            </nav>
            <div class="settings-content">
                <h2>Options Group 1</h2>
                <h3>Option 1</h3>
                <p>Description for Option 1</p>
                <h3>Option 2</h3>
                <p>Description for Option 2</p>
            </div>
        `;
    }
}