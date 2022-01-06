import AbstactView from "./AbstactView.js";

export default class extends AbstactView {
    constructor(params) {
        super(params);
        this.setTitle("Library");
    }

    async getHtml() {
        return `
            <nav class="library-nav">
                <a href="#view-select">View: All</a>
                <div style="flex-grow: 2;"></div>
                <a href="/media/upnext" data-link>Up Next</a>
                <a href="/media/library" data-link>Library</a>
                <div style="flex-grow: 2.5;"></div>
            </nav>
            <div class="library-content">${this.getContent(this.params)}</div>
        `;
    }

    getContent(params) {
        if (params.view === "library") {
            return `Dynamic Library View`;
        }
        if (params.view === "upnext") {
            return `Dynamic Up Next View`;
        }
        // 404 Error
        throw "PageNotFound";
    }
}