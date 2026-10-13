#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "192.168.0.100"
#define SERVER_PORT 5353

int main(void)
{
    // ШАГ 1: Создание сокетаS
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }
    printf("Сокет создан успешно. Файловый дескриптор: %d\n", sockfd);

    // ШАГ 2: Заполнение структуры адреса
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr)); // Обнуление структуры
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);    // Преобразование порта

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        if (errno == 0) {
            fprintf(stderr, "Ошибка: неверный формат IP-адреса '%s'\n", SERVER_IP);
        } else {
            perror("inet_pton");
        }
        close(sockfd);
        return EXIT_FAILURE;
    }
    printf("Адрес заполнен: %s:%d\n", SERVER_IP, SERVER_PORT);

    // ШАГ 3: Обратное преобразование адреса для проверки
    char ip_str[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &server_addr.sin_addr, ip_str, sizeof(ip_str)) == NULL) {
        perror("inet_ntop");
        close(sockfd);
        return EXIT_FAILURE;
    }
    printf("Проверка адреса: %s:%d\n", ip_str, ntohs(server_addr.sin_port));

    // ШАГ 4: Привязка сокета к адресу
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        return EXIT_FAILURE;
    }
    printf("Сокет успешно привязан к %s:%d\n", SERVER_IP, SERVER_PORT);

    // ШАГ 5: Вывод информации о сокете
    printf("\n--- Информация о сокете ---\n");
    printf("Файловый дескриптор: %d\n", sockfd);
    printf("Семейство адресов: AF_INET (%d)\n", server_addr.sin_family);
    printf("Порт (сетевой): 0x%04X\n", ntohs(server_addr.sin_port));
    printf("IP-адрес: %s\n", ip_str);

    // ШАГ 6: Закрытие сокета
    if (close(sockfd) < 0) {
        perror("close");
        return EXIT_FAILURE;
    }
    printf("\nСокет закрыт. Дескриптор %d освобождён.\n", sockfd);

    return EXIT_SUCCESS;
}
