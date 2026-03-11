#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>

#define BUFFER_SIZE 128



int main()
{
    int pipe1[2];
    int pipe2[2];
    pid_t pid1, pid2;
    char buffer[BUFFER_SIZE];

    if (pipe(pipe1) == -1)
    {
        perror("Pipe1 failed");
        exit(1);
    }

    if (pipe(pipe2) == -1)
    {
        perror("Pipe2 failed");
        exit(1);
    }

    pid1 = fork();

    if (pid1 < 0)
    {
        perror("Fork failed");
        exit(1);
    }

    if (pid1 == 0)
    {
        char clean[BUFFER_SIZE];
        int n;
        int i, j = 0;

        close(pipe1[1]);
        close(pipe2[0]);

        n = read(pipe1[0], buffer, BUFFER_SIZE - 1);
        if (n < 0)
        {
            perror("Read failed");
            exit(1);
        }

        buffer[n] = '\0';

        for (i = 0; buffer[i] != '\0'; i++)
        {
            if (!isdigit((unsigned char)buffer[i]))
            {
                clean[j++] = buffer[i];
            }
        }

        clean[j] = '\0';

        write(pipe2[1], clean, strlen(clean) + 1);

        close(pipe1[0]);
        close(pipe2[1]);
        exit(0);
    }

    pid2 = fork();

    if (pid2 < 0)
    {
        perror("Fork failed");
        exit(1);
    }

    if (pid2 == 0)
    {
        char received[BUFFER_SIZE];
        int n;
        int count = 0;
        int i;

        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[1]);

        n = read(pipe2[0], received, BUFFER_SIZE - 1);
        if (n < 0)
        {
            perror("Read failed");
            exit(1);
        }

        received[n] = '\0';

        for (i = 0; received[i] != '\0'; i++)
        {
            received[i] = toupper((unsigned char)received[i]);
            count++;
        }

        printf("Uppercase String: %s\n", received);
        printf("Character Count: %d\n", count);

        close(pipe2[0]);
        exit(0);
    }

    close(pipe1[0]);
    close(pipe2[0]);
    close(pipe2[1]);

    printf("Enter a string: ");
    fgets(buffer, BUFFER_SIZE, stdin);

    buffer[strcspn(buffer, "\n")] = '\0';

    write(pipe1[1], buffer, strlen(buffer) + 1);

    close(pipe1[1]);

    wait(NULL);
    wait(NULL);

    return 0;
}
