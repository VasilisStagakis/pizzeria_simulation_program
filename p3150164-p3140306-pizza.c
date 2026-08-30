#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "p3150164-p3140306-pizza.h"

//macros initialization
int tel_avl = Ntel;
int cook_avl = Ncook;
int ovens_avl = Noven;
int packers_avl = 1;
int del_avl = Ndeliverer;

//locks
pthread_mutex_t tel_lock;
pthread_mutex_t cook_lock;
pthread_mutex_t oven_lock;
pthread_mutex_t packer_lock;
pthread_mutex_t deliverer_lock;
pthread_mutex_t screen_lock;
pthread_mutex_t statistics_lock;

//conditions
pthread_cond_t avl_tel_emp_cond;
pthread_cond_t avl_cook_emp_cond;
pthread_cond_t avl_ovens_cond;
pthread_cond_t avl_pack_emp_cond;
pthread_cond_t avl_del_emp_cond;

//thread id array
int* id;

//seed 
unsigned int seed;

//output variables
int balance = 0;
int num_s_orders = 0;
int num_f_orders = 0;
int time_on_hold = 0;
int max_time_on_hold = 0;
int time_till_delivery = 0;
int max_time_till_delivery = 0;
int cooling_time = 0;
int max_cooling_time = 0;



//PIZZERIA FUNCTION
void *Pizzeria(void* arg){
	//order id
	int oid = *(int *)arg;
    //each thread changes the seed
    seed += oid;
	//time structures
	struct timespec start_time;
	struct timespec while_on_hold;
	struct timespec time_till_packed;
	struct timespec time_till_delivered;
	struct timespec start_cooling;
	
	
	//CLIENT ON TELEPHONE
	
	
	//start time
	clock_gettime(CLOCK_REALTIME, &start_time);
	
	pthread_mutex_lock(&tel_lock);
	//in case of no available telephones
	while(tel_avl == 0){
		pthread_cond_wait(&avl_tel_emp_cond,&tel_lock);
	}
	tel_avl--;
	pthread_mutex_unlock(&tel_lock);
	//calculate client's time on hold
	pthread_mutex_lock(&statistics_lock);
	clock_gettime(CLOCK_REALTIME, &while_on_hold);
	int waiting = while_on_hold.tv_sec - start_time.tv_sec;
	time_on_hold += waiting;
	if(waiting > max_time_on_hold){
		printf("waiting\n");
		max_time_on_hold = waiting;
	}
	pthread_mutex_unlock(&statistics_lock);
	//order of pizzas
	int num_of_pizzas = (rand_r(&seed)%(Norderhigh-Norderlow+1))+Norderlow;
	//generating propability 1-100 of card to fail( 1-95 success , 96-100 fail )
	int card = (rand_r(&seed)%100)+1;
	//waiting for payment
	sleep((rand_r(&seed)%(Tpaymenthigh-Tpaymentlow+1))+Tpaymentlow);
	
	//payment successful
	if(card<=Pfail){
		pthread_mutex_lock(&screen_lock);
		printf("Η παραγγελία με αριθμό %d καταχωρήθηκε.\n",oid);
		pthread_mutex_unlock(&screen_lock);
		//update statistics
		pthread_mutex_lock(&statistics_lock);
		num_s_orders++;
		balance = balance + (num_of_pizzas*Cpizza);
		pthread_mutex_unlock(&statistics_lock);
		//telephone employee again available
		pthread_mutex_lock(&tel_lock);
		tel_avl++;
		pthread_mutex_unlock(&tel_lock);
		pthread_cond_signal(&avl_tel_emp_cond);
	}
	//payment failed
	else{
		pthread_mutex_lock(&screen_lock);
		printf("Η παραγγελία με αριθμό %d απέτυχε.\n",oid);
		pthread_mutex_unlock(&screen_lock);
		//update statistics
		pthread_mutex_lock(&statistics_lock);
		num_f_orders++;
		pthread_mutex_unlock(&statistics_lock);
		//telephone employee again available
		pthread_mutex_lock(&tel_lock);
		tel_avl++;
		pthread_mutex_unlock(&tel_lock);
		pthread_cond_signal(&avl_tel_emp_cond);
		//failed order thread exits the function
    	pthread_exit(NULL);
	}
	
	
	//ORDER IS BEING PREPARED AND BAKED
	
	
	//checking for available cook
	pthread_mutex_lock(&cook_lock);
	while(cook_avl == 0){
		pthread_cond_wait(&avl_cook_emp_cond,&cook_lock);
	}
	cook_avl--;
	pthread_mutex_unlock(&cook_lock);
	//cook prepares order
	sleep(num_of_pizzas*Tprep);
	
	//check if enough ovens are availabe
	pthread_mutex_lock(&oven_lock);
	while(num_of_pizzas>ovens_avl){
		pthread_cond_wait(&avl_ovens_cond,&oven_lock);
	}
	ovens_avl = ovens_avl - num_of_pizzas;
	pthread_mutex_unlock(&oven_lock);
    //cook again available
    pthread_mutex_lock(&cook_lock);
	cook_avl++;
    pthread_mutex_unlock(&cook_lock);
    pthread_cond_signal(&avl_cook_emp_cond);
    //the pizzas are baking
	sleep(Tbake);
	//pizzas started cooling
	clock_gettime(CLOCK_REALTIME, &start_cooling);
    
    
    //ORDER IS BEING PACKED
    
    
    //check for available packer
    pthread_mutex_lock(&packer_lock);
    while(packers_avl == 0){
    	pthread_cond_wait(&avl_pack_emp_cond,&packer_lock);
    }
    packers_avl--;
    pthread_mutex_unlock(&packer_lock);
    //order is getting packed
    sleep(Tpack);
    //ovens again available
	pthread_mutex_lock(&oven_lock);
	ovens_avl = ovens_avl + num_of_pizzas;
    pthread_mutex_unlock(&oven_lock);
    pthread_cond_signal(&avl_ovens_cond);
    //packer is again available
    pthread_mutex_lock(&packer_lock);
    packers_avl++;
    pthread_cond_signal(&avl_pack_emp_cond);
    pthread_mutex_unlock(&packer_lock);
    //calculate time to pack
    pthread_mutex_lock(&statistics_lock);
    clock_gettime(CLOCK_REALTIME, &time_till_packed);
    int packing = time_till_packed.tv_sec - start_time.tv_sec;
    //printing statistics
    printf("Η παραγγελία με αριθμό %d ετοιμάστηκε σε %d λεπτά.\n",oid,packing);
    pthread_mutex_unlock(&statistics_lock);
    
    
    //ORDER IS BEING DELIVERED
    
    
    //check for available deliverers
    pthread_mutex_lock(&deliverer_lock);
    while(del_avl == 0){
    	pthread_cond_wait(&avl_del_emp_cond,&deliverer_lock);
    }
    del_avl--;
    pthread_mutex_unlock(&deliverer_lock);
    
    //generate random time to deliver the order
    int delivery_time = rand_r(&seed)%(Tdelhigh-Tdellow+1)+Tdellow;
    //deliver the order
    sleep(delivery_time);
    //calculate time order waw cooling and total time spent untill order was delivered
    pthread_mutex_lock(&statistics_lock);
    clock_gettime(CLOCK_REALTIME,&time_till_delivered);
    int time_cooling = time_till_delivered.tv_sec - start_cooling.tv_sec;
    cooling_time += time_cooling;
    if(time_cooling > max_cooling_time){
    	max_cooling_time = time_cooling;
    }
    int deliver_time = time_till_delivered.tv_sec - start_time.tv_sec;
    time_till_delivery += deliver_time;
    if(deliver_time > max_time_till_delivery){
    	max_time_till_delivery = deliver_time;
    }
    //order delivered 
    printf("Η παραγγελία με αριθμό %d παραδόθηκε σε %d λεπτά.\n",oid,deliver_time);
    pthread_mutex_unlock(&statistics_lock);
    //deliverer returns to the pizzeria
    sleep(delivery_time);
    //deliverer again available
    pthread_mutex_lock(&deliverer_lock);
    del_avl++;
    pthread_cond_signal(&avl_del_emp_cond);
    pthread_mutex_unlock(&deliverer_lock);
    
    //successful order thread exits the routine
    pthread_exit(NULL);
}




