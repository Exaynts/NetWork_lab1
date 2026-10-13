#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_PORT 9999

// Вспомогательная функция для вывода открытых дескрипторов
void print_open_fds(const char* stage)
{
    printf("\n--- Открытые дескрипторы (%s) ---\n", stage);
    DIR *dir = opendir("/proc/self/fd");
    if (!dir) {
        perror("opendir");
        return;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] != '.') {
            printf("%s ", entry->d_name);
        }
    }
    printf("\n");
    closedir(dir);
}

int main(void)
{
    print_open_fds("до создания сокета");

    // ШАГ 1: Создание TCP-сокета (SOCK_STREAM)
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }
    printf("\nСокет создан успешно. Файловый дескриптор: %d\n", sockfd);

    print_open_fds("после создания сокета");

    // ШАГ 2: Заполнение структуры адреса
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY; // Слушаем все интерфейсы

    // ШАГ 3: Привязка сокета (bind)
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        return EXIT_FAILURE;
    }
    printf("Сокет успешно привязан к INADDR_ANY:%d\n", SERVER_PORT);

    // ШАГ 4: Демонстрация обработки ошибки EADDRINUSE
    printf("\nПопытка повторной привязки к тому же порту (ожидаем ошибку)...\n");
    int sockfd2 = socket(AF_INET, SOCK_STREAM, 0);
    if (bind(sockfd2, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        if (errno == EADDRINUSE) {
            printf("Ожидаемая ошибка: EADDRINUSE. Порт %d уже используется.\n", SERVER_PORT);
        } else {
            perror("bind (вторая попытка)");
        }
    }
    close(sockfd2);

    // ШАГ 5: Проверка типа сокета через getsockopt (доп. требование варианта 12)
    int type;
    socklen_t len = sizeof(type);
    if (getsockopt(sockfd, SOL_SOCKET, SO_TYPE, &type, &len) == 0) {
        if (type == SOCK_STREAM) {
            printf("Тип сокета подтвержден: SOCK_STREAM (TCP)\n");
        }
    }

    // ШАГ 6: Проверка автовыбора UDP ядром (доп. требование варианта 12)
    printf("\nПроверка автовыбора протокола для SOCK_DGRAM...\n");
    int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sock >= 0) {
        if (getsockopt(udp_sock, SOL_SOCKET, SO_TYPE, &type, &len) == 0) {
            if (type == SOCK_DGRAM) {
                printf("Успех: при SOCK_DGRAM и protocol=0 ядро выбрало UDP.\n");
            }
        }
        close(udp_sock);
    }

    // ШАГ 7: Закрытие основного сокета и проверка освобождения дескриптора
    if (close(sockfd) < 0) {
        perror("close");
        return EXIT_FAILURE;
    }
    printf("\nСокет закрыт. Дескриптор %d освобождён.\n", sockfd);

    print_open_fds("после закрытия сокета");

    return EXIT_SUCCESS;
}
