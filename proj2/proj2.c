/*
* @author: xseman06
* @date: 2023-04-20
* @project: Projekt 2
*/

#include "proj2.h"

/**
 * @todo: opravit cas posty <f/2, f>
 * osetrit lepsie argumenty +
 * urobit .h subor
 * errory musia uvolnovat pamat
 * errory aj pre init semaforoch +
 * okomentovat veci
*/


// prints help how to use the program
void help(){
    fprintf(stderr, "Usage: ./proj2 NZ NU TZ TU F\n");
    fprintf(stderr, "NZ: počet zákazníků\n");
    fprintf(stderr, "NU: počet úředníků\n");
    fprintf(stderr, "TZ: Maximální čas v milisekundách, po který zákazník po svém vytvoření čeká, než vejde na poštu. 0<=TZ<=10000\n");
    fprintf(stderr, "TU: Maximální délka přestávky úředníka v milisekundách. 0<=TU<=100\n");
    fprintf(stderr, "F: Maximální čas v milisekundách, po kterém je uzavřena pošta pro nově příchozí. 0<=F<=10000\n");
}

// checks if the number of arguments is correct
int argcheck(int argc, char *argv[]) {

    if (argc != 6) {
        fprintf(stderr, "Incorrect number of arguments \n");
        help();
        return EXIT_FAILURE;
    }
    
    // checks if arguments are numbers
    char *a, *b, *c, *d, *e;
    int NZ = strtol(argv[1], &a, 10);
    int NU = strtol(argv[2], &b, 10);
    int TZ = strtol(argv[3], &c, 10);
    int TU = strtol(argv[4], &d, 10);
    int F = strtol(argv[5], &e, 10);

    if (*a != '\0' || *b != '\0' || *c != '\0' || *d != '\0' || *e != '\0') {
        fprintf(stderr, "Incorrect format of arguments\n");
        return EXIT_FAILURE;
    }

    // checks if NZ or NU is in range
    if (NZ < 0 || NU < 0){
        fprintf(stderr, "Incorrect format of arguments (NZ/NU)\n");
        return EXIT_FAILURE;
    }

    // checks if TZ or F is in range 0<=TZ/F<=10000
    if ( TZ < 0 || TZ > 10000 || F < 0 || F > 10000 ){
        fprintf(stderr, "Incorrect format of arguments (TZ/F)\n");
        return EXIT_FAILURE;
    }
    
    // checks if TU is in range 0<=TZ/F<=100
    if ( TU < 0 || TU > 100){
        fprintf(stderr, "Incorrect format of arguments (TU)\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void startup(){

    // open file
    file = fopen("proj2.out", "w");
    if (file == NULL){
        fprintf(stderr, "Error opening file %s\n", "proj2.out");
        exit(EXIT_FAILURE);
    }

    // initialize shared variables
    mutex = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    customer_done = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    employee_done = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    action_counter = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    post_office = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    queue = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    queue2 = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    queue3 = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    queue_done = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    queue2_done = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    queue3_done = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    queue_size = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    queue_size2 = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    queue_size3 = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    // checks if shared variables were created correctly
    if (mutex == MAP_FAILED 
    ||  customer_done == MAP_FAILED 
    ||  employee_done == MAP_FAILED 
    ||  action_counter == MAP_FAILED 
    ||  post_office == MAP_FAILED 
    ||  queue == MAP_FAILED 
    ||  queue2 == MAP_FAILED 
    ||  queue3 == MAP_FAILED 
    ||  queue_done == MAP_FAILED 
    ||  queue2_done == MAP_FAILED
    ||  queue3_done == MAP_FAILED 
    ||  queue_size == MAP_FAILED 
    ||  queue_size2 == MAP_FAILED 
    ||  queue_size3 == MAP_FAILED )
    {
        fprintf(stderr, "Error creating shared memory\n");
        exit(EXIT_FAILURE);
    }


    // initialize semaphores
    if ( 
           sem_init(mutex, 1, 1) == -1 
        || sem_init(queue_done, 1, 0) == -1
        || sem_init(queue2_done, 1, 0) == -1
        || sem_init(queue3_done, 1, 0) == -1
        || sem_init(customer_done, 1, 0) == -1 
        || sem_init(employee_done, 1, 0) == -1 
        || sem_init(queue, 1, 0) == -1 
        || sem_init(queue2, 1, 0) == -1 
        || sem_init(queue3, 1, 0) == -1
        ){
        fprintf(stderr, "Error creating semaphore\n");
        exit(EXIT_FAILURE);
    }
    setbuf(file, NULL);
}

void cleanup(){

    // close semaphores
    sem_destroy(mutex);
    sem_destroy(queue_done);
    sem_destroy(queue2_done);
    sem_destroy(queue3_done);
    sem_destroy(customer_done);
    sem_destroy(employee_done);
    sem_destroy(queue);
    sem_destroy(queue2);
    sem_destroy(queue3);

    // close shared memory
    munmap(mutex, sizeof(sem_t));
    munmap(queue_done, sizeof(sem_t));
    munmap(queue2_done, sizeof(sem_t));
    munmap(queue3_done, sizeof(sem_t));
    munmap(customer_done, sizeof(sem_t));
    munmap(employee_done, sizeof(sem_t));
    munmap(action_counter, sizeof(int));
    munmap(post_office, sizeof(int));
    munmap(queue, sizeof(sem_t));
    munmap(queue2, sizeof(sem_t));
    munmap(queue3, sizeof(sem_t));
    munmap(queue_size, sizeof(int));
    munmap(queue_size2, sizeof(int));
    munmap(queue_size3, sizeof(int));

    // close file
    if(file != NULL){
        fclose(file);
    }
}


void customer_process(Customer customer){
    srand(getpid());

    sem_wait(mutex);
    fprintf(file, "%d: Z %d: started\n", ++(*action_counter), customer.CustomerID);
    sem_post(mutex);

    // waits infront of post office
    usleep((rand() % customer.CustomerWaitTime)*1000);
    
    // enters post office
    sem_wait(mutex);
    if (*post_office == 1){
        fprintf(file, "%d: Z %d: going home\n", ++(*action_counter), customer.CustomerID);
        sem_post(mutex);
        exit(EXIT_SUCCESS);
    }
    customer.CustomerDemand = (rand() % 3)+1;
    fprintf(file, "%d: Z %d: entering office for a service %d\n", ++(*action_counter), customer.CustomerID, customer.CustomerDemand);
    sem_post(mutex);

    if (customer.CustomerDemand == 1){
        (*queue_size)++;
        sem_wait(queue);
        sem_wait(mutex);
        fprintf(file, "%d: Z %d: called by office worker\n", ++(*action_counter), customer.CustomerID);
        sem_post(queue_done);
        sem_post(mutex);
    }
    else if (customer.CustomerDemand == 2){
        (*queue_size2)++;
        sem_wait(queue2);
        sem_wait(mutex);
        fprintf(file, "%d: Z %d: called by office worker\n", ++(*action_counter), customer.CustomerID);
        sem_post(queue2_done);
        sem_post(mutex);

    }
    else if (customer.CustomerDemand == 3){
        (*queue_size3)++;
        sem_wait(queue3);
        sem_wait(mutex);
        fprintf(file, "%d: Z %d: called by office worker\n", ++(*action_counter), customer.CustomerID);
        sem_post(mutex);
        sem_post(queue3_done);
    }
    else{
        exit(EXIT_FAILURE);
    }
    sem_wait(employee_done);
    sem_wait(mutex);
    fprintf(file, "%d: Z %d: going home\n", ++(*action_counter), customer.CustomerID);
    sem_post(mutex);
    exit(EXIT_SUCCESS);

}

void employee_process(int EmployeeID, int breaktime){
    srand(getpid());

    sem_wait(mutex);
    fprintf(file, "%d: U %d: started\n", ++(*action_counter), EmployeeID);
    sem_post(mutex);

    while(true){
        int random = (rand() % 3) + 1;
        printf("U %d: posta %d, queue %d, queue2 %d, queue3 %d, rand %d\n", EmployeeID, *post_office, *queue_size, *queue_size2, *queue_size3, random);
        if(random == 1 && *queue_size > 0){
            sem_wait(mutex);
            sem_post(queue);
            (*queue_size)--;
            sem_post(mutex);
            sem_wait(queue_done);
            sem_wait(mutex);
            fprintf(file, "%d: U %d: serving a service of type 1\n", ++(*action_counter), EmployeeID);
            sem_post(mutex);
            usleep((rand() % 10)*1000);
            sem_wait(mutex);
            fprintf(file, "%d: U %d: service finished\n", ++(*action_counter), EmployeeID);
            sem_post(employee_done);
            sem_post(mutex);
        }
        else if(random == 2 && *queue_size2 > 0){
            sem_wait(mutex);
            sem_post(queue2);
            (*queue_size2)--;
            sem_post(mutex);
            sem_wait(queue2_done);
            sem_wait(mutex);
            fprintf(file, "%d: U %d: serving a service of type 2\n", ++(*action_counter), EmployeeID);
            sem_post(mutex);
            usleep((rand() % 10)*1000);
            sem_wait(mutex);
            fprintf(file, "%d: U %d: service finished\n", ++(*action_counter), EmployeeID);
            sem_post(employee_done);
            sem_post(mutex);
        }
        else if(random == 3 && *queue_size3 > 0){
            sem_wait(mutex);
            sem_post(queue3);
            
            (*queue_size3)--;
            sem_post(mutex);
            sem_wait(queue3_done);
            sem_wait(mutex);
            fprintf(file, "%d: U %d: serving a service of type 3\n", ++(*action_counter), EmployeeID);
            sem_post(mutex);
            usleep((rand() % 10)*1000);
            sem_wait(mutex);
            fprintf(file, "%d: U %d: service finished\n", ++(*action_counter), EmployeeID);
            sem_post(employee_done);
            sem_post(mutex);
        }
        else if( *post_office == 1 && *queue_size == 0 && *queue_size2 == 0 && *queue_size3 == 0){
            sem_wait(mutex);
            fprintf(file, "%d: U %d: going home\n", ++(*action_counter), EmployeeID);
            sem_post(mutex);
            break;
        }
        else if ( *post_office == 0 && *queue_size == 0 && *queue_size2 == 0 && *queue_size3 == 0){
            sem_wait(mutex);
            fprintf(file, "%d: U %d: taking break\n", ++(*action_counter), EmployeeID);
            sem_post(mutex);
            usleep((rand() % breaktime)*1000);
            fprintf(file, "%d: U %d: break finished\n", ++(*action_counter), EmployeeID);
        }
        if (*queue_size < 0 || *queue_size2 < 0 || *queue_size3 < 0){
            exit(EXIT_FAILURE);
        }
    }
}



int main(int argc, char *argv[]) {

    
    // checks if arguments are in correct format
    if (argcheck(argc, argv)){
        return EXIT_FAILURE;
    }
    int CustomerCount = atoi(argv[1]);
    int EmployeeCount = atoi(argv[2]);
    int CustomerMaxWait = atoi(argv[3]);
    int EmployeeMaxBreak = atoi(argv[4]);
    int MaxTime = atoi(argv[5]);

    startup();


    pid_t pid_post_office = fork();
    if (pid_post_office == 0){
        int closing_hours = rand() % MaxTime;
        usleep(closing_hours * 1000);
        fprintf(file, "%d: closing\n", ++(*action_counter));
        (*post_office) = 1;
        exit(EXIT_SUCCESS);
    }
    // check if fork failed
    else if (pid_post_office < 0){
        fprintf(stderr, "fork failed\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < EmployeeCount; i++){
        pid_t pid_e_id = fork();
        if (pid_e_id == 0){
            int breaktime = rand() % EmployeeMaxBreak;
            employee_process(i+1, breaktime);
            exit(EXIT_SUCCESS);
        }
        // check if fork failed
        else if (pid_e_id < 0){
            fprintf(stderr, "fork failed\n");
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < CustomerCount; i++){
        pid_t pid_c_id = fork();
        if (pid_c_id == 0){
            Customer customer;
            customer.CustomerID = i+1;
            customer.CustomerWaitTime = CustomerMaxWait;
            customer.CustomerDemand = rand() % 3;
            customer_process(customer);
            exit(EXIT_SUCCESS);
        }
        // check if fork failed
        else if (pid_c_id < 0){
            fprintf(stderr, "fork failed\n");
            return EXIT_FAILURE;
        }
    }
    while( wait(NULL) > 0);
    cleanup();
    return EXIT_SUCCESS;
}