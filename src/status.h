#pragma once

#define PEACHOS_ALL_OK 0
#define EIO 1 // IO error (this is how linux does it)
#define EINVARG 2 // invalid argument
#define ENOMEM 3 // out of memory
#define EBADPATH 4 // malformed path
#define EFSNOTUS 5 // disk does not conform to the filesystem that is attempting to resolve to the disk
#define ERDONLY 6 // filesystem is read-only
#define EUNIMP 7 // not implemented
#define EISTKN 8 // is taken
