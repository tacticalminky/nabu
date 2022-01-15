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
    } catch(e) {
        console.error(e.message);
    }
} // importItem()

const forms = document.forms;

for (const form of forms) {
    form.addEventListener("submit", function(event) {
        event.preventDefault();
        
        json = {
            title : form.elements['title'].value,
            sortTitle : form.elements['sort-title'].value,
            volNum : form.elements['vnum'].value,
            issNum : form.elements['inum'].value,
            isbn : form.elements['isbn'].value,
            date : form.elements['date'].value,
            author : form.elements['author'].value,
            illistrator : form.elements['illistrator'].value,
            publisher : form.elements['publisher'].value,
            genere : form.elements['genere'].value,
            type : form.elements['type'].value,
            collection : form.elements['collection'].value,
            file : form.elements['file'].value
        }
        
        // handle validation -> sql compramising characters
        // on error alert and cancel or correct an log

        console.log(json);

        importItem(json);

        form.remove();
    });
}
