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

#include <neko.h>
#include <mongo/mongo.h>
#include <stdio.h>

DEFINE_KIND(k_mongo);
DEFINE_KIND(k_bson);

typedef struct s_chained_values
{
    value *val;
    struct s_chained_values *next;
} chained_values;

value init()
{
    mongo *conn = malloc(sizeof(mongo));

    if (!conn)
    {
        fprintf(stderr, "Cannot allocate main mongo struct\n");
        neko_error();
    }

    mongo_init(conn);
    mongo_set_op_timeout(conn, 1000);
    return alloc_abstract(k_mongo, conn);
}

value neko_mongo_connect(value a_conn, value ip, value port)
{
    if(!val_is_abstract(a_conn) || !val_is_kind(a_conn, k_mongo))
        neko_error();

    val_check(ip, string); val_check(port, int);

    mongo *conn = val_data(a_conn);

    int status = mongo_client(conn, val_string(ip), val_int(port));
    return alloc_bool(status == MONGO_OK);
}

void finaliseBson(value a_bson)
{
    if(!val_is_abstract(a_bson) || !val_is_kind(a_bson, k_bson))
        return;

    bson *val = val_data(a_bson);
    bson_destroy(val);
    free(val);
}

value createBson()
{
    bson *val = malloc(sizeof(bson));
    bson_init(val);

    value v = alloc_abstract(k_bson, val);
    val_gc(v,finaliseBson);

    return v;
}

value appendBson(value a_bson, value name, value to_add);

static void appendObjFieldToBson(value v, field f, void* bson)
{
    appendBson(*((value*)bson), val_field_name(f), v);
}

value appendBson(value a_bson, value name, value to_add)
{
    if(!val_is_abstract(a_bson) || !val_is_kind(a_bson, k_bson))
        neko_error();

    val_check(name, string);

    bson *val = val_data(a_bson);
    if (val_is_int(to_add))
        bson_append_int(val, val_string(name), val_int(to_add));
    else if (val_is_float(to_add))
        bson_append_double(val, val_string(name), val_float(to_add));
    else if (val_is_string(to_add))
        bson_append_string(val, val_string(name), val_string(to_add));
    else if (val_is_array(to_add))
    {
        bson_append_start_array(val, val_string(name));
        char c[20];
        for (int i = 0; i < val_array_size(to_add); i++)
        {
            value current_val = val_array_ptr(to_add)[i];
            sprintf(c, "%d", i);
            appendBson(a_bson, alloc_string(c), current_val);
        }
        bson_append_finish_array(val);
    }
    else if (val_is_object(to_add))
    {
        bson_append_start_object(val, val_string(name));
        val_iter_fields(to_add, appendObjFieldToBson, &a_bson);
        bson_append_finish_object(val);
    }

    return val_null;
}

value finishBson(value a_bson)
{
    if(!val_is_abstract(a_bson) || !val_is_kind(a_bson, k_bson))
        neko_error();

    bson_finish(val_data(a_bson));
    return val_null;
}

value printBson(value a_bson)
{
    if(!val_is_abstract(a_bson) || !val_is_kind(a_bson, k_bson))
        neko_error();

    bson_print(val_data(a_bson));
    return val_null;
}

value bsonToDynamic(value a_bson)
{
    if(!val_is_abstract(a_bson) || !val_is_kind(a_bson, k_bson))
        neko_error();

    bson_iterator i[1];
    bson *val = val_data(a_bson);
    bson_type type;
    const char * key;

    bson_iterator_init(i, val);
    value obj = alloc_object(val_null);
    value field_value;

    while ((type = bson_iterator_next(i)))
    {
        key = bson_iterator_key(i);

        if (type == BSON_STRING)
            field_value = alloc_string(bson_iterator_string(i));
        else if (type == BSON_INT)
            field_value = alloc_int(bson_iterator_int_raw(i));
        else if (type == BSON_DOUBLE)
            field_value = alloc_float( (float) bson_iterator_double_raw(i));
        else
            continue;

        alloc_field(obj, val_id(key), field_value);
    }

    return obj;
}

static void _free_chained_values(chained_values* head)
{
    chained_values* current = head;
    while (current)
    {
        chained_values* next = current->next;
        free(current);
        current = next;
    }
}

value insertInto(value a_conn, value collection_name, value a_bson)
{
    if(!val_is_abstract(a_conn) || !val_is_kind(a_conn, k_mongo))
        neko_error();

    if(!val_is_abstract(a_bson) || !val_is_kind(a_bson, k_bson))
        neko_error();

    val_check(collection_name, string);

    mongo *conn = val_data(a_conn);
    bson *doc = val_data(a_bson);

    if (mongo_insert(conn, val_string(collection_name), doc, NULL) == MONGO_OK)
        return val_true;
    else
        return val_false;
}

value insertBatchInto(value a_conn, value collection_name, value a_bson)
{
    if(!val_is_abstract(a_conn) || !val_is_kind(a_conn, k_mongo))
        neko_error();

    val_check(collection_name, string);
    val_check(a_bson, array);

    int i;
    int size = val_array_size(a_bson);
    value* a = val_array_ptr(a_bson);
    const bson** to = malloc(sizeof( bson ) * size);
    mongo *conn = val_data(a_conn);

    for (i = 0; i < size; i++)
    {
        if(!val_is_abstract(a[i]) || !val_is_kind(a[i], k_bson))
            neko_error();

        to[i] = (bson*) val_data(a[i]);
    }

    int ret = mongo_insert_batch(conn, val_string(collection_name), to, size, 0, MONGO_CONTINUE_ON_ERROR);

    free(to);

    if (ret == MONGO_OK)
        return val_true;

    return val_false;
}

value queryAll(value a_conn, value collection_name, value a_query, value limit)
{
    if(!val_is_abstract(a_conn) || !val_is_kind(a_conn, k_mongo))
        neko_error();

    if(!val_is_null(a_query) && (!val_is_abstract(a_query) || !val_is_kind(a_query, k_bson)))
        neko_error();

    val_check(collection_name, string);

    mongo_cursor cursor[1];
    mongo_cursor_init(cursor, val_data(a_conn), val_string(collection_name));

    if (!val_is_null(a_query))
    {
        bson *query = val_data(a_query);
        mongo_cursor_set_query(cursor, query);
    }

    if (!val_is_null(limit))
    {
        mongo_cursor_set_limit(cursor, val_int(limit));
    }

    int size = 0;
    chained_values *head = NULL;
    chained_values *current = NULL;
    chained_values *old_current = NULL;
    while (mongo_cursor_next(cursor) == MONGO_OK)
    {
        old_current = current;
        size++;

        current = calloc(1, sizeof(chained_values));
        if (!head)
        {
            head = current;
        }
        else
        {
            old_current->next = current;
        }

        value v = bsonToDynamic(alloc_abstract(k_bson, &cursor->current));
        value *v_ref = malloc(sizeof(value));
        memcpy(v_ref, &v, sizeof(value));
        current->val = v_ref;
    }

    mongo_cursor_destroy(cursor);

    value array = alloc_array(size);
    value* array_values = val_array_ptr(array);
    current = head;
    for (int i = 0; i < size; i++)
    {
        array_values[i] = *(current->val);
        current = current->next;
    }

    _free_chained_values(head);

    return array;
}

value close(value a_conn)
{
    if(!val_is_abstract(a_conn) || !val_is_kind(a_conn, k_mongo))
        neko_error();

    mongo *conn = val_data(a_conn);
    mongo_destroy(conn);
    free(conn);

    return val_null;
}

DEFINE_PRIM(init, 0);
DEFINE_PRIM(neko_mongo_connect, 3);
DEFINE_PRIM(queryAll, 4);
DEFINE_PRIM(insertInto, 3);
DEFINE_PRIM(insertBatchInto, 3);
DEFINE_PRIM(close, 1);

DEFINE_PRIM(createBson, 0);
DEFINE_PRIM(appendBson, 3);
DEFINE_PRIM(printBson, 1);
DEFINE_PRIM(bsonToDynamic, 1);
DEFINE_PRIM(finishBson, 1);
