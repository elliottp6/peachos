// https://www.udemy.com/course/developing-a-multithreaded-kernel-from-scratch/learn/lecture/23983772
#pragma once

// types
struct path_root {
    int drive_no;
    struct path_part* first;
};

struct path_part {
    const char* part;
    struct path_part* next;
};

// functions
struct path_root* pathparser_parse( const char* path, const char* current_directory_path );
void pathparser_free( struct path_root* root );
