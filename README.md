# The Nabu Project

## Intro

The goal of this project is to make a self hosted eReader for reading pdf, epub, and cbz files. The goal is to make this eReader run in a docker container on a linux server so that you can read all your books anyware where you have internet access. The backend will be on C++ with the frontend on JavaScript. I want to use an API and database to retrieve and store metadata for all of the books and collections.

## Build Instructions

Pull the docker image from `tacticalminky/nabu`

Run:

    docker run -d \
        -p 8080:8080 \
        --name=<container name>
        -v <path for appdata>:/appdata \
        -v <path for media>:/media \
        -v <path for imports>:/imports \
        tacticalminky/nabu

Then you can connect to the container at `http://localhost:8080`

The default user is root with the password admin. The ability to add users will be added at a later time.
