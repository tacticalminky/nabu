import AbstactView from "./AbstactView.js";

export default class extends AbstactView {
    constructor() {
        super();
        this.setTitle("Help");
    }

    async getHtml() {
        return `
            <h2>Help</h2>
            <p>
                This is the help section.<br>
                There is currently no help right now.
            </p>

        `;
    }
}