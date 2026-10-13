#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Вспомогательная функция для вывода открытых дескрипторов (требование среднего уровня)
void print_open_fds(const char* stage)
{
    printf("\n--- Открытые дескрипторы (%s) ---\n", stage);
    DIR *dir = opendir("/proc/self/fd");
    if (!dir) return;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] != '.') printf("%s ", entry->d_name);
    }
    printf("\n");
    closedir(dir);
}

void print_usage(const char* prog_name) {
    fprintf(stderr, "Использование: %s <domain> <type> <ip> <port> [--bind]\n", prog_name);
    fprintf(stderr, "Пример: %s AF_INET SOCK_DGRAM 127.0.0.1 8080 --bind\n", prog_name);
}

int main(int argc, char *argv[])
{
    if (argc < 5 || argc > 6) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    // ШАГ 1: Разбор аргументов командной строки
    int domain = 0, type = 0;
    if (strcmp(argv[1], "AF_INET") == 0) domain = AF_INET;
    else {
        fprintf(stderr, "Ошибка: неподдерживаемый домен '%s'. Используйте AF_INET.\n", argv[1]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[2], "SOCK_STREAM") == 0) type = SOCK_STREAM;
    else if (strcmp(argv[2], "SOCK_DGRAM") == 0) type = SOCK_DGRAM;
    else {
        fprintf(stderr, "Ошибка: неверный тип сокета '%s'.\n", argv[2]);
        return EXIT_FAILURE;
    }

    const char* ip = argv[3];
    int port = atoi(argv[4]);
    int do_bind = (argc == 6 && strcmp(argv[5], "--bind") == 0);

    // ШАГ 2: Проверка граничных случаев
    if (port < 0 || port > 65535) {
        fprintf(stderr, "Ошибка: порт %d вне допустимого диапазона (0-65535).\n", port);
        return EXIT_FAILURE;
    }

    print_open_fds("до создания сокета");

    // ШАГ 3: Создание сокета
    int sockfd = socket(domain, type, 0);
    if (sockfd < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }
    printf("\nСокет создан. Файловый дескриптор: %d\n", sockfd);

    // ШАГ 4: Заполнение структуры адреса
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = domain;
    addr.sin_port = htons(port);

    // Устанавливаем опцию SO_REUSEADDR, чтобы избежать ошибки EADDRINUSE
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(sockfd);
        return EXIT_FAILURE;
    }

    if (inet_pton(domain, ip, &addr.sin_addr) <= 0) {
        if (errno == 0) {
            fprintf(stderr, "Ошибка: неверный формат IP-адреса '%s'\n", ip);
        } else {
            perror("inet_pton");
        }
        close(sockfd);
        return EXIT_FAILURE;
    }
    printf("Адрес заполнен: %s:%d\n", ip, port);

    // ШАГ 5: Привязка сокета (если указан флаг --bind)
    if (do_bind) {
        if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            if (errno == EADDRINUSE) {
                fprintf(stderr, "Ошибка: порт %d уже используется (EADDRINUSE).\n", port);
            } else {
                perror("bind");
            }
            close(sockfd);
            return EXIT_FAILURE;
        }
        printf("Сокет успешно привязан к %s:%d\n", ip, port);
    } else {
        printf("Флаг --bind не указан. Привязка не выполнена.\n");
    }

    // ШАГ 6: Проверка автовыбора UDP (доп. требование варианта 12)
    if (type == SOCK_DGRAM) {
        int actual_type;
        socklen_t len = sizeof(actual_type);
        if (getsockopt(sockfd, SOL_SOCKET, SO_TYPE, &actual_type, &len) == 0) {
            if (actual_type == SOCK_DGRAM) {
                printf("Проверка варианта 12: при SOCK_DGRAM и protocol=0 ядро выбрало UDP.\n");
            }
        }
    }

    // ШАГ 7: Вывод информации и закрытие
    printf("\n--- Информация о сокете ---\n");
    printf("Дескриптор: %d\n", sockfd);
    printf("Домен: %s (%d)\n", argv[1], addr.sin_family);
    printf("Порт (сетевой): 0x%04X\n", ntohs(addr.sin_port));

    if (close(sockfd) < 0) {
        perror("close");
        return EXIT_FAILURE;
    }
    printf("Сокет закрыт. Дескриптор %d освобождён.\n", sockfd);

    print_open_fds("после закрытия сокета");

    return EXIT_SUCCESS;
}
