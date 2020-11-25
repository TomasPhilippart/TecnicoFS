#ifndef FS_H
#define FS_H
#include "state.h"

void unlock_inodes(int *inodes_visited, int num_inodes_visited);
void init_fs();
void destroy_fs();
int is_dir_empty(DirEntry *dirEntries);
int create(char *name, type nodeType);
int delete(char *name);
int move(char *path, char *newPath);
int lookup(char *name, int *inodes_visited, int *num_inodes_visited, int mode);
int print_tecnicofs_tree(char *path);

#endif /* FS_H */
