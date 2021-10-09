#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <string.h>
#include <crypt.h>

int main(int argc, char* argv[]) {
    char password[512];
    char buffer[1024];
    char name[32];
    char full_hash[128];
    char salt[20];
    char* token;
    struct termios old, new;
    FILE* file;
    int uid = getuid();

    printf("ID asking is %d\n", uid);
    if (setuid(0)) exit(1);

    file = fopen("/etc/passwd", "r");
    while (fgets(buffer, sizeof(buffer), file)) {
        token = strtok(buffer, ":");
        memcpy(name, token, sizeof(name));
        token = strtok(NULL, ":");
        token = strtok(NULL, ":");
        if (atoi(token) == uid) break;
        else name[0] = '\0';
    }
    if (name[0] == '\0') {
        exit(2);
    }
    printf("found user name: %s\n", name);
    fclose(file);

    printf("[sushi] password for %s: ", name);

    ioctl(STDIN_FILENO, TCGETS, &old);
    new = old;
    new.c_lflag &= ~(ICANON | ECHO); // clear echo and canonical input
    ioctl(STDIN_FILENO, TCSETS, &new);

    fgets(password, sizeof(password), stdin);
    password[strlen(password) - 1] = '\0';

    ioctl(STDIN_FILENO, TCSETS, &old); // restore
    printf("\n");
    file = fopen("/etc/shadow", "r");
    while (fgets(buffer, sizeof(buffer), file)) {
        token = strtok(buffer, ":");
        if (!strcmp(name, token)) {
            token = strtok(NULL, ":");
            memcpy(full_hash, token, sizeof(full_hash));
            //printf("found hash for user %s\n", name);
        }
    }
    memcpy(salt, full_hash, 20);
    if ((strcmp(full_hash, crypt(password, salt))))
    {
        printf("WRONG PASSWORD\n");
        exit(1);
    }
    
    char** args = malloc(sizeof(char*)*argc+3); // the more the merrier :)
    args[0] = "/usr/bin/env";
    args[1] = "--";

    for (int i = 0; i < argc; i++) {
        args[i+2] = argv[i+1];
    }
    execv("/usr/bin/env", args);
}