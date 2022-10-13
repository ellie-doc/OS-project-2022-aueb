#include "p3180269-p3200276.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h> // for CLOCK_REALTIME?
#include <pthread.h>

// generate random number between two values (inclusive)
#define RND_LOW_HIGH(L, H) (rand_r(&seedp) % ((H) - (L) + 1) + (L))

// Customer (caller) info struct
struct customer_status
{
    int id;        // customer id (also thread id)
    int status;    // 0=initial(δεν υπαρχουν θεσεις), 1=seats found, 2=payment made
    int t_start;   // clock time start
    int t_end;     // clock time end
    int t_service; // xronos eksipirethshs
    int t_waiting; // xronos anamonhs
    int zone;      // seating zone
    int seats;     // number of seats requested
    int row;       // row of found seats
    int leftmost;  // starting seat
    int rightmost; // ending seat
    int cost;      // total tickets cost (for payment)
};

void* theater(void * arg);
int getSeats(struct customer_status *cst);
int pay(struct customer_status *cst);
int print_plan();

int bankacc = 0;
int operators = tel; // διαθεσιμοι τηλεφωνητες
int cashiers = cash; // διαθεσιμοι ταμιες
unsigned int seedp;
int plan[zonea + zoneb][seat];
int count0 = 0; // counters για τον υπολογισμο ποσοστου συναλλαγων με τους 3 τροπους
int count1 = 0;
int count2 = 0;

// sum times
int waitsum = 0;
int tsum = 0;

pthread_mutex_t o; // operators
pthread_mutex_t c; // cashiers
pthread_mutex_t p; // plan
pthread_mutex_t s; // statistics
pthread_mutex_t tws; // wait sum
pthread_mutex_t tss; // service sum
pthread_cond_t op; // operators
pthread_cond_t ca; // cashiers


int main(int argc, char *argv[])
{
    int Ncust = atoi(argv[1]);
    seedp = atoi(argv[2]);

    // initialize seating
    for (int i = 0; i < zonea + zoneb; i++)
    {
        for (int j = 0; j < seat; j++)
        {
            plan[i][j] = 0;
        }
    }

    // mutex and cond init
    pthread_mutex_init(&o, NULL);
    pthread_mutex_init(&c, NULL);
    pthread_mutex_init(&p, NULL);
    pthread_mutex_init(&s, NULL);
    pthread_cond_init(&op, NULL);
    pthread_cond_init(&ca, NULL);

    // threads πελατών
    pthread_t *threads = malloc(sizeof(pthread_t) * Ncust);
    for (int i = 1; i <= Ncust; i++)
    {
        int *id = malloc(sizeof(int));
        *id = i;
        int t = pthread_create(&threads[i-1], NULL, &theater, id);
        // ο 1ος πελάτης δεν περιμένει
        if(*id!=1){
            // wait some random time before the next customer call
            sleep(RND_LOW_HIGH(t_reslow, t_reshigh));
        }  
    }

    for (int i = 1; i < Ncust+1; i++)
    {
        int t = pthread_join(threads[i-1], NULL);
    }

    // mutex and cond destroy
    pthread_mutex_destroy(&o);
    pthread_mutex_destroy(&c);
    pthread_mutex_destroy(&p);
    pthread_mutex_destroy(&s);
    pthread_cond_destroy(&op);
    pthread_cond_destroy(&ca);

    print_plan();

    return 0;
}

int getSeats(struct customer_status *cst)
{
    struct timespec tstart;
    struct timespec tend;
    clock_gettime(CLOCK_REALTIME, &tstart); // start waiting time (op)
    pthread_mutex_lock(&o);
    // δεν υπαρχει διαθεσιμος τηλεφωνητης, wait
    while (operators == 0)
    {
        pthread_cond_wait(&op, &o);
    }
    operators--;
    pthread_mutex_unlock(&o);
    clock_gettime(CLOCK_REALTIME, &tend); // end waiting time (op)
    cst->t_waiting += tend.tv_sec - tstart.tv_sec;

    // seat selection
    int zone = RND_LOW_HIGH(1, 100) <= (p_zonea * 100) ? 0 : 1; // 0=ZoneA, 1=ZoneB
    int seats = RND_LOW_HIGH(seatlow, seathigh);                // number of seats
    int start_row = zone == 0 ? 0 : zonea;
    int end_row = zone == 0 ? zonea : zonea + zoneb;
    int price = zone == 0 ? c_zonea : c_zoneb; // zone price per seat
    int pay_total = seats * price;

    int wait_t2 = RND_LOW_HIGH(t_seatlow, t_seathigh); // operator wait time για επομενο πελατη
    sleep(wait_t2);

    cst->status = 0;
    cst->zone = zone;
    cst->seats = seats;
    cst->cost = pay_total;

    // lock plan
    pthread_mutex_lock(&p);

    // check available seats in row
    for (int i = start_row; i < end_row; i++)
    {
        int counter = 0;
        for (int j = 0; j < seat; j++)
        {
            if (plan[i][j] == 0)
            {
                counter++;
            }
            else
            {
                counter = 0; // πετυχαμε πιασμενη θεση, παλι απο την αρχη
                continue;
            }
            if (counter == seats) // κανουμε τη διαδικασια μεχρι να βρουμε seats στη σειρα
            {
                for (int k = 0; k < seats; k++)
                {
                    plan[i][j - k] = cst->id; // δεσμευσε τις θεσεις με το id του πελατη
                }
                cst->row = i;
                cst->leftmost = j - seats;
                cst->rightmost = j;
                cst->status = 1;
                break;
            }
        }
        if (cst->status == 1)
            break;
    }
    pthread_mutex_unlock(&p); // release the plan
    pthread_mutex_lock(&o);
    operators++;
    pthread_cond_signal(&op);
    pthread_mutex_unlock(&o); // release the operator

    clock_gettime(CLOCK_REALTIME, &tend); // τελος εξυπηρετηση τηλεφωνητη
    cst->t_service += tend.tv_sec - tstart.tv_sec;

    if (cst->status == 1)
        return 1;
}

int pay(struct customer_status *cst)
{
    struct timespec tstart;
    struct timespec tend;
    clock_gettime(CLOCK_REALTIME, &tstart); // start waiting time (ca)
    // go to cashier
    pthread_mutex_lock(&c);
    // δεν υπαρχει διαθεσιμος ταμιας, wait
    while (cashiers == 0)
    {
        pthread_cond_wait(&ca, &c);
    }
    cashiers--;
    pthread_mutex_unlock(&c);
    clock_gettime(CLOCK_REALTIME, &tend); // end waiting time (op)
    cst->t_waiting += tend.tv_sec - tstart.tv_sec;

    int wait_t3 = RND_LOW_HIGH(t_cashlow, t_cashhigh);
    sleep(wait_t3);
    if (p_cardsuccess * 100 >= rand_r(&seedp) % (101))
    {
        //ο πελάτης πληρώνει για το συνολικό κόστος και τις θέσεις του στην επιλεγμένη ζώνη.
        bankacc += cst->cost;
        cst->status = 2;
    }
    else
    {
        // αποτυχία συναλλαγής, οι θέσεις επιστρέφονται στο πλάνο του θεάτρου από τον ταμία
        for (int j = cst->leftmost; j <= cst->rightmost; j++)
        {
            plan[cst->row][j] = 0;
        }
    }
    pthread_mutex_lock(&c);
    cashiers++;               // release the cashier
    pthread_cond_signal(&ca);
    pthread_mutex_unlock(&c); 

    clock_gettime(CLOCK_REALTIME, &tend); // τελος εξυπηρετηση ταμία
    cst->t_service += tend.tv_sec - tstart.tv_sec;
}

void *theater(void *arg)
{
    struct customer_status *cst = malloc(sizeof(struct customer_status));
    int th_id = *(int *)arg;
    cst->id = *(int *)arg;
    cst->t_service = 0;
    cst->t_waiting = 0;

    // καθε επομενος πελατης
    int r = getSeats(cst);

    // check for seating failure
    if (r && cst->status == 1)
    {
        // proceed with payment
        r = pay(cst);
    }

    //update wait sum
    pthread_mutex_lock(&tws);
    waitsum += cst->t_waiting;
    pthread_mutex_unlock(&tws);
    //update serice sum
    pthread_mutex_lock(&tss);
    tsum += cst->t_service;
    pthread_mutex_unlock(&tss);

    if (cst->status == 0)
    {
        printf("%d: Η κράτηση απέτυχε γιατί δεν υπάρχουν κατάλληλες θέσεις.\n", cst->id);
        count0 += 1;
    }
    else if (cst->status == 1)
    {
        printf("%d: Η κράτηση απέτυχε γιατί η συναλλαγή με την πιστωτική κάρτα δεν έγινε αποδεκτή.\n", cst->id);
        count1 += 1;
    }
    else
    {
        printf("%d: Η κράτηση ολοκληρώθηκε επιτυχώς. Οι θέσεις σας είναι στη ζώνη %c, σειρά %d, αριθμός ",  cst->id, cst->zone == 0 ? 'A' : 'B', cst->row+1);
        for (int i = cst->leftmost+2; i < cst->rightmost+2; i++)
        {
            printf("%d, ", i); // εμφανισε τους αριθμους θεσης που εχει δεσμευσει
        }
        printf("και το κόστος της συναλλαγής είναι %d ευρώ.\n", cst->cost);
        count2 += 1;
    }
    printf("Service time: %d\nWaiting time: %d\n", cst->t_service, cst->t_waiting);
    pthread_exit(NULL);

    return NULL;
}

int print_plan()
{
    // the plan
    printf("---------------- ZONE A ----------------");
    for (int i = 0; i < seat; i++)
    {
        printf("\n");
        for (int j = 0; j < zonea; j++)
        {
            printf("%-4d", plan[i][j]);
        }
    }
    printf("\n---------------- ZONE B ----------------");
    for (int i = zonea; i < zonea + zoneb; i++)
    {
        printf("\n");
        for (int j = 0; j < seat; j++)
        {
            printf("%-4d", plan[i][j]);
        }
    }
    printf("\n");
    
    printf("Τα συνολικά έσοδα είναι %d ευρώ.\n", bankacc);

    // ποσοστό συναλλαγών με 3 τρόπους
    // printf("%d, %d, %d \n", count0, count1, count2);
    printf("Το ποσοστό συναλλαγών με αποτυχία θέσεων είναι %0.2f%%.\n", ((float)count0 / (count0 + count1 + count2) * 100));
    printf("Το ποσοστό συναλλαγών με αποτυχία συναλλαγής είναι %0.2f%%.\n", ((float)count1 / (count0 + count1 + count2) * 100));
    printf("Το ποσοστό συναλλαγών με επιτυχία είναι %0.2f%%.\n", (count2 / ((float)count0 + count1 + count2) * 100));

    printf("Μέσος χρόνος αναμονής: %0.2f sec\n", (float)waitsum/(count0 + count1 + count2));
    printf("Μέσος χρόνος εξυπηρέτησης: %0.2f sec\n", (float)tsum/(count0 + count1 + count2));

}