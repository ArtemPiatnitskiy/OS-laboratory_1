#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define RED_COLOR    "\x1b[31m" // child process output color
#define GREEN_COLOR  "\x1b[32m" // parent process output color




int main() {
    int pipe1[2], pipe12[2], pipe2[2];
    pid_t pid1, pid2;

    if (pipe(pipe1) == -1 || pipe(pipe12) == -1 || pipe(pipe2) == -1) {
        perror("pipe error");
        exit(EXIT_FAILURE);
    }

    
    pid1 = fork();

    if (pid1 == -1) {
        perror("fork error");
        exit(EXIT_FAILURE);
    }
// child1 process
    if (pid1 == 0){
        close(pipe1[1]);
        close(pipe12[0]);
        close(pipe2[1]);
        close(pipe2[0]);

        dup2(pipe1[0], STDIN_FILENO);
        dup2(pipe12[1], STDOUT_FILENO);

        close(pipe1[0]);
        close(pipe12[1]);

        execl("./child1", "child1", NULL);
        perror("execl error");
        exit(EXIT_FAILURE);


    }

    pid2 = fork();

    if (pid2 == -1) {
        perror("fork error");
        exit(EXIT_FAILURE);
    }
// child2 process pipe12
    
    if (pid2 == 0) {
        close(pipe1[1]);
        close(pipe1[0]);
        close(pipe12[1]);
        close(pipe2[0]);

        dup2(pipe12[0], STDIN_FILENO);
        dup2(pipe2[1], STDOUT_FILENO);

        close(pipe2[1]);
        close(pipe12[0]);


        execl("./child2", "child2", NULL);
        perror("execl error");
        exit(EXIT_FAILURE);
    }

    else {
        close(pipe1[0]);
        close(pipe12[0]);
        close(pipe12[1]);
        close(pipe2[1]);


        char *buffer = NULL;
        size_t capacity = 0;
        ssize_t nread = getline(&buffer, &capacity, stdin);
        if (nread == -1) {
            perror("getline error");
            free(buffer);
            close(pipe1[1]);
            close(pipe2[0]);
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
            exit(EXIT_FAILURE);
        }

        ssize_t nw = write(pipe1[1], buffer, nread);
        if (nw == -1) perror("write to pipe1");
       
        close(pipe1[1]);
        free(buffer);

        char buf[1024];
        ssize_t r;
        while ((r = read(pipe2[0], buf, sizeof(buf))) > 0) {
            ssize_t written = fwrite(buf, 1, r, stdout);
            if (written != r) {
                perror("fwrite");
                break;
            }
        }
        if (r == -1) perror("read from pipe2");

        close(pipe2[0]);

        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
    }


    return 0;
}