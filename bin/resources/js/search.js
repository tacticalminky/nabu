/*
This file is part of Nabu.

Nabu is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Nabu is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License along with Nabu. If not, see <https://www.gnu.org/licenses/>.
*/

function updateSearch(value) {
    for (const item of document.getElementsByClassName("media-item")) {
        if (item.firstElementChild.innerText.toLowerCase().indexOf(value.toLowerCase()) != -1) {
            item.style.display = "flex";
        } else {
            item.style.display = "none";
        }
    }
}

function search() {
    updateSearch(document.getElementById("search-bar").value);
}

