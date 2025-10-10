#include "rwstdio.h"
#include <stdlib.h>
#include <errno.h>

// Чтение всего потока в динамический буфер
char *read_all_fd(int fd, size_t *out_len) {
    size_t cap = 4096, len = 0;
    char *buf = malloc(cap);
    if (!buf) return NULL;
    ssize_t r;
    // Читаем из fd в буфер пока есть данные
    // Если буфер заполняется, увеличиваем его размер вдвое с помощью realloc
    // Аргументы read: файловый дескриптор, указатель на буфер, количество байт для чтения
    while ((r = read(fd, buf + len, cap - len)) > 0) {
        len += (size_t)r;
        // Если буфер заполнен, увеличиваем его размер
        if (cap - len == 0) {
            size_t newcap = cap * 2;
            char *p = realloc(buf, newcap);
            if (!p) { free(buf); return NULL; }
            buf = p;
            cap = newcap;
        }
    }
    // Если произошла ошибка при чтении, освобождаем память и возвращаем NULL
    if (r == -1) { free(buf); return NULL; }
    if (out_len) *out_len = len;
    return buf;
}

// Функция для записи всех данных в pipe
// На вход подаётся файловый дескриптор, буфер и количество байт для записи
// В неё инициализируется указатель на буфер и количество оставшихся байт
// В цикле пока остались байты для записи, вызывается write и пишется в pipe
// Если write возвращает -1 и errno равен EINTR, то цикл продолжается
// ERRNO EINTR означает, что выполнение было прервано сигналом и нужно повторить попытку. Errno из библиотеки errno.h
// Уменьшаем количество оставшихся байт и сдвигаем указатель на буфер
// Если всё прошло успешно, возвращается общее количество записанных байт
// Если произошла ошибка, возвращается -1
int write_all_fd(int fd, const char *buf, size_t len) {
    // Записываем данные в поток, пока не будет записано все
    // Если write возвращает -1 и errno равен EINTR, продолжаем попытку записи
    size_t left = len;
    const char *p = buf;
    while (left > 0) {
        ssize_t w = write(fd, p, left);
        if (w == -1) {
            if (errno == EINTR) continue;
            return -1;
        }
        // Обновляем количество оставшихся байт и сдвигаем указатель
        left -= (size_t)w;
        p += w;
    }
    return 0;
}