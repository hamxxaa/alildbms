#include "table.h"
#include "file_io.h"
#include "fnv_hash.h"
#include "hashmap.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
    {
        Column columns[4] = {
            {"id", INT, 0},
            {"name", STRING, 50},
            {"age", INT, 0},
            {"email", STRING, 50}};
        Column primary_key = {"id", INT, 0};

        Table *user_table = create_table("users", columns, 4, primary_key);
        if (!user_table)
        {
            printf("Failed to create table\n");
            return 1;
        }

        if (insert_record(user_table, 1, "Alice", 25, "mailll") != 0)
        {
            printf("Failed to insert record 1\n");
        }
        if (insert_record(user_table, 2, "Bob", 30, "maill") != 0)
        {
            printf("Failed to insert record 2\n");
        }
        if (insert_record(user_table, 3, "Charlie", 35, "ma") != 0)
        {
            printf("Failed to insert record 3\n");
        }

        // Update record

        char *record = search_record_by_key(user_table, 1);
        if (record)
        {
            printf("Found record:\n");
            print_row_readable(user_table, record);
            free(record);
        }
        else
        {
            printf("Record not found\n");
        }

        // Update record
        if (update_record(user_table, columns[3], 1, "updated_mail") != 0)
        {
            printf("Failed to update record\n");
        }
        char *record1 = search_record_by_key(user_table, 1);
        if (record1)
        {
            printf("Found record:\n");
            print_row_readable(user_table, record1);
            free(record1);
        }

        // Delete record
        if (delete_record(user_table, 1) != 0)
        {
            printf("Failed to delete record\n");
        }
        char *record2 = search_record_by_key(user_table, 1);
        if (record2)
        {
            printf("Found record:\n");
            print_row_readable(user_table, record2);
            free(record2);
        }
        else
        {
            printf("Record not found\n");
        }
    }

    // {
    //     Table *user_table = read_table_metadata("users");
    //     if (!user_table)
    //     {
    //         printf("Failed to read table metadata\n");
    //         return 1;
    //     }

    //     // search records
    //     char *record = search_record_by_key(user_table, 1333);
    //     if (record)
    //     {
    //         printf("Found record:\n");
    //         print_row_readable(*user_table, record);
    //         free(record);
    //     }
    //     else
    //     {
    //         printf("Record not found\n");
    //     }

    // for (int i = 4; i < 5000; i++)
    // {
    //     if (insert_record(user_table, i, "Alice", 25, "mailll") != 0)
    //     {
    //         printf("Failed to insert record %d\n", i);
    //     }
    // }
    // }

    return 0;
}
