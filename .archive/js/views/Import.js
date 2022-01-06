import AbstactView from "./AbstactView.js";

export default class extends AbstactView {
    constructor(params) {
        super(params);
        this.setTitle("Import");
    }

    async getHtml() {
        return ``;
    }
}