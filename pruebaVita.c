#include <stdio.h>
#include <string.h>

void remove_last_element(char *filename) {
    if (filename == NULL || strlen(filename) == 0) {
        return;
    }

    // Find the length of the string
    size_t len = strlen(filename);

    // Check if the string ends with a '/'
    if (filename[len - 1] == '/') {
        // Remove trailing '/'
        filename[len - 1] = '\0';
        len--;
    }

    // Find the last '/' in the string
    for (size_t i = len - 1; i > 0; i--) {
        if (filename[i] == '/') {
            // If the only slash is at the beginning, keep it
            if (i == 0) {
                filename[1] = '\0';
            } else {
                filename[i + 1] = '\0';
            }
            return;
        }
    }

    // If no '/' is found, make the filename "/"
    strcpy(filename, "/");
}

int main() {
    char path1[] = "/home/user/docs/folder/";
    char path2[] = "/dir2/";
    char path3[] = "/";

    printf("Original path1: %s\n", path1);
    remove_last_element(path1);
    printf("Modified path1: %s\n", path1);

    printf("Original path2: %s\n", path2);
    remove_last_element(path2);
    printf("Modified path2: %s\n", path2);

    printf("Original path3: %s\n", path3);
    remove_last_element(path3);
    printf("Modified path3: %s\n", path3);

    return 0;
}