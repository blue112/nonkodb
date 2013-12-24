//
// Copyright 2013 Blue112
//
// This file is part of NonkoDB.
//
// NonkoDB is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// NonkoDB is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with NonkoDB.  If not, see <http://www.gnu.org/licenses/>.
//

package;

import neko.NativeString;

class MongoDB
{
    var s : Dynamic;

    public function new(ip:String, port:Int)
    {
        s = _init();
        _connect(s, NativeString.ofString(ip), port);
    }

    public function queryAll(collection:String, ?query:Bson, ?limit:Int):Array<Dynamic>
    {
        if (query == null)
            return neko.NativeArray.toArray(_queryAll(s, NativeString.ofString(collection), null, limit));
        else
            return neko.NativeArray.toArray(_queryAll(s, NativeString.ofString(collection), query.get_c_obj(), limit));
    }

    public function insert(collection:String, bson:Bson):Array<Dynamic>
    {
        return _insertInto(s, NativeString.ofString(collection), bson.get_c_obj());
    }

    public function insertBatch(collection:String, bson:Array<Bson>):Array<Dynamic>
    {
        var out = neko.NativeArray.ofArrayCopy([for (i in bson) i.get_c_obj()]);

        return _insertBatchInto(s, NativeString.ofString(collection), out);
    }

    static var _init = neko.Lib.load("mongodb", "init", 0);
    static var _connect = neko.Lib.load("mongodb", "neko_mongo_connect", 3);
    static var _queryAll = neko.Lib.load("mongodb", "queryAll", 4);
    static var _insertInto = neko.Lib.load("mongodb", "insertInto", 3);
    static var _insertBatchInto = neko.Lib.load("mongodb", "insertBatchInto", 3);
    static var _close = neko.Lib.load("mongodb", "close", 1);
}
