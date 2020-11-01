#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/time.h>
#include "tands.h"

pthread_mutex_t x_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t x_cond = PTHREAD_COND_INITIALIZER;

int works_remain = 0;
int total_works = 0;
int sleeps = 0;
int receives = 0;
int total_asks = 0;
int total_completes = 0;
bool get_all_works = false;
int thread_id = 1;
int *thread_work_counts;

struct timeval start;

typedef struct node {
    int para;
    struct node *next;
} node_t;

node_t *queue_head = NULL;

void enqueue(node_t **head, int para) {
    node_t *new_node = malloc(sizeof(node_t));
    if (!new_node) return;

    new_node->para = para;
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

double get_time(){
    struct timeval end;
    gettimeofday(&end, NULL);
    double time = end.tv_sec + end.tv_usec / 1e6 - start.tv_sec - start.tv_usec / 1e6;
    return time;
}


void producer() {
    char line[10];

    while (fgets(line, sizeof(line), stdin) != NULL) {
        if (line[0] == 'T') {
            char *endptr;
            int para = (int) strtol(&line[1], &endptr, 10);
            pthread_mutex_lock(&x_mutex);
            pthread_cond_signal(&x_cond);
            works_remain++;
            total_works++;
            enqueue(&queue_head, para);
//            sleep(1);
//            printf("%c %d %.3f\n", line[0], para, get_time());
            printf("   %.3f ID= 0 Q= %d\tWork         %d\n", get_time(), works_remain, para);
            pthread_mutex_unlock(&x_mutex);
        }
        else {
            char *endptr;
            int para = (int) strtol(&line[1], &endptr, 10);
            pthread_mutex_lock(&x_mutex);
            printf("   %.3f ID= 0     \tSleep        %d\n", get_time(), para);
            sleeps++;
            pthread_mutex_unlock(&x_mutex);
            Sleep(para);
        }
    }
    get_all_works = true;
}

void consumer() {

    pthread_mutex_lock(&x_mutex);
    int id = thread_id;
    thread_id++;
    thread_work_counts[id-1] = 0;
    pthread_mutex_unlock(&x_mutex);

    int para;

    while (true) {
        pthread_mutex_lock(&x_mutex);

        printf("   %.3f ID= %d     \tAsk\n",get_time(), id);
        total_asks++;
        pthread_mutex_unlock(&x_mutex);
        jump:
        pthread_mutex_lock(&x_mutex);
        para = dequeue(&queue_head);
        if (para == -1) {
            if (get_all_works == false) {
                pthread_cond_wait(&x_cond, &x_mutex);
                pthread_mutex_unlock(&x_mutex);
//                continue;
                goto jump;
            }
            else {
                pthread_mutex_unlock(&x_mutex);
                pthread_exit(0);
            }
        }
        else {
            thread_work_counts[id-1]++;
            works_remain--;
            receives++;
            printf("   %.3f ID= %d Q= %d\tReceive      %d\n", get_time(), id, works_remain, para);
            pthread_mutex_unlock(&x_mutex);
            Trans(para);

            pthread_mutex_lock(&x_mutex);
            total_completes++;
            printf("   %.3f ID= %d     \tComplete     %d\n", get_time(), id, para);
            pthread_mutex_unlock(&x_mutex);
        }

    }

}



int main(int argc, char *argv[]) {
    char *endptr;
    int num_threads = (int) strtol(argv[1], &endptr, 10);

    thread_work_counts = malloc(sizeof(int)*num_threads);
    fflush(stdout);
    gettimeofday(&start, NULL);
    char filename[] = "prodcon.0.log";
    if (argc == 3) {
        filename[8] = *argv[2];
    }
    // https://stackoverflow.com/questions/8516823/redirecting-output-to-a-file-in-c#:~:text=When%20you%20fork%20the%20child,ls%20as%20usual%3B%20its%20standard
    int out = open(filename, O_RDWR|O_CREAT|O_APPEND, 0600);
    if (-1 == out) { perror("opening log"); return 255; }
    int save_out = dup(fileno(stdout));
    if (-1 == dup2(out, fileno(stdout))) { perror("cannot redirect stdout"); return 255; }

    printf("\nstart\n");

    pthread_t thread_ids[num_threads];
    for (int i=0; i < num_threads; i++) {
        pthread_create(&thread_ids[i], NULL, consumer, NULL);
    }

    producer();
    for (int i=0; i<num_threads; i++) {
        pthread_join(thread_ids[i], NULL);
    }

    printf("Summary:\n");
    printf("\tWork\t       \t%d\n", total_works);
    printf("\tAsk\t        \t%d\n", total_asks);
    printf("\tReceive\t    \t%d\n", receives);
    printf("\tComplete\t   \t%d\n", total_completes);
    printf("\tSleep\t      \t%d\n", sleeps);

    for (int i=1; i<=num_threads; i++) {
        printf("\tThread\t%d   \t%d\n",i, thread_work_counts[i-1]);
    }


    double trans_per_sec;
    trans_per_sec = total_works / get_time();
    printf("Transactions per second: %.2f", trans_per_sec);

    fflush(stdout);
    dup2(save_out, fileno(stdout));
    close(out);
    close(save_out);
    return 0;
}