//Main
int main(int argc , char* argv[]){
    
    //Number of arguments check
    if(argc != 3){
        printf("Please give 2 parameters to the program.\n");
        return -1;
    }
    
    //Number of Customers(using atoi to convert char* to int)
    int Ncust = atoi(argv[1]);
    //Seed for random number generating(using atoi to convert char* to int)
    seed = (unsigned int) atoi(argv[2]);
    
    //Program Parameters Check
    if(Ncust <= 0 ){
        printf("Invalid Parameters.\nNumber of Customers must be at least 1.\n");
        return -1;
    }
    
    //Parameters Successful
    printf("\n---------- Welcome to the Pizzeria! ----------\n\n");
    
    //Initializing Locks
    pthread_mutex_init(&tel_lock, NULL);
    pthread_mutex_init(&cook_lock, NULL);
    pthread_mutex_init(&oven_lock, NULL);
    pthread_mutex_init(&packer_lock, NULL);
    pthread_mutex_init(&deliverer_lock, NULL);
    pthread_mutex_init(&screen_lock, NULL);
    pthread_mutex_init(&statistics_lock, NULL);
    
    //Initializing Conditions
    pthread_cond_init(&avl_tel_emp_cond, NULL);
    pthread_cond_init(&avl_cook_emp_cond, NULL);
    pthread_cond_init(&avl_ovens_cond, NULL);
    pthread_cond_init(&avl_pack_emp_cond, NULL);
    pthread_cond_init(&avl_del_emp_cond, NULL);
    
	//Creating Thread Array and Threaad ids.
    pthread_t Customers[Ncust];
    int i;
    //allocating memory for id array
    id = malloc(Ncust * sizeof(int));
    for(i = 0; i < Ncust; i++){
    	id[i] = i + 1;
    	//random wait time untill next order
    	if(1){
    		sleep((rand_r(&seed)%(Torderhigh - Torderlow+1))+Torderlow);
    	}
    	//creating threads
        if(pthread_create(&Customers[i], NULL, &Pizzeria, &id[i]) != 0){
            printf("Failed to create thread.\n");
            return -1;
        }
    }
    
    //Joining Threads
    for(i = 0; i < Ncust; i++){
        if(pthread_join(Customers[i], NULL) != 0){
            printf("Failed to join threads.\n");
            return -1;
        }
    }
    
    //Destroying Locks
    pthread_mutex_destroy(&tel_lock);
    pthread_mutex_destroy(&cook_lock);
    pthread_mutex_destroy(&oven_lock);
    pthread_mutex_destroy(&packer_lock);
    pthread_mutex_destroy(&deliverer_lock);
    pthread_mutex_destroy(&screen_lock);
    pthread_mutex_destroy(&statistics_lock);
    
    //Destroying Conditions
    pthread_cond_destroy(&avl_tel_emp_cond);
    pthread_cond_destroy(&avl_cook_emp_cond);
    pthread_cond_destroy(&avl_ovens_cond);
    pthread_cond_destroy(&avl_pack_emp_cond);
    pthread_cond_destroy(&avl_del_emp_cond);
    
    //Printing Statistics
    printf("\n\nΣτατιστικά Αποτελέσματα\n\n");
    printf("Έσοδα: %d\nΕπιτυχημένες Παραγγελίες: %d\nΑποτυχημένες Παραγγελίες: %d\n",balance,num_s_orders,num_f_orders);
    printf("Μέσος χρόνος αναμονής πελατών: %d λεπτά.\nΜέγιστος χρόνος αναμονής πελατών: %d λεπτά.\n",(time_on_hold/Ncust),max_time_on_hold);
    printf("Μέσος χρόνος εξυπηρέτησης πελατών: %d λεπτά.\nΜέγιστος χρόνος εξυπηρέτησης πελατών: %d λεπτά.\n",(time_till_delivery/(Ncust-num_f_orders)),max_time_till_delivery);
    printf("Μέσος χρόνος κρυώματος παραγγελιών: %d λεπτά.\nΜέγιστος χρόνος κρυώματος παραγγελιών: %d λεπτά.\n",(cooling_time/(Ncust-num_f_orders)),max_cooling_time);
    printf("\n---------- Please Come Again! ----------\n\n");
    
    //freeing allocated memory
    free(id);
    return 0;
}
