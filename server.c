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

#define MYPORT 3490
#define BACKLOG 10
#define MAXDATASIZE 100
#define FILE_BUFFER_SIZE 1024

void send_file_content(int client_fd, const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("File not found or cannot be opened");
        const char *error_msg = "Error: File not found or cannot be opened.\n";
        send(client_fd, error_msg, strlen(error_msg), 0);
        return;
    }

    char file_buffer[FILE_BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(file_buffer, 1, FILE_BUFFER_SIZE, file)) > 0)
    {
        if (send(client_fd, file_buffer, bytes_read, 0) == -1)
        {
            perror("send");
            fclose(file);
            return;
        }
    }

    fclose(file);
}

void getDirContent(int client_fd)
{
    FILE *fp;
    char buffer[FILE_BUFFER_SIZE];

    fp = popen("ls -1 *.txt", "r");
    if (fp == NULL)
    {
        perror("popen");
        return;
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        if (send(client_fd, buffer, strlen(buffer), 0) == -1)
        {
            perror("send");
            fclose(fp);
            return;
        }
    }
    fclose(fp);
}

void modify_file_content(int client_fd, const char *filename, const char *nv_contenu)
{
    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        perror("File not found or cannot be opened");
        const char *error_msg = "Error: File not found or cannot be opened.\n";
        send(client_fd, error_msg, strlen(error_msg), 0);
        return;
    }

    size_t bytes_written = fwrite(nv_contenu, 1, strlen(nv_contenu), file);
    if (bytes_written < strlen(nv_contenu))
    {
        perror("Error writing to file");
        const char *write_error_msg = "Error: Could not write to the file.\n";
        send(client_fd, write_error_msg, strlen(write_error_msg), 0);
        fclose(file);
        return;
    }

    const char *success_msg = "File content modified successfully.\n";
    send(client_fd, success_msg, strlen(success_msg), 0);

    fclose(file);
}

void delete_file(int client_fd, const char *filename)
{
    if (remove(filename) == 0)
    {
        const char *success_msg = "File successfully deleted.\n";
        send(client_fd, success_msg, strlen(success_msg), 0);
        printf("File '%s' deleted successfully.\n", filename);
    }
    else
    {
        perror("Error deleting file");
        const char *error_msg = "Error: Could not delete the file. It may not exist.\n";
        send(client_fd, error_msg, strlen(error_msg), 0);
    }
}

void creer_file(int client_fd, const char *filename, const char *nv_contenu)
{
    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        perror("File not found or cannot be opened");
        const char *error_msg = "Error: File not found or cannot be opened.\n";
        send(client_fd, error_msg, strlen(error_msg), 0);
        return;
    }

    size_t bytes_written = fwrite(nv_contenu, 1, strlen(nv_contenu), file);
    if (bytes_written < strlen(nv_contenu))
    {
        perror("Erreur de transfert de donnes au fichier");
        const char *write_error_msg = "Erreur: Sauvegarde de fichier non reussi\n";
        send(client_fd, write_error_msg, strlen(write_error_msg), 0);
        fclose(file);
        return;
    }

    const char *success_msg = "File created and content saved successfully.\n";
    send(client_fd, success_msg, strlen(success_msg), 0);

    fclose(file);
}

int main()
{
    int sockfd, new_fd;
    struct sockaddr_in my_addr;
    struct sockaddr_in their_addr;
    socklen_t sin_size;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(MYPORT);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(my_addr.sin_zero), 8);

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("bind");
        close(sockfd);
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1)
    {
        perror("listen");
        close(sockfd);
        exit(1);
    }

    while (1)
    {
        int numbytes;
        char buf[MAXDATASIZE];
        sin_size = sizeof(struct sockaddr_in);

        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1)
        {
            perror("accept");
            continue;
        }

        printf("serveur: Reçu connection de %s\n", inet_ntoa(their_addr.sin_addr));

        int choice;
        if (recv(new_fd, &choice, sizeof(choice), 0) <= 0)
        {
            perror("recv");
            close(new_fd);
            continue;
        }

        switch (choice)
        {
        case 0:
            printf("Liste des fichiers demandée\n");
            getDirContent(new_fd);
            break;
        case 1:
            if ((numbytes = recv(new_fd, buf, MAXDATASIZE - 1, 0)) <= 0)
            {
                perror("recv");
            }
            else
            {
                buf[numbytes] = '\0';
                send_file_content(new_fd, buf);
            }
            break;
        case 2:
        {
            char filename[MAXDATASIZE], new_content[MAXDATASIZE];
            recv(new_fd, filename, MAXDATASIZE - 1, 0);
            recv(new_fd, new_content, MAXDATASIZE - 1, 0);
            modify_file_content(new_fd, filename, new_content);
            break;
        }
        case 3:
        {
            char filename_del[MAXDATASIZE];
            recv(new_fd, filename_del, MAXDATASIZE - 1, 0);
            delete_file(new_fd, filename_del);
            break;
        }
        case 4:
        {
            char filename_cr[MAXDATASIZE], new_content_cr[MAXDATASIZE];
            recv(new_fd, filename_cr, MAXDATASIZE - 1, 0);
            recv(new_fd, new_content_cr, MAXDATASIZE - 1, 0);
            creer_file(new_fd, filename_cr, new_content_cr);
            break;
        }
        default:
            printf("Choix invalide reçu\n");
            break;
        }

        close(new_fd);
    }

    close(sockfd);
    return 0;
}
