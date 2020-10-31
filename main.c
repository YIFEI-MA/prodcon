#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>


pthread_mutex_t x_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t x_cond = PTHREAD_COND_INITIALIZER;

int works_remain = 0;
int total_works = 0;
int sleeps = 0;

typedef struct node {
    int para;
    char command;
    struct node *next;
} node_t;

void enqueue(node_t **head, int para, char command) {
    node_t *new_node = malloc(sizeof(node_t));
    if (!new_node) return;

    new_node->para = para;
    new_node->command = command;
    new_node->next = *head;

    *head = new_node;
}

int dequeue(node_t **head) {
    node_t *current, *prev = NULL;
    int retval = -1;

    if (*head == NULL) return -1;

    current = *head;
    while (current->next != NULL) {
        prev = current;
        current = current->next;
    }

    retval = current->para;
    free(current);

    if (prev)
        prev->next = NULL;
    else
        *head = NULL;

    return retval;
}


int main(int argc, char *argv[]) {
    fflush(stdout);
    char filename[] = "prodcon.0.log";
    if (argc == 3) {
        filename[8] = *argv[2];
    }
    // https://stackoverflow.com/questions/8516823/redirecting-output-to-a-file-in-c#:~:text=When%20you%20fork%20the%20child,ls%20as%20usual%3B%20its%20standard
    int out = open(filename, O_RDWR|O_CREAT|O_APPEND, 0600);
    if (-1 == out) { perror("opening log"); return 255; }
    int save_out = dup(fileno(stdout));
    if (-1 == dup2(out, fileno(stdout))) { perror("cannot redirect stdout"); return 255; }





    printf("Hello, World!\n");
    fflush(stdout);
    dup2(save_out, fileno(stdout));
    close(out);
    close(save_out);
    return 0;
}
