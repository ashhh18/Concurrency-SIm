#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <sys/time.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

int b, k, n;
int time_arr[15], tol_arr[15], wait_arr[15];
int overall_time = -1;
int wasted = 0;
int leave_ct = 0;
double wait_time = 0;

int barista_free[100];

pthread_mutex_t waste;

time_t time_st;
sem_t customer_sem;
sem_t barista_sem;

struct cofe_details
{
    int hash;
    char name[40];
    int taken;
    int time;
};

struct details
{
    int idx;
    int tol_time;
    int arr_time;
    int making_time;
    char name[40];
    int barista;
};

void *customers(void *st)
{
    sem_post(&customer_sem);
    struct details *sst = (struct details *)st;
    printf("Customer %d arrives at %d seconds(s)\n", sst->idx, overall_time);
    printf("\x1b[33mCustomer %d orders a %s\n\x1b[0m", sst->idx, sst->name);

    int created_time = 0;
    struct timespec abs_timeout;
    clock_gettime(CLOCK_REALTIME, &abs_timeout);
    int x = sst->tol_time + 1;
    // printf("%d\n",x);
    abs_timeout.tv_sec += sst->tol_time;

    // int problem_time = 0;
    // problem_time = overall_time +

    int z = sem_timedwait(&barista_sem, &abs_timeout);
    if (z == -1)
    {
        sleep(1);
        // if (sst->idx == 2)
        // {
        //     printf ("hiii\n");
        // }

        printf("\x1b[31mCustomer %d leaves without their order at %d second(s)\n\x1b[0m", sst->idx, overall_time);
        pthread_mutex_lock(&mutex1);
        leave_ct++;
        pthread_mutex_unlock(&mutex1);
        return NULL;
    }
    for (int e = 1; e <= b; e++)
    {
        if (barista_free[e] == -1)
        {
            barista_free[e] = 1;
            sst->barista = e;
            break;
        }
    }

    int coffee_start = overall_time;
    sleep(1);
    printf("\x1b[36mBarista %d begins preparing the order of customer %d at %d second(s)\n\x1b[0m", sst->barista, sst->idx, overall_time);
    int end_time = coffee_start + sst->making_time + 1;
    // sst->tol_time;
    int scam_time = sst->arr_time + sst->tol_time + 1;
    // if (scam_time <= coffee_start)
    // {
    //     printf("\x1b[31mCustomer %d leaves without their order at %d second(s)\n\x1b[0m", sst->idx, overall_time);
    //     sem_post(&barista_sem);
    // }
    // if (sst->idx == 2)
    //     {
    //         printf ("%d %d %d\n",end_time);
    //     }

    if (end_time > sst->arr_time + sst->tol_time + 1)
    {
        int minn = sst->arr_time + sst->tol_time + 1 - overall_time;
        sleep(minn);
        printf("\x1b[31mCustomer %d leaves without their order at %d second(s)\n\x1b[0m", sst->idx, overall_time);
        pthread_mutex_lock(&mutex);
        wait_time += (overall_time-sst->arr_time);
        wasted++;
        pthread_mutex_unlock(&mutex);

        int remaining = 0;
        remaining = end_time - overall_time;

        sleep(remaining);
        printf("\x1b[34mBarista %d completes the order of customer %d at %d second(s)\n\x1b[0m", sst->barista, sst->idx, overall_time); // ???????
        barista_free[sst->barista] = -1;
        sem_post(&barista_sem);
        pthread_mutex_lock(&mutex1);
        leave_ct++;
        pthread_mutex_unlock(&mutex1);
    }
    else
    {
        while (1)
        {
            if (end_time == overall_time)
            {
                printf("\x1b[34mBarista %d completes the order of customer %d at %d second(s)\n\x1b[0m", sst->barista, sst->idx, overall_time);
                barista_free[sst->barista] = -1;
                printf("\x1b[32mCustomer %d leaves with their order at %d second(s)\n\x1b[0m", sst->idx, overall_time);
                pthread_mutex_lock(&mutex2);
                wait_time += (overall_time-sst->arr_time);
                pthread_mutex_unlock(&mutex2);

                break;
            }
        }
        sem_post(&barista_sem);
        usleep(1000);
        pthread_mutex_lock(&mutex1);
        leave_ct++;
        pthread_mutex_unlock(&mutex1);
    }
    return NULL;
}

int main()
{
    scanf("%d %d %d", &b, &k, &n);
    memset(barista_free, -1, 100 * sizeof(barista_free[0]));
    int custom_hash = 1;
    int total_coffee[k + 1];
    struct cofe_details *store[n + 1];
    for (int i = 0; i <= n; i++)
    {
        store[i] = malloc(sizeof(struct cofe_details));
        store[i]->hash = -1;
        store[i]->taken = 0;
    }
    int customer[n + 1];
    pthread_t custom_thread[n + 1];
    int temp = k;
    while (temp--)
    {
        char coffee[11];
        scanf("%s", coffee);
        int t_prep;
        scanf("%d", &t_prep);

        for (int i = 1; i <= k; i++)
        {
            if (store[i]->taken == 0)
            {
                strcpy(store[i]->name, coffee);
                store[i]->taken = 1;
                store[i]->time = t_prep;
                break;
            }
        }
    }
    sem_init(&customer_sem, 0, 0);
    sem_init(&barista_sem, 0, b);

    int j = 1;
    overall_time = -1;
    int br = n;

    struct details *arg_struct[n + 1];
    for (int i = 0; i <= n; i++)
    {
        arg_struct[i] = malloc(sizeof(struct details));
    }

    while (br--)
    {
        int i, t_arr_i, tol_arr_i;
        char coffee[11];
        scanf("%d", &i);
        scanf("%s", coffee);
        scanf("%d %d", &t_arr_i, &tol_arr_i);

        time_arr[i] = t_arr_i;
        tol_arr[i] = tol_arr_i;

        for (int t = 1; t <= k; t++)
        {
            if (strcmp(coffee, store[t]->name) == 0)
            {
                int x = store[t]->time;
                strcpy(arg_struct[i]->name, coffee);
                wait_arr[i] = store[t]->time;
                break;
            }
        }
        j = i;
    }
    struct timeval st, en;
    gettimeofday(&st, NULL);
    gettimeofday(&en, NULL);
    double timediff = st.tv_sec - en.tv_sec + (st.tv_usec - en.tv_usec) / 1000000;
    j = 1;

    while (1)
    {
        gettimeofday(&en, NULL);
        double timediff = en.tv_sec - st.tv_sec + (en.tv_usec - st.tv_usec) / 1000000;
        if (timediff - overall_time >= 1)
        {
            overall_time++;
            int time_left = 0;
            time_left = time_arr[j] - overall_time;

            if (time_left == 0)
            {
                for (int k = j; k <= n; k++)
                {
                    if (time_arr[k] - overall_time != 0)
                        break;
                    arg_struct[k]->arr_time = time_arr[k];
                    arg_struct[k]->idx = k;
                    arg_struct[k]->tol_time = tol_arr[k];
                    arg_struct[k]->making_time = wait_arr[k];
                    int y = arg_struct[k]->idx;
                    pthread_create(&custom_thread[j++], NULL, customers, arg_struct[k]);
                    usleep(5000);
                    sem_wait(&customer_sem);
                }
            }
        }
        if (leave_ct >= n)
            break;
    }
    wait_time /= (n*n);
    for (int i = 1; i <= n; i++)
    {
        pthread_join(custom_thread[i], NULL);
    }
    printf("\n%d coffee wasted\n", wasted);
    // printf ("%lf : Avg Wait-time\n",wait_time);
}