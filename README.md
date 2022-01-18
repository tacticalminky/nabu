# The Nabu Project

## Intro

The goal of this project is to make a self hosted eReader for reading pdf, epub, and cbz files. The goal is to make this eReader run in a docker container on a linux server so that you can read all your books anyware where you have internet access. The backend will be on C++ with the frontend on JavaScript. I want to use an API and database to retrieve and store metadata for all of the books and collections.

Importing with Google Books API does not work.

## Build Instructions

In order to run the project, SQLite3 will need to be installed to create the database. THe other required source files should be in the include file. Before running the app, a few things will need to be done.

1. Change or create the paths in the config.json file under the app paths location and the file_server alias location to where your directories will be

2. Run the `sqlite3` command in terminal followed by `.open --new PATH/TO/DATABASE/FILE` then `.exit`

3. Edit line 30 in initialize.cpp, where the database is called with the path to the database you just created

4. In the root of the project run `make init` to populate the database

You should now be able to build and run the project with `make run`. You might need to adjust the include paths.
