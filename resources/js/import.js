/*
This file is part of Nabu.

Nabu is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Nabu is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License along with Nabu. If not, see <https://www.gnu.org/licenses/>.
*/

// takes in the form of the import item in json format and sends it to the server
async function importItem(sendJson) {
    data = {
        method: "import",
        params: [ sendJson ],
        id: 21
    }
    
    try {
        let response = await fetch('/data-rpc', {
            method: 'POST',
            headers: {
                'Content-Type' : 'application/json'
            },
            body: JSON.stringify(data)
        });

        if (!response.ok) {
            throw new Error(`HTTP error: ${response.status}`);
        }

        let json = await response.json();
        console.log(json);
        if (json.error !== null) {
            throw new Error(`RPC error: ${json.error}`);
        }
        return true;
    } catch(e) {
        console.error(e.message);
        return false;
    }
} // importItem()

// there must be at least on of the three paramaters for the seach to function
// for best results the isbn should be used or the title and author together
// no dashes in isbn
async function matchGoogleAPI(isbn, title, author, publisher, form) {
    try {
        let response = await fetch(`https://www.googleapis.com/books/v1/volumes?q=`
             + ((isbn) ? `+isbn:${isbn}` : "" )
             + ((title) ? `intitle:${title}` : "")
             + ((author) ? `+inauthor:${author}` : "" )
             + ((publisher) ? `+inpublisher:${publisher}` : "" )
             + `&orderBy=relevance`);

        if (!response.ok) {
            throw new Error(`HTTP error: ${response.status}`);
        }

        let json = await response.json();
        console.log(json);

        viewMatches(form, json.items)
    } catch(e) {
        console.error(e.message);
    }
} // searchGoogleAPI()

function viewMatches(form, matches) {
    document.getElementById("api-modal").innerHTML = "";
    document.getElementById("api-modal").style.visibility = "visible";
    for (const match of matches) {
        // change to grab all authors and catagories
        document.getElementById("api-modal").innerHTML += `
            <div class="potential-match">
                <img src="${((match.volumeInfo.imageLinks) ? match.volumeInfo.imageLinks.smallThumbnail : "" )}" alt="Cover Preview">
                <table>
                    <tr>
                        <th colspan="2" class="title">${match.volumeInfo.title}</th>
                    </tr>
                    <tr>
                        <th>ISBN:</th>
                        <td>${((match.volumeInfo.industryIdentifiers) ? match.volumeInfo.industryIdentifiers[0].identifier : "" )}</td>
                    </tr>
                    <tr>
                        <th>Date:</th>
                        <td>${match.volumeInfo.publishedDate}</td>
                    </tr>
                    <tr>
                        <th>Author:</th>
                        <td>${((match.volumeInfo.authors) ? match.volumeInfo.authors[0] : "" )}</td>
                    </tr>
                    <tr>
                        <th>Illistrator:</th>
                        <td></td>
                    </tr>
                    <tr>
                        <th>Publisher:</th>
                        <td>${match.volumeInfo.publisher}</td>
                    </tr>
                    <tr> 
                        <th>Generes:</th>
                        <td>${((match.volumeInfo.categories) ? match.volumeInfo.categories[0] : "" )}</td>
                    </tr>
                    <tr>
                        <th>Collection:</th>
                        <td>${((match.volumeInfo.seriesInfo) ? match.volumeInfo.seriesInfo.title : "" )}</td>
                    </tr>
                    <tr>
                        <th>Volume:</th>
                        <td>${((match.volumeInfo.seriesInfo) ? match.volumeInfo.seriesInfo.bookDisplayNumber : "" )}</td>
                    </tr>
                    <tr class="source">
                        <td><a href="${match.volumeInfo.canonicalVolumeLink}" target="_blank">Source</a><td>
                        <td><button type="button" id="${match.id}">Select Match</button></td>
                    </tr>
                </table>
            </div>
        `;
        document.getElementById(match.id).addEventListener("click", function() { matchForm(form,match); });
    }
    // fetch more
}

