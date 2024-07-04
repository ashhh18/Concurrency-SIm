#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <sys/time.h>

#define RESET "\033[0m"
#define ORANGE "\e[38;2;255;85;0m"
#define RED "\033[1;31m"
#define CYAN "\033[1;36m"
#define YELLOW "\033[1;33m"
#define WHITE "\033[1;37m"
#define BLUE "\033[1;34m"
#define GREEN "\033[1;32m"

int n, k, f, t;
int overall_time = -1;
int max_at_once = 0;
int curr_people = 0;

sem_t orders_wait[1000];
int sem_check[1000];
int machine_check[1000];
pthread_t order_thread[1000];
pthread_mutex_t lock;
pthread_mutex_t lock1;

struct machine_details
{
    int idx;
    int tm_start;
    int tm_end;
};

struct ice_cream_details
{
    int idx;
    char name[40];
    int t_f;
};

struct topping_details
{
    int idx;
    char name[40];
    int q_t;
    int limited; // limited = -1 means limited, 1 means unlimited
    int cur_count;
};

struct customer_details
{
    int idx;
    int t_arr;
    int orders;
    char **details;
    int valid;
    int finish_cnt;
    int last_time;
    int left;
};

struct order_details
{
    char given_order[1024];
    int customer;
    struct customer_details *cust;
    int time;
    int machine;
    int idx;
    int local_num;
    struct topping_details **tops;
    int valid;
};

struct topping_details *topping[1000 + 1];
void *order_function(void *st)
{
    // sleep(1);
    struct order_details *sst = (struct order_details *)st;
    int idx = sst->idx;
    sem_check[idx] = -1;
    sem_wait(&orders_wait[idx]);
    int top_check = 0;

    char temp[1024];
    strcpy(temp, sst->given_order);
    char *token = strtok(temp, " \n");
    int it = 0;
    int flaggg = 0;
    int cust_idx = sst->customer;

    while (token != NULL)
    {
        if (it == 0)
        {
            it++;
            token = strtok(NULL, " \n");
            continue;
        }
        else
        {
            for (int i = 1; i <= t; i++)
            {
                if (strcmp(token, topping[i]->name) == 0)
                {
                    if (topping[i]->limited == -1)
                    {
                        topping[i]->q_t--;
                    }
                }
            }
        }
        it++;
        token = strtok(NULL, " \n");
    }
    sleep(1);
    printf(CYAN"Machine %d starts preparing ice cream %d of customer %d at %d second(s)\n"RESET, sst->machine, sst->local_num, sst->customer, overall_time);
    int prep_time;
    prep_time = sst->time;
    sleep(prep_time);
    printf(BLUE"Machine %d completes preparing ice cream %d of customer %d at %d seconds(s)\n"RESET, sst->machine, sst->local_num, sst->customer, overall_time);
    sst->cust->finish_cnt++;
    if (overall_time > sst->cust->last_time)
        sst->cust->last_time = overall_time;
    pthread_mutex_lock(&lock);
    machine_check[sst->machine] = 0;
    pthread_mutex_unlock(&lock);
}

