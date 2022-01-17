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
             + ((publisher) ? `+:${publisher}` : "" )
             + `&orderBy=relevance`);

        if (!response.ok) {
            throw new Error(`HTTP error: ${response.status}`);
        }

        let json = await response.json();
        console.log(json);

        document.getElementById("api-modal").innerHTML = "";
        document.getElementById("api-modal").style.visibility = "visible";
        for (const match of json.items) {
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
            document.getElementById(match.id).addEventListener("click", function() {
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
                                <td class="grab-btn"><button type="button" onclick="newValue(${form.elements['title']},${match.volumeInfo.title})">Grab New</button></td>
                            </tr>
                            <tr>
                                <th>ISBN:</th>
                                <td>${form.elements['isbn'].value}</td>
                                <td>${((match.volumeInfo.industryIdentifiers) ? match.volumeInfo.industryIdentifiers[0].identifier : "" )}</td>
                                <td class="grab-btn"><button type="button" onclick="newValue(${form.elements['isbn']},${match.volumeInfo.industryIdentifiers[0].identifier})">Grab New</button></td>
                            </tr>
                            <tr>
                                <th>Date:</th>
                                <td>${form.elements['date'].value}</td>
                                <td>${match.volumeInfo.publishedDate}</td>
                                <td class="grab-btn"><button type="button" onclick="newValue(${form.elements['date']},${match.volumeInfo.publishedDate})">Grab New</button></td>
                            </tr>
                            <tr>
                                <th>Author:</th>
                                <td>${form.elements['author'].value}</td>
                                <td>${((match.volumeInfo.authors) ? match.volumeInfo.authors[0] : "" )}</td>
                                <td class="grab-btn"><button type="button" onclick="newValue(${form.elements['author']},${((match.volumeInfo.authors) ? match.volumeInfo.authors[0] : "" )})">Grab New</button></td>
                            </tr>
                            <tr>
                                <th>Illistrator:</th>
                                <td>${form.elements['illistrator'].value}</td>
                                <td></td>
                                <td class="grab-btn"><button type="button" onclick="newValue(${form.elements['illistrator']}, null)">Grab New</button></td>
                            </tr>
                            <tr>
                                <th>Publisher:</th>
                                <td>${form.elements['publisher'].value}</td>
                                <td>${match.volumeInfo.publisher}</td>
                                <td class="grab-btn"><button type="button" onclick="newValue(${form.elements['publisher']},${match.volumeInfo.publisher})">Grab New</button></td>
                            </tr>
                            <tr> 
                                <th>Generes:</th>
                                <td>${form.elements['genere'].value}</td>
                                <td>${((match.volumeInfo.categories) ? match.volumeInfo.categories[0] : "" )}</td>
                                <td class="grab-btn"><button type="button" onclick="newValue(${form.elements['genere']},${((match.volumeInfo.categories) ? match.volumeInfo.categories[0] : "" )})">Grab New</button></td>
                            </tr>
                            <tr>
                                <th>Collection:</th>
                                <td>${form.elements['collection'].value}</td>
                                <td>${((match.volumeInfo.seriesInfo) ? match.volumeInfo.seriesInfo.title : "" )}</td>
                                <td class="grab-btn"><button type="button" onclick="newValue(${form.elements['collection']},${((match.volumeInfo.seriesInfo) ? match.volumeInfo.seriesInfo.title : "" )})">Grab New</button></td>
                            </tr>
                            <tr>
                                <th>Volume:</th>
                                <td>${form.elements['vnum'].value}</td>
                                <td>${((match.volumeInfo.seriesInfo) ? match.volumeInfo.seriesInfo.bookDisplayNumber : "" )}</td>
                                <td class="grab-btn"><button type="button" onclick="newValue(${form.elements['vnum']},${((match.volumeInfo.seriesInfo) ? match.volumeInfo.seriesInfo.bookDisplayNumber : "" )})">Grab New</button></td>
                            </tr>
                            <tr>
                                <th>Issue:</th>
                                <td>${form.elements['inum'].value}</td>
                                <td>${((match.volumeInfo.seriesInfo) ? match.volumeInfo.seriesInfo.bookDisplayNumber : "" )}</td>
                                <td class="grab-btn"><button type="button" onclick="newValue(${form.elements['inum']},${((match.volumeInfo.seriesInfo) ? match.volumeInfo.seriesInfo.bookDisplayNumber : "" )})">Grab New</button></td>
                            </tr>
                            <tr>
                                <td colspan="2"><button type="button" onclick="closeAPIModal()">Exit</button></td>
                            </tr>
                        </table>
                    </div>
                `;
            });
        }
        // fetch more
    } catch(e) {
        console.error(e.message);
    }
} // searchGoogleAPI()

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
