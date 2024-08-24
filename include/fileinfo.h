#ifndef FILEINFO_H
#define FILEINFO_H

#include <sys/types.h>
#include <time.h>

int directory_exists(const char *path);
int file_exists(const char *filename);
void get_files_folder_info(const char *folder, void *files_info, const char *base_path);
off_t get_folder_size(const char *folder);
void *create_files_info(const char *path_or_file, const char *base_path);
void *inner_file_infos(void *files_infos, const char *base_dir);


#endif // FILEINFO_H