var isDoubleView, isDoubleViewNow, firstLoaded, lastLoaded, currentPage, pageCount, forwardsChunk, backwardsChunk;

// Takes in the id of a book
async function loadInit(setId) {
    data = {
        method: "loadInit",
        params: [ setId ],
        id: 1
    }
    
    try {
        let response = await fetch('/reading-rpc', {
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

        document.getElementById("pages").innerHTML = json.result.html;

        firstLoaded = json.result.firstLoaded;
        lastLoaded = json.result.lastLoaded;
        currentPage = firstLoaded;
        pageCount = json.result.pageCount;
        isDoubleView = json.result.isDoubleView;
        isDoubleViewNow = isDoubleView;
        forwardsChunk = json.result.forwardsChunk;
        backwardsChunk = json.result.backwardsChunk;
    } catch(e) {
        console.error(e.message);
    }

    if (firstLoaded > 0) {
        await loadBackwards([ currentPage, firstLoaded ]);
    }
    let isStillLoading = false;
    do {
        isStillLoading = false;

        if (firstLoaded > 0 && currentPage < ((isDoubleView) ? backwardsChunk*3 : backwardsChunk*2) + firstLoaded) {
            isStillLoading = true;
            await loadBackwards([ currentPage, firstLoaded ]);
        }
        if (lastLoaded < pageCount && currentPage + ((isDoubleView) ? forwardsChunk*3 : forwardsChunk*2) > lastLoaded) {
            isStillLoading = true;
            await loadForwards([ currentPage, lastLoaded ]);
        }
    } while (isStillLoading)
    
    if (isDoubleView && currentPage % 2 !== 0) { currentPage--; }
    goToCurrentPage();
} // loadInit()

// takes in [ currentPage, lastLoaded ]
async function loadForwards(setParams) {
    data = {
        method: "loadForwards",
        params: setParams,
        id: 2
    }

    try {
        let response = await fetch('/reading-rpc', {
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
        
        document.getElementById("pages").innerHTML += json.result.html;
    
        firstLoaded = json.result.firstLoaded;
        lastLoaded = json.result.lastLoaded;
    } catch(e) {
        console.error(e.message);
    }
} // loadForwards()

// takes in [ currentPage, firstLoaded ]
async function loadBackwards(setParams) {
    if (firstLoaded === 0) {
        console.warn("Cant load backwards anymore");
        return;
    }

    data = {
        method: "loadBackwards",
        params: setParams,
        id: 3
    }

    try {
        let response = await fetch('/reading-rpc', {
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
        
        document.getElementById("pages").innerHTML = json.result.html + document.getElementById("pages").innerHTML;
        
        firstLoaded = json.result.firstLoaded;
        lastLoaded = json.result.lastLoaded;
    } catch(e) {
        console.error(e.message);
    }
} // loadBackwards()

async function loadCheck() {
    if (firstLoaded > 0 && currentPage < ((isDoubleView) ? backwardsChunk*3 : backwardsChunk*2) + firstLoaded) {
        await loadBackwards([ currentPage, firstLoaded ]);
    } else if (lastLoaded < pageCount && currentPage + ((isDoubleView) ? forwardsChunk*3 : forwardsChunk*2) > lastLoaded) {
        await loadForwards([ currentPage, lastLoaded ]);
    }
}

function openModal() {
    document.getElementById("modal").style.display = "block";
}

function closeModal() {
    document.getElementById("modal").style.display = "none";
}

// n should be 1 or -1
function flipPage(n) {
    if (isDoubleView) {
        if (currentPage % 2 !== 0) { currentPage++; }

        if (!(((currentPage + (n * 2)) >= pageCount) && (currentPage + n < pageCount)) || (((currentPage + (n * 2)) < 0) && (currentPage + n >= 0))) {
            n *= 2;
            isDoubleViewNow = true;
        } else {
            isDoubleViewNow = false;
        }
    }
    newPage = currentPage + n;
    if (newPage >= pageCount || newPage < 0) {
        return;
    }
    currentPage = newPage;
    goToCurrentPage();
}

function goToCurrentPage() {
    let pages = document.getElementsByClassName("myPages");
    if (pages.length < pageCount) { loadCheck(); }
    for (let index = 0; index < pages.length; index++) {
        pages[index].style.display = "none";
    }
    console.log("Page: " + currentPage);
    pages[currentPage].style.display = "flex";
    if (isDoubleViewNow) {
        pages[currentPage + 1].style.display = "flex";
    }
}
