#ifndef rwstdio_h
#define rwstdio_h

#include <unistd.h>
#include <stddef.h>
#include <sys/types.h>

/* Чтение всего потока в динамический буфер (возвращает указатель и размер через out_len) */
char *read_all_fd(int fd, size_t *out_len);

// Запись всего буфера в поток (возвращает 0 при успехе, -1 при ошибке)
int write_all_fd(int fd, const char *buf, size_t len);



#endif // rwstdio_h