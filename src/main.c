#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#ifdef _WIN32
#define PATH_LIST_SEP ";"
#else
#define PATH_LIST_SEP ":"
#endif

int find_path(char* cmd, int is_type) {
    char *path_value = strdup(getenv("PATH"));
#ifdef _WIN32
    // Windows executables carry an extension (.exe/.cmd/...), so try the
    // common ones in addition to the bare name.
    static const char *exts[] = {"", ".exe", ".cmd", ".bat", ".com"};
#endif
    // p = strtok(str, sep) parses str and returns the next token after sep
    // if str is NULL, it parses same string
    for (char *p = strtok(path_value, PATH_LIST_SEP); p != NULL; p = strtok(NULL, PATH_LIST_SEP)) {
        // build path of cmd to check
        char full_path[1028];
#ifdef _WIN32
        for (size_t i = 0; i < sizeof(exts) / sizeof(exts[0]); i++) {
            sprintf(full_path, "%s/%s%s", p, cmd, exts[i]);
            if (access(full_path, X_OK) == 0) {
                if (is_type) {
                    printf("%s is %s\n", cmd, full_path);
                }
                return 0;
            }
        }
#else
        sprintf(full_path, "%s/%s", p, cmd);
        if (access(full_path, X_OK) == 0) {
            if (is_type) {
                printf("%s is %s\n", cmd, full_path);
            }
            return 0;
        }
#endif
    }
    return 1;
}

void handle_type(char *cmd) {
    const char *builtins[] = {"echo", "exit", "type", "pwd", "cd"};
    for (size_t i = 0; i < sizeof(builtins) / sizeof(builtins[0]); i++) {
        if (strcmp(cmd, builtins[i]) == 0) {
            printf("%s is a shell builtin\n", cmd);
            return;
        }
    }

    if (find_path(cmd, true) != 0) {
        fprintf(stderr, "%s: not found\n", cmd);
    }

}

int handle_exec(char* line) {
    char full_line[1024];
    strcpy(full_line, line);
    char* cmd = strtok(line, " ");

    if (find_path(cmd, false) == 0) {
        system(full_line);
    } else {
        fprintf(stderr, "%s: command not found\n", cmd);
    }
    return 0;
}

int main(int argc, char *argv[]) {
    // Flush after every printf
    setbuf(stdout, NULL);


    for (;;) {
        printf("$ ");

        char input[100];
        fgets(input, 100, stdin);

        input[strlen(input) - 1] = '\0';

        if (strcmp(input, "exit") == 0) {
            break;
        }
        if (strncmp(input, "echo ", 5) == 0) {
            // print everything after the "echo " using pointer arithmetic
            printf("%s\n", input + 5);
        } else if (strncmp(input, "type ", 5) == 0) {
            handle_type(input + 5);
        } else if (strcmp(input, "pwd") == 0) {
            char pwdbuf[1024];
            if (getcwd(pwdbuf, sizeof pwdbuf) == NULL) {
                perror("getcwd"); // print err meesage
                return 1;
            }
            printf("%s\n", pwdbuf);
        } else if (strncmp(input, "cd ", 3) == 0) {
            char* arg = input + 3;
            if (strcmp(arg, "~") == 0) {
                // repoint arg to home path
                arg = getenv("HOME");
            }
            if (chdir(arg) != 0) {
                // strerror gets the string given errno, perror just prints it out
                fprintf(stderr, "cd: %s: %s\n", arg, strerror(errno));
            }
        }
        else {
            handle_exec(input);
        }

    }

    return 0;
}
