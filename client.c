#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 3490
#define MAXDATASIZE 100
#define FILE_BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    int sockfd, numbytes;
    char buf[FILE_BUFFER_SIZE];
    struct hostent *he;
    struct sockaddr_in their_addr;

    if (argc != 2)
    {
        fprintf(stderr, "usage: client hostname\n");
        exit(1);
    }

    if ((he = gethostbyname(argv[1])) == NULL)
    {
        herror("gethostbyname");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket");
        exit(1);
    }

    their_addr.sin_family = AF_INET;
    their_addr.sin_port = htons(PORT);
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    bzero(&(their_addr.sin_zero), 8);

    if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("connect");
        exit(1);
    }

    int choice;
    char filename[MAXDATASIZE];
    char new_content[MAXDATASIZE];

    printf("Choisissez une operation :\n");
    printf("0. Liste des fichiers disponibles\n");
    printf("1. Lire un fichier\n");
    printf("2. Modifier un fichier\n");
    printf("3. Supprimer un fichier\n");
    printf("4. Ajouter un fichier\n");
    printf("Entrez votre choix : ");
    scanf("%d", &choice);

    if (send(sockfd, &choice, sizeof(choice), 0) == -1)
    {
        perror("Erreur d'envoi du choix");
        close(sockfd);
        exit(1);
    }

    switch (choice)
    {
    case 0:
        printf("Liste des fichiers demandée...\n");
        break;
    case 1:
        printf("Entrez le nom du fichier à lire : ");
        scanf("%s", filename);
        send(sockfd, filename, strlen(filename), 0);
        break;
    case 2:
        printf("Entrez le nom du fichier à modifier : ");
        scanf("%s", filename);
        send(sockfd, filename, strlen(filename), 0);
        printf("Entrez le nouveau contenu : ");
        scanf(" %[^\n]s", new_content);
        send(sockfd, new_content, strlen(new_content), 0);
        break;
    case 3:
        printf("Entrez le nom du fichier à supprimer : ");
        scanf("%s", filename);
        send(sockfd, filename, strlen(filename), 0);
        break;
    case 4:
        printf("Entrez le nom du fichier à creer : ");
        scanf("%s", filename);
        send(sockfd, filename, strlen(filename), 0);
        printf("Entrez le nouveau contenu : ");
        scanf(" %[^\n]s", new_content);
        send(sockfd, new_content, strlen(new_content), 0);
        break;
    default:
        printf("Choix invalide\n");
        break;
    }

    printf("Réponse du serveur :\n");
    while ((numbytes = recv(sockfd, buf, FILE_BUFFER_SIZE - 1, 0)) > 0)
    {
        buf[numbytes] = '\0';
        printf("%s", buf);
    }

    close(sockfd);
    return 0;
}
