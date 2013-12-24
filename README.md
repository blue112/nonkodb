nonkodb
=======

MongoDB C driver wrapper from NekoVM
Feel free to fork, and send me pull-request if you wish your work to me merged in this repository.

Current status
=============

Currently, it only supports :
* Connect to a Mongo database
* Query with limit and parameters
* Insert
* Insert in batch

Depends
=======

You will need mongo C driver installed and copied into include path.
You can find mongo C driver here : https://github.com/mongodb/mongo-c-driver
You will also need neko.h and libneko. You can have them by installing neko.

Compilation
===========

Currently I only made an Makefile for Linux.
Contributions are welcome to compile DLL for Windows / MacOS.

To compile for linux, just open a terminal in the root folder, and use "make" command.
NDLL will be placed in the bin/ folder.

Usage
=====

This driver is made to be as closed as the C driver as possible.
You will need to use MongoDB.hx and Bson.hx files included in the src/ directory.

Connect
-------

        var db = new MongoDB("127.0.0.1", 27017);

Auth connection aren't supported yet by this wrapper.

Query
-----

    var query = new Bson();
    query.set("index"+level, index);
    query.set("random", {'$gte' : r});
    query.finish();
    var values:Array<Dynamic> = db.queryAll('dbtest.test', query);

Insert
------

    var b = new Bson();
    b.set("word", "hello");
    b.set("random", Math.random());
    b.finish();
    db.insert('dbtest.test', b);

Licence
=======

This wrapper is released under the GPL V3 licence. For more information, see the LICENCE file included.