function matchForm(form, match) {
    console.log("Calling match");
    // newValue for all
    // newValue for new -> currently null
    document.getElementById("api-modal").innerHTML = `
        <div class="match">
            <table>
                <tr>
                    <th class="hrow">Group</th>
                    <th class="hrow">Current</th>
                    <th class="hrow">New</th>
                </tr>
                <tr>
                    <th>Title:</th>
                    <td>${form.elements['title'].value}</td>
                    <td>${match.volumeInfo.title}</td>
                    <td class="grab-btn"><button type="button" id="title-btn">Grab New</button></td>
                </tr>
                <tr>
                    <th>ISBN:</th>
                    <td>${form.elements['isbn'].value}</td>
                    <td>${((match.volumeInfo.industryIdentifiers) ? match.volumeInfo.industryIdentifiers[0].identifier : "" )}</td>
                    <td class="grab-btn"><button type="button" id="isbn-btn">Grab New</button></td>
                </tr>
                <tr>
                    <th>Date:</th>
                    <td>${form.elements['date'].value}</td>
                    <td>${match.volumeInfo.publishedDate}</td>
                    <td class="grab-btn"><button type="button" id="date-btn">Grab New</button></td>
                </tr>
                <tr>
                    <th>Author:</th>
                    <td>${form.elements['author'].value}</td>
                    <td>${((match.volumeInfo.authors) ? match.volumeInfo.authors[0] : "" )}</td>
                    <td class="grab-btn"><button type="button" id="author-btn">Grab New</button></td>
                </tr>
                <tr>
                    <th>Illistrator:</th>
                    <td>${form.elements['illistrator'].value}</td>
                    <td></td>
                    <td class="grab-btn"><button type="button" id="illistrator-btn">Grab New</button></td>
                </tr>
                <tr>
                    <th>Publisher:</th>
                    <td>${form.elements['publisher'].value}</td>
                    <td>${match.volumeInfo.publisher}</td>
                    <td class="grab-btn"><button type="button" id="publisher-btn">Grab New</button></td>
                </tr>
                <tr> 
                    <th>Generes:</th>
                    <td>${form.elements['genere'].value}</td>
                    <td>${((match.volumeInfo.categories) ? match.volumeInfo.categories[0] : "" )}</td>
                    <td class="grab-btn"><button type="button" id="genere-btn">Grab New</button></td>
                </tr>
                <tr>
                    <th>Collection:</th>
                    <td>${form.elements['collection'].value}</td>
                    <td>${((match.volumeInfo.seriesInfo) ? match.volumeInfo.seriesInfo.title : "" )}</td>
                    <td class="grab-btn"><button type="button" id="collection-btn">Grab New</button></td>
                </tr>
                <tr>
                    <th>Volume:</th>
                    <td>${form.elements['vnum'].value}</td>
                    <td>${((match.volumeInfo.seriesInfo) ? match.volumeInfo.seriesInfo.bookDisplayNumber : "" )}</td>
                    <td class="grab-btn"><button type="button" id="vnum-btn">Grab New</button></td>
                </tr>
                <tr>
                    <th>Issue:</th>
                    <td>${form.elements['inum'].value}</td>
                    <td>${((match.volumeInfo.seriesInfo) ? match.volumeInfo.seriesInfo.bookDisplayNumber : "" )}</td>
                    <td class="grab-btn"><button type="button" id="inum-btn">Grab New</button></td>
                </tr>
                <tr>
                    <td colspan="2"><button type="button" onclick="closeAPIModal()">Exit</button></td>
                </tr>
            </table>
        </div>
    `;

    document.getElementById("title-btn").addEventListener("click", function() {
        newValue(form.elements['title'], match.volumeInfo.title);
        newValue(form.elements['sort-title'], match.volumeInfo.title);
        matchForm(form, match);
    });
    document.getElementById("isbn-btn").addEventListener("click", function() {
        newValue(form.elements['isbn'], ((match.volumeInfo.industryIdentifiers) ? match.volumeInfo.industryIdentifiers[0].identifier : null ));
        matchForm(form, match);
    });
    document.getElementById("date-btn").addEventListener("click", function() {
        newValue(form.elements['date'], match.volumeInfo.publishedDate);
        matchForm(form, match);
    });
    document.getElementById("author-btn").addEventListener("click", function() {
        newValue(form.elements['author'], ((match.volumeInfo.authors) ? match.volumeInfo.authors[0] : null ));
        matchForm(form, match);
    });
    document.getElementById("illistrator-btn").addEventListener("click", function() {
        newValue(form.elements['illistrator'], null);
        matchForm(form, match);
    });
    document.getElementById("publisher-btn").addEventListener("click", function() {
        newValue(form.elements['publisher'], match.volumeInfo.publisher);
        matchForm(form, match);
    });
    document.getElementById("genere-btn").addEventListener("click", function() {
        newValue(form.elements['genere'], ((match.volumeInfo.categories) ? match.volumeInfo.categories[0] : null ));
        matchForm(form, match);
    });
    document.getElementById("collection-btn").addEventListener("click", function() {
        newValue(form.elements['collection'], ((match.volumeInfo.seriesInfo) ? match.volumeInfo.seriesInfo.title : null ));
        matchForm(form, match);
    });
    document.getElementById("vnum-btn").addEventListener("click", function() {
        newValue(form.elements['vnum'], ((match.volumeInfo.seriesInfo) ? match.volumeInfo.seriesInfo.bookDisplayNumber : null ));
        matchForm(form, match);
    });
    document.getElementById("inum-btn").addEventListener("click", function() {
        newValue(form.elements['inum'], ((match.volumeInfo.seriesInfo) ? match.volumeInfo.seriesInfo.bookDisplayNumber : null ));
        matchForm(form, match);
    });
} // match()

function newValue(formElement, val) {
    formElement.value = val;
} // newValue()

function closeAPIModal() {
    document.getElementById("api-modal").style.visibility = "hidden";
    document.getElementById("api-modal").innerHTML = "";
} // closeAPIModal()

// sets what to do for each form on submit
const forms = document.forms;
for (const form of forms) {
    form.addEventListener("submit", function(event) {
        event.preventDefault();
        
        json = {
            title : form.elements['title'].value.trim(),
            sortTitle : form.elements['sort-title'].value.trim(),
            volNum : form.elements['vnum'].value,
            issNum : form.elements['inum'].value,
            isbn : form.elements['isbn'].value.trim(),
            date : form.elements['date'].value.trim(),
            author : form.elements['author'].value.trim(),
            illistrator : form.elements['illistrator'].value.trim(),
            publisher : form.elements['publisher'].value.trim(),
            genere : form.elements['genere'].value.trim(),
            type : form.elements['type'].value,
            collection : form.elements['collection'].value.trim(),
            file : form.elements['file'].value
        }
        
        // handle validation -> sql compramising characters
        // on error alert and cancel or correct an log

        console.log(json);

        if (importItem(json)) {
            form.remove();
        }
    });
    form.getElementsByClassName("api-search")[0].addEventListener("click", async function() {
        matchGoogleAPI(
            form.elements['isbn'].value.trim(),
            form.elements['title'].value.trim(),
            form.elements['author'].value.trim(),
            form.elements['publisher'].value.trim(),
            form);
    });
}
