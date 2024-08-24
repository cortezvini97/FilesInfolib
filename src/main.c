#include <stdio.h>
#include "fileinfo.h"

int main(){
    char *folder = "./teste";
    long long size = get_folder_size(folder);
    printf("Tamanho do diretório %s %lld Bytes\n", folder, size);
    return 0;
}