#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>

#define BUFFER_SIZE 128



int main()
{
    int pipe1[2];   /* P1 -> P2 */
    int pipe2[2];   /* P2 -> P3 */

    pid_t pid1, pid2;
    char buffer[BUFFER_SIZE];

    if (pipe(pipe1) == -1)
    {
        perror("pipe1");
        exit(1);
    }

    if (pipe(pipe2) == -1)
    {
        perror("pipe2");
        exit(1);
    }

    pid1 = fork();

    if (pid1 < 0)
    {
        perror("fork");
        exit(1);
    }

    if (pid1 == 0)
    {
        /* -------------------------
           Child A (P2)
           ------------------------- */
        char cleaned[BUFFER_SIZE];
        int bytesRead;
        int i, j = 0;

        close(pipe1[1]);  /* P2 does not write to pipe1 */
        close(pipe2[0]);  /* P2 does not read from pipe2 */

        bytesRead = read(pipe1[0], buffer, BUFFER_SIZE - 1);
        if (bytesRead < 0)
        {
            perror("read in P2");
            close(pipe1[0]);
            close(pipe2[1]);
            exit(1);
        }

        buffer[bytesRead] = '\0';

        for (i = 0; buffer[i] != '\0'; i++)
        {
            if (!isdigit((unsigned char)buffer[i]))
            {
                cleaned[j] = buffer[i];
                j++;
            }
        }
        cleaned[j] = '\0';

        if (write(pipe2[1], cleaned, strlen(cleaned) + 1) == -1)
        {
            perror("write in P2");
            close(pipe1[0]);
            close(pipe2[1]);
            exit(1);
        }

        close(pipe1[0]);
        close(pipe2[1]);
        exit(0);
    }

    pid2 = fork();

    if (pid2 < 0)
    {
        perror("fork");
        exit(1);
    }

    if (pid2 == 0)
    {
        /* -------------------------
           Child B (P3)
           ------------------------- */
        char received[BUFFER_SIZE];
        int bytesRead;
        int count = 0;
        int i;

        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[1]);  /* P3 does not write to pipe2 */

        bytesRead = read(pipe2[0], received, BUFFER_SIZE - 1);
        if (bytesRead < 0)
        {
            perror("read in P3");
            close(pipe2[0]);
            exit(1);
        }

        received[bytesRead] = '\0';

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

    /* -------------------------
       Parent Process (P1)
       ------------------------- */
    close(pipe1[0]);  /* P1 does not read from pipe1 */
    close(pipe2[0]);  /* P1 does not use pipe2 */
    close(pipe2[1]);

    printf("Enter a string: ");
    if (fgets(buffer, BUFFER_SIZE, stdin) == NULL)
    {
        perror("fgets");
        close(pipe1[1]);
        wait(NULL);
        wait(NULL);
        exit(1);
    }

    buffer[strcspn(buffer, "\n")] = '\0';

    if (write(pipe1[1], buffer, strlen(buffer) + 1) == -1)
    {
        perror("write in P1");
        close(pipe1[1]);
        wait(NULL);
        wait(NULL);
        exit(1);
    }

    close(pipe1[1]);

    wait(NULL);
    wait(NULL);

    return 0;
}