int main()
{
    scanf("%d %d %d %d", &n, &k, &f, &t);
    max_at_once = k;
    struct machine_details *machine[n + 1];
    struct ice_cream_details *ice_cream[f + 1];
    struct customer_details *customer[1000];
    struct order_details *orders[1000];

    for (int i = 0; i <= n; i++)
    {
        machine[i] = malloc(sizeof(struct machine_details));
    }
    for (int i = 0; i <= f; i++)
    {
        ice_cream[i] = malloc(sizeof(struct ice_cream_details));
    }
    for (int i = 0; i <= t; i++)
    {
        topping[i] = malloc(sizeof(struct topping_details));
        topping[i]->limited = 1;
    }

    for (int i = 0; i < 1000; i++)
    {
        customer[i] = malloc(sizeof(struct customer_details));
        customer[i]->valid = 1;
        customer[i]->finish_cnt = 0;
        customer[i]->left = 0;
    }

    for (int i = 0; i < 1000; i++)
    {
        orders[i] = malloc(sizeof(struct order_details));
        orders[i]->valid = 1;
    }

    int a1 = n;
    int i1 = 1;
    while (a1--)
    {
        scanf("%d %d", &machine[i1]->tm_start, &machine[i1]->tm_end);
        machine[i1]->idx = i1;
        i1++;
    }
    int i2 = 1;
    int a2 = f;
    while (a2--)
    {
        char temp[40];
        scanf("%s", temp);
        strcpy(ice_cream[i2]->name, temp);
        scanf("%d", &ice_cream[i2]->t_f);
        i2++;
    }

    int a3 = t;
    int i3 = 1;
    while (a3--)
    {
        char temp[40];
        scanf("%s", temp);
        strcpy(topping[i3]->name, temp);
        scanf("%d", &topping[i3]->q_t);
        topping[i3]->cur_count = topping[i3]->q_t;

        if (topping[i3]->q_t != -1)
        {
            topping[i3]->limited = -1;
        }
        i3++;
    }

    int cust_cnt = 0;
    char buffer[1024];
    char garbage;
    scanf("%c", &garbage);
    int total_orders = 0;
    int order_iter = 1;
    while (1)
    {
        cust_cnt++;
        int idx, t_arr, cnt;
        fgets(buffer, 1024, stdin);
        // if(buffer[0]!='\n')
        if (buffer[0] == '\n')
            break;
        char *token = strtok(buffer, " ");
        int it = 0;

        while (token != NULL)
        {
            if (it == 0)
                idx = atoi(token);
            else if (it == 1)
                t_arr = atoi(token);
            else if (it == 2)
                cnt = atoi(token);
            it++;
            token = strtok(NULL, " ");
        }
        total_orders += cnt;

        customer[idx]->idx = idx;
        customer[idx]->orders = cnt;
        customer[idx]->t_arr = t_arr;
        customer[idx]->details = malloc(sizeof(char *) * cnt);
        for (int i = 1; i <= cnt; i++)
        {
            customer[idx]->details[i] = malloc(sizeof(char) * 1024);
        }

        for (int i = 1; i <= cnt; i++)
        {
            int ptr = 0;
            memset(buffer, '\0', 1024);
            fgets(buffer, 1024, stdin);

            for (int h = 0; h < 1024; h++)
            {
                customer[idx]->details[i][h] = buffer[h];
            }
            orders[order_iter]->customer = customer[idx]->idx;
            orders[order_iter]->idx = order_iter;
            orders[order_iter]->local_num = i;
            orders[order_iter]->cust = customer[idx];
            orders[order_iter]->tops = &topping[idx];
            strncpy(orders[order_iter]->given_order, buffer, 1024);
            int lst = 0;
            for (int h = 0; h < 1024; h++)
            {
                if (buffer[h] == ' ')
                {
                    lst = h - 1;
                    break;
                }
            }
            for (int h = 1; h <= f; h++)
            {
                if (strncmp(buffer, ice_cream[h]->name, lst) == 0)
                {
                    orders[order_iter]->time = ice_cream[h]->t_f;
                    break;
                }
            }
            order_iter++;
        }
    }

    cust_cnt--;
    //----------------------------------------------input ends--------------------------------------------------------//

    int machine_check[n + 1];
    memset(machine_check, 0, n + 1);
    overall_time = -1;

    for (int i = 1; i <= total_orders; i++)
    {
        sem_init(&orders_wait[i], 0, -1);
        sem_check[i] = 0;
    }

    for (int i = 1; i <= n; i++)
    {
        machine_check[i] = 0;
    }

    while (1)
    {
        sleep(1);
        overall_time++;
        for (int i = 1; i <= n; i++)
        {
            int m_arr = machine[i]->tm_start;
            if (m_arr == overall_time)
            {
                printf(ORANGE"Machine %d has started working at %d second(s)\n"RESET, machine[i]->idx, machine[i]->tm_start);
            }
        }

        for (int i = 1; i <= n; i++)
        {
            if (overall_time >= machine[i]->tm_start && machine[i]->tm_end > overall_time && machine_check[i] == 0)
            {
                for (int j = 1; j <= total_orders; j++)
                {
                    if (machine_check[i] == 0 && orders[j]->valid == 1)
                    {
                        int val;
                        val = sem_check[j];
                        // sem_getvalue(&orders_wait[j],&val);
                        if (val >= 0)
                            continue;
                        else
                        {
                            int left_time = 0;
                            left_time = machine[i]->tm_end - overall_time;
                            if (left_time < orders[j]->time)
                            {
                                continue;
                            }
                            else
                            {
                                orders[j]->machine = i;
                                sem_check[j] = 0;
                                machine_check[i] = 1;

                                char temp[1024];
                                strcpy(temp, orders[j]->given_order);
                                char *token = strtok(temp, " \n");
                                int it = 0;
                                int flaggg = 0;
                                int cust_idx = orders[j]->customer;
                                if (customer[cust_idx]->valid == 0)
                                    continue;

                                while (token != NULL)
                                {
                                    if (it == 0)
                                    {
                                        it++;
                                        token = strtok(NULL, " \n");
                                        continue;
                                    }
                                    else
                                    {
                                        for (int m = 1; m <= t; m++)
                                        {
                                            if (strcmp(token, topping[m]->name) == 0)
                                            {
                                                if (topping[m]->limited == -1)
                                                {
                                                    if (topping[m]->q_t == 0)
                                                    {
                                                        orders[m]->valid = 0;
                                                        customer[cust_idx]->valid = 0;
                                                        flaggg = 1;
                                                        break;
                                                    }
                                                }
                                            }
                                        }
                                        if (flaggg == 1)
                                            break;
                                    }
                                    if (flaggg == 1)
                                        break;
                                    it++;
                                    token = strtok(NULL, " \n");
                                }
                                if (flaggg != 1)
                                {
                                    sem_post(&orders_wait[j]);
                                    usleep(1000);
                                }
                            }
                        }
                    }
                }
            }
        }

        for (int i = 1; i <= cust_cnt; i++)
        {
            if (customer[i]->left == 0)
            {
                if (customer[i]->valid == 0)
                {
                    printf(RED"Customer %d left at %d second(s) due to lack of ingredients\n"RESET, i, overall_time);
                    customer[i]->left = 1;
                }
            }
        }

        for (int i = 1; i <= cust_cnt; i++)
        {
            int t_arr = customer[i]->t_arr;
            if (t_arr == overall_time)
            {
                printf("Customer %d enters at %d second(s)\n", i, overall_time);
                if (curr_people >= k)
                {
                    // usleep(1000);
                    printf(RED"Customer %d exits at %d second(s) due to no space available\n"RESET, i, overall_time);
                    customer[i]->valid = 0;
                    customer[i]->left = 1;
                }
                pthread_mutex_lock(&lock1);
                curr_people++;
                pthread_mutex_unlock(&lock1);
                if (customer[i]->valid == 1 && customer[i]->left == 0)
                {
                    printf(YELLOW"Customer %d order a %d ice cream(s)\n"RESET, i, customer[i]->orders);
                    int z = customer[i]->orders;
                    int r = 0;
                    while (z--)
                    {
                        r++;
                        printf(YELLOW"Ice cream %d: "RESET, r);
                        printf(YELLOW"%s"RESET, customer[i]->details[r]);
                    }
                }
            }
        }

        for (int i = 1; i <= total_orders; i++)
        {
            int t_arr = customer[orders[i]->customer]->t_arr;
            if (t_arr == overall_time)
            {
                pthread_create(&order_thread[i], NULL, order_function, orders[i]);
            }
        }
        for (int i = 1; i <= cust_cnt; i++)
        {
            if (customer[i]->finish_cnt == customer[i]->orders)
            {
                customer[i]->finish_cnt++;
                printf(GREEN"Customer %d has collected their order(s) and left at %d second(s)\n"RESET, i, customer[i]->last_time);
            }
        }
        for (int i = 1; i <= n; i++)
        {
            int m_end = machine[i]->tm_end;
            if (m_end == overall_time)
            {
                printf(ORANGE"Machine %d has stopped working at %d second(s)\n"RESET, machine[i]->idx, machine[i]->tm_end);
            }
        }
        int z = 0;
        for (int i = 1; i <= n; i++)
        {
            if (overall_time >= machine[i]->tm_end)
                z++;
        }
        if (z == n)
            break;
    }
    for (int i = 1; i <= cust_cnt; i++)
    {
        if (customer[i]->finish_cnt < customer[i]->orders && customer[i]->valid == 1)
            printf(RED"Customer %d was not serviced due to unavailability of machines\n"RESET, i);
    }
    printf("Parlour Closed\n");
}