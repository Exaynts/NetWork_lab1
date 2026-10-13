#include <stdio.h> // printf, perror, fprintf
#include <stdlib.h> // exit, EXIT_FAILURE, EXIT_SUCCESS
#include <string.h> // memset
#include <unistd.h> // close
#include <errno.h> // errno
#include <sys/socket.h> // socket, bind
#include <netinet/in.h> // sockaddr_in, in_addr, INADDR_ANY
#include <arpa/inet.h> // htons, htonl, inet_pton, inet_ntop
/* Константы для настройки сокета */
#define SERVER_IP "127.0.0.1" /* IP-адрес для привязки */
#define SERVER_PORT 8080 /* Порт для привязки */
int main(void)
{
    /* ============================================================
* ШАГ 1: Создание сокета
* ============================================================
*/
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        return EXIT_FAILURE;
    }
    printf("Сокет создан успешно. Файловый дескриптор: %d\n", sockfd);
    /* ============================================================
* ШАГ 2: Заполнение структуры адреса
* ============================================================
*/
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0)
    {
        if (errno == 0)
        {
            fprintf(stderr, "Ошибка: неверный формат IP-адреса '%s'\n",
                    SERVER_IP);
        }
        else
        {
            perror("inet_pton");
        }
        close(sockfd);
        return EXIT_FAILURE;
    }
    printf("Адрес заполнен: %s:%d\n", SERVER_IP, SERVER_PORT);
    /* ============================================================
* ШАГ 3: Вывод адреса в текстовом виде
* ============================================================
*/
    char ip_str[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &server_addr.sin_addr, ip_str, sizeof(ip_str)) ==
        NULL)
    {
        perror("inet_ntop");
        close(sockfd);
        return EXIT_FAILURE;
    }
    printf("Проверка адреса: %s:%d\n", ip_str, ntohs(server_addr.sin_port));
    /* ============================================================
* ШАГ 4: Привязка сокета к адресу (bind)
* ============================================================
*/
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        close(sockfd);
        return EXIT_FAILURE;
    }
    printf("Сокет успешно привязан к %s:%d\n", SERVER_IP, SERVER_PORT);
    /* ============================================================
* ШАГ 5: Вывод информации о сокете
* ============================================================
*/
    printf("\n--- Информация о сокете ---\n");printf("Файловый дескриптор: %d\n", sockfd);
    printf("Семейство адресов: AF_INET (%d)\n", server_addr.sin_family);
    printf("Порт (сетевой): 0x%04X\n", ntohs(server_addr.sin_port));
    printf("IP-адрес: %s\n", ip_str);
    /* ============================================================
* ШАГ 6: Закрытие сокета
* ============================================================
*/
    if (close(sockfd) < 0)
    {
        perror("close");
        return EXIT_FAILURE;
    }
    printf("\nСокет закрыт. Дескриптор %d освобождён.\n", sockfd);
    return EXIT_SUCCESS;
}
