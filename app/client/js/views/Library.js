import AbstactView from "./AbstactView.js";

export default class extends AbstactView {
    constructor() {
        super();
        this.setTitle("Library");
    }

    async getHtml() {
        return `
            <nav class="library-nav">
                <a href="#view-select">View: All</a>
                <div style="flex-grow: 2;"></div>
                <a href="/upnext" data-link>Up Next</a>
                <a href="/" data-link>Library</a>
                <div style="flex-grow: 2.5;"></div>
            </nav>
            <div class="library-content"></div>
        `;
    }
}