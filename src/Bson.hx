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

import neko.Lib;
import neko.NativeString;

class Bson
{
    var s:Dynamic;

    public function get_c_obj()
    {
        return s;
    }

    public function new()
    {
        s = _create();
    }

    static public function convert(value:Dynamic):Dynamic
    {
        if (Std.is(value, Array))
        {
            for (i in 0...cast(value, Array<Dynamic>).length)
            {
                value[i] = convert(value[i]);
            }
            value = neko.NativeArray.ofArrayCopy(value);
        }
        else if (Std.is(value, String))
        {
            value = NativeString.ofString(value);
        }
        else
        {
            for (i in Reflect.fields(value))
            {
                var field = Reflect.field(value, i);

                Reflect.setField(value, i, convert(field));
            }
        }
        return value;
    }

    public function set(name:String, value:Dynamic)
    {
        value = convert(value);

        _append(s, NativeString.ofString(name), value);
    }

    public function finish()
    {
        _finish(s);
    }

    public function toObj():Dynamic
    {
        return _toDynamic(s);
    }

    public function toString()
    {
        var str = _print(s);
        return NativeString.toString(str);
    }

    static var _create = Lib.load("mongodb", "createBson", 0);
    static var _append = Lib.load("mongodb", "appendBson", 3);
    static var _print = Lib.load("mongodb", "printBson", 1);
    static var _finish = Lib.load("mongodb", "finishBson", 1);
    static var _toDynamic = Lib.load("mongodb", "bsonToDynamic", 1);
}
