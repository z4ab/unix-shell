#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#ifdef _WIN32
#define PATH_LIST_SEP ";"
#else
#include <sys/wait.h>
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

int handle_exec(char* args[]) {
    if (find_path(args[0], false) != 0) {
        fprintf(stderr, "%s: command not found\n", args[0]);
        return -1;
    }

    pid_t pid = fork();
    if (pid == 0) { // child
        execvp(args[0], args);
        // if exec fails,
        perror(args[0]);
        _exit(127);
    } else if (pid > 0) { // parent
        int status;
        waitpid(pid, &status, 0);
    } else { // -1, meaning error
        perror("fork");
    }
    return 0;
}

int main(int argc, char *argv[]) {
    // Flush after every printf
    setbuf(stdout, NULL);


    for (;;) {
        printf("$ ");


        // tokenize input into args

        char* args[1024];
        int j = 0;
        int argc = 0;

        char curr_arg[1024];

        bool in_arg = false;
        bool in_single_quote = false;

        int c;
        while ((c = getchar()) != '\n' && c != '\n') {
            // every char enclosed in quotes is in the same argument
            // handle single quotes
            if (c == '\'') {
                in_single_quote = !in_single_quote;
            } else if ((c == ' ' || c == '\t') && !in_single_quote) {
                if (in_arg) {
                    curr_arg[j++] = '\0';
                    in_arg = false;
                }
            } else {
                if (!in_arg) {
                    args[argc++] = curr_arg + j;
                    in_arg = true;
                }
                curr_arg[j++] = c;
            }
        }
        if (in_arg) {
            curr_arg[j++] = '\0';
        }
        args[argc] = NULL;
        if (c == EOF && argc == 0) {
            // EOF signal on empty line, exit shell
            break;
        }

        if (strcmp(args[0], "exit") == 0) {
            break;
        }
        if (strcmp(args[0], "echo") == 0) {
            // print args seperated by space
            for (int k = 1; k < argc - 1; k++) {
                printf("%s", args[k]);
                printf(" ");
            }
            // print last arg
            printf("%s\n", args[argc - 1]);
        } else if (strcmp(args[0], "type") == 0) {
            handle_type(args[1]);
        } else if (strcmp(args[0], "pwd") == 0) {
            char pwdbuf[1024];
            if (getcwd(pwdbuf, sizeof pwdbuf) == NULL) {
                perror("getcwd"); // print err meesage
                return 1;
            }
            printf("%s\n", pwdbuf);
        } else if (strcmp(args[0], "cd") == 0) {
            if (strcmp(args[1], "~") == 0) {
                // repoint arg to home path
                args[1] = getenv("HOME");
            }
            if (chdir(args[1]) != 0) {
                // strerror gets the string given errno, perror just prints it out
                fprintf(stderr, "cd: %s: %s\n", args[1], strerror(errno));
            }
        }
        else {
            handle_exec(args);
        }

    }

    return 0;
}
