async function myFetch(data) {
    try {
        let response = await fetch('/rpc', {
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
        if (json.error !== null) {
            console.log(json);
            throw new Error(`RPC error: ${json.error}`);
        }
        
        document.getElementById("pages").innerHTML = json.result;
                
        console.log(json);
    } catch(e) {
        console.error(e.message);
    }
}