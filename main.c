#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#define RED_COLOR    "\x1b[31m" // child process output color
#define GREEN_COLOR  "\x1b[32m" // parent process output color
#define STANDARD_COLOR "\x1b[0m" // Standard color


// Функция для записи всех данных в pipe
// Она является статичной, так как используется только в этом файле, чтобы избежать конфликтов
//  на вход подаётся файловый дескриптор, буфер и количество байт для записи
// В неё инициализируется указатель на буфер и количество оставшихся байт
// В цикле пока остались байты для записи, вызывается write и пишется в pipe
// Если write возвращает -1 и errno равен EINTR, то цикл продолжается
// ERRNO EINTR означает, что выполнение было прервано сигналом и нужно повторить попытку. Errno из библиотеки errno.h
// Уменьшаем количество оставшихся байт и сдвигаем указатель на буфер
// Если всё прошло успешно, возвращается общее количество записанных байт
// Если произошла ошибка, возвращается -1
static ssize_t write_all(int fd, const void *buf, size_t count) {
    const char *p = buf;
    size_t left = count;
    while (left > 0) {
        ssize_t w = write(fd, p, left);
        if (w == -1) {
            if (errno == EINTR) continue; // Выполнение прервано
            return -1; 
        }
        left -= (size_t)w;
        p += w;
    }
    return (ssize_t)count;
}


int main() {
    // Инициализация трёх pipe`ов
    int pipe1[2], pipe12[2], pipe2[2];
    pid_t pid1, pid2;

    // Создание pipe`ов и проверка на ошибки
    if (pipe(pipe1) == -1 || pipe(pipe12) == -1 || pipe(pipe2) == -1) {
        perror("pipe error");
        exit(EXIT_FAILURE);
    }

    // Создание первого дочернего процесса
    pid1 = fork();
    // проверка на ошибки  
    if (pid1 == -1) {
        perror("fork error");
        exit(EXIT_FAILURE);
    }
// child1 process
    if (pid1 == 0){
        // Закрытие неиспользуемых концов pipe`ов
        // child1 читает из pipe1 и пишет в pipe12
        // Ему не нужны pipe1[1], pipe12[0], pipe2[0], pipe2[1]
        close(pipe1[1]);
        close(pipe12[0]);
        close(pipe2[1]);
        close(pipe2[0]);

        // Перенаправление стандартного ввода и вывода
        // dup2 заменяет один файловый дескриптор другим
        // Если происходит ошибка, выводится сообщение и процесс завершается

        if (dup2(pipe1[0], STDIN_FILENO) == -1) { perror("dup2 pipe1[0]"); exit(EXIT_FAILURE); }
        if (dup2(pipe12[1], STDOUT_FILENO) == -1) { perror("dup2 pipe12[1]"); exit(EXIT_FAILURE); }

        // Закрытие оригинальных файловых дескрипторов после перенаправления

        close(pipe1[0]);
        close(pipe12[1]);

        // Замена текущего процесса на выполнение программы child1 с помощью execl
        // Если execl завершается с ошибкой, выводится сообщение и процесс завершается

        execl("./child1", "child1", NULL);
        perror("execl error");
        exit(EXIT_FAILURE);


    }

    // Создание второго дочернего процесса

    pid2 = fork();
    // проверка на ошибки
    if (pid2 == -1) {
        perror("fork error");
        exit(EXIT_FAILURE);
    }
// child2 process pipe12
    // child2 читает из pipe12 и пишет в pipe2
    // Ему не нужны pipe1[0], pipe1[1], pipe12[1], pipe2[0]
    if (pid2 == 0) {
        close(pipe1[1]);
        close(pipe1[0]);
        close(pipe12[1]);
        close(pipe2[0]);

        // Перенаправление стандартного ввода и вывода мы перенаправляем stdin на pipe12[0] и stdout на pipe2[1] (это происходит не моменте взаимодействия детей через pipe12)

        if (dup2(pipe12[0], STDIN_FILENO) == -1) { perror("dup2 pipe12[0]"); exit(EXIT_FAILURE); }
        if (dup2(pipe2[1], STDOUT_FILENO) == -1) { perror("dup2 pipe2[1]"); exit(EXIT_FAILURE); }

        // Закрытие оригинальных файловых дескрипторов после перенаправления

        close(pipe2[1]);
        close(pipe12[0]);

        // Замена текущего процесса на выполнение программы child2 с помощью execl
        execl("./child2", "child2", NULL);
        perror("execl error");
        exit(EXIT_FAILURE);
    }

    else {
        // Родительский процесс
        // Закрытие неиспользуемых концов pipe`ов
        // Ему не нужны pipe1[0], pipe12[0], pipe12[1], pipe2[1]
        close(pipe1[0]);
        close(pipe12[0]);
        close(pipe12[1]);
        close(pipe2[1]);

        // Чтение из стандартного ввода и запись в pipe1
        // Про функцию getline написано в materials.md, а также почему её можно использовать в linux
        // Она выделяет память под буфер сама, поэтому нужно не забыть её освободить
        // В цикле читаем строки из stdin и пишем их в pipe1 с помощью write_all
        // Если write_all возвращает -1, выводим сообщение об ошибке, освобождаем память, закрываем pipe и ждём завершения дочерних процессов
        // После окончания ввода закрываем pipe1[1] и освобождаем память
        char *buffer = NULL;
        size_t capacity = 0;
        ssize_t nread;
        while ((nread = getline(&buffer, &capacity, stdin)) != -1) {
            if (write_all(pipe1[1], buffer, nread) == -1) {
                perror("write_all to pipe1");
                free(buffer);
                close(pipe1[1]);
                close(pipe2[0]);
                waitpid(pid1, NULL, 0);
                waitpid(pid2, NULL, 0);
                exit(EXIT_FAILURE);
            }
        }
 
        close(pipe1[1]);
        free(buffer);

        // Чтение из pipe2 и вывод в стандартный вывод
        // В цикле читаем из pipe2 и пишем в stdout
        // Пишем мы временным буфером по 1024 байта
        // Если fwrite записывает не то количество байт, что прочитали, выводим сообщение об ошибке и выходим из цикла
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

        // Ожидание завершения дочерних процессов
        // Это очень важно, чтобы избежать зомби-процессов, про которые написано в materials.md
        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
    }


    return 0;
}