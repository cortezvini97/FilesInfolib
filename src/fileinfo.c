#include "fileinfo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <libgen.h>  // Para basename
#include <linux/limits.h>
#include <cJSON.h>


int directory_exists(const char *path) {
    struct stat stats;
    if (stat(path, &stats) == 0 && S_ISDIR(stats.st_mode)) {
        return 1;
    }
    return 0;
}

int file_exists(const char *filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}


void get_files_folder_info(const char *folder, void *files_info, const char *base_path) {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    char file_path[PATH_MAX];
    char relative_path[PATH_MAX];

    if (!(dir = opendir(folder))) return;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        snprintf(file_path, sizeof(file_path), "%s/%s", folder, entry->d_name);
        stat(file_path, &file_stat);

        realpath(file_path, relative_path);
        //snprintf(relative_path, sizeof(relative_path), "%s", relative_path + strlen(base_path) + 1);
        snprintf(relative_path, sizeof(relative_path), "%s", file_path + strlen(base_path) + 1);

        cJSON *info = cJSON_CreateObject();
        cJSON_AddStringToObject(info, "name", entry->d_name);
        cJSON_AddStringToObject(info, "path", relative_path);
        cJSON_AddNumberToObject(info, "size", (double)file_stat.st_size);
        cJSON_AddNumberToObject(info, "mtime", (double)file_stat.st_mtime);
        cJSON_AddNumberToObject(info, "permissions", file_stat.st_mode);

        if (S_ISDIR(file_stat.st_mode)) {
            cJSON_AddStringToObject(info, "type", "folder");
            cJSON_AddItemToArray((cJSON*)files_info, info);
            get_files_folder_info(file_path, files_info, base_path);
        } else if (S_ISREG(file_stat.st_mode)) {
            cJSON_AddStringToObject(info, "type", "file");
            cJSON_AddItemToArray((cJSON*)files_info, info);
        }
    }
    closedir(dir);
}

off_t get_folder_size(const char *folder) {
    off_t total_size = 0;
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    char file_path[PATH_MAX];

    if (!(dir = opendir(folder))) return 0;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        snprintf(file_path, sizeof(file_path), "%s/%s", folder, entry->d_name);
        stat(file_path, &file_stat);

        if (S_ISDIR(file_stat.st_mode)) {
            total_size += get_folder_size(file_path);
        } else if (S_ISREG(file_stat.st_mode)) {
            total_size += file_stat.st_size;
        }
    }
    closedir(dir);
    return total_size;
}

void *create_files_info(const char *path_or_file, const char *base_path) {
    struct stat file_stat;
    char relative_path[PATH_MAX];
    cJSON *info = cJSON_CreateObject();

    stat(path_or_file, &file_stat);
    
    snprintf(relative_path, sizeof(relative_path), "%s", path_or_file + strlen(base_path) + 1);

    if (S_ISDIR(file_stat.st_mode)) {
        cJSON_AddStringToObject(info, "name", strrchr(path_or_file, '/') + 1);
        cJSON_AddStringToObject(info, "type", "root_folder");
        cJSON_AddStringToObject(info, "path", relative_path);
        cJSON_AddNumberToObject(info, "size", (double)get_folder_size(path_or_file));
        cJSON_AddNumberToObject(info, "mtime", (double)file_stat.st_mtime);
        cJSON_AddNumberToObject(info, "permissions", file_stat.st_mode);
    } else if (S_ISREG(file_stat.st_mode)) {
        cJSON_AddStringToObject(info, "name", strrchr(path_or_file, '/') + 1);
        cJSON_AddStringToObject(info, "type", "file");
        cJSON_AddStringToObject(info, "path", relative_path);
        cJSON_AddNumberToObject(info, "size", (double)file_stat.st_size);
        cJSON_AddNumberToObject(info, "mtime", (double)file_stat.st_mtime);
        cJSON_AddNumberToObject(info, "permissions", file_stat.st_mode);
    }

    return info;
}

void *inner_file_infos(void *files_infos, const char *base_dir) {
    cJSON *new_file_infos = cJSON_CreateArray();
    cJSON *file_info;
    cJSON_ArrayForEach(file_info, (cJSON*)files_infos) {
        const char *type = cJSON_GetObjectItem(file_info, "type")->valuestring;
        if (strcmp(type, "file") == 0) {
            cJSON_AddItemToArray(new_file_infos, cJSON_Duplicate(file_info, 1));
        } else if (strcmp(type, "root_folder") == 0 || strcmp(type, "folder") == 0) {
            cJSON_AddItemToArray(new_file_infos, cJSON_Duplicate(file_info, 1));
            char dir_file[PATH_MAX];
            snprintf(dir_file, sizeof(dir_file), "%s/%s", base_dir, cJSON_GetObjectItem(file_info, "path")->valuestring);
            get_files_folder_info(dir_file, new_file_infos, base_dir);
        }
    }
    return new_file_infos;
}
