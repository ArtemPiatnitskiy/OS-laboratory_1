#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     // STDIN_FILENO, STDOUT_FILENO
#include <errno.h>
#include "string_to_lowercase.h"
#include "rwstdio.h"

int main(void) {
    char *buf = NULL;
    size_t len = 0;

    /* Читаем весь stdin в динамический буфер */
    buf = read_all_fd(STDIN_FILENO, &len);
    if (buf == NULL) {
        perror("Ошибка чтения из stdin");
        return 1;
    }

    /* read_all_fd гарантирует, что после чтения в буфере есть дополнительное место
       (реалокация внутри функции делает это), но на всякий случай делаем буфер
       нуль-терминированным для удобства обработки строк. */
    buf[len] = '\0';

    /* Проходим по буферу и обрабатываем по-строчно */
    size_t start = 0;
    while (start < len) {
        size_t end = start;
        /* ищем конец строки или конец буфера */
        while (end < len && buf[end] != '\n') end++;

        /* временно нуль-терминируем строку для string_to_lowercase */
        char saved = buf[end]; /* saved == '\n' или '\0' (если конец без '\n') */
        buf[end] = '\0';

        /* Преобразуем в нижний регистр (функция изменяет строку in-place) */
        string_to_lowercase(buf + start);

        /* Восстанавливаем символ и вычисляем длину куска для записи */
        buf[end] = saved;
        size_t chunk_len = end - start + (saved == '\n' ? 1 : 0);

        if (chunk_len > 0) {
            if (write_all_fd(STDOUT_FILENO, buf + start, chunk_len) != 0) {
                perror("Ошибка записи в stdout");
                free(buf);
                return 1;
            }
        }

        start = end + (saved == '\n' ? 1 : 0);
    }

    free(buf);
    return 0;
}