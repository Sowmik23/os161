#include <types.h>
#include <lib.h>
#include <synch.h>
#include <test.h>
#include <thread.h>

#include "investor_producer.h"
#include "invest_assignment.h"

#define TAKEN 2

static int stopped_customer=NCUSTOMER;
static int service_arr[NCUSTOMER];

/*
 * **********************************************************************
 * YOU ARE FREE TO CHANGE THIS FILE BELOW THIS POINT AS YOU SEE FIT
 *
 */

extern struct bankdata bank_account[NBANK];
extern long int customer_spending_amount[NCUSTOMER];
extern long int producer_income[NPRODUCER];
//extern int NET_PRICE;
static struct semaphore *mutex,*mutex2,*TO_full,*CLA,*WFP,*TO_empty,*CONSUME_ITEM_SEM[NCUSTOMER],*mutex3;
static struct semaphore *print;

//////////////////////////////////******************************************////////////////////
//SELF MADE FUNCTION





struct item *cur_head_taken;
//,*CLA
/*
 * **********************************************************************
 * FUNCTIONS EXECUTED BY CUSTOMER THREADS
 * **********************************************************************
 */


/*
 * order_item()
 *
 * Takes one argument specifying the item produces. The function
 * makes the item order available to producer threads and then blocks until the producers
 * have produced the item with that appropriate for the customers.
 *
 * The item itself contains the number of ordered items.
 */ 

void order_item(void *itm){
	(void)itm; // to avoid warning
    //panic("You need to write some code!!!!");
	int i; 
	P(mutex);

	struct item *temp1 = req_serv_item;
	struct item *temp2;
	int diff = N_ITEM_TYPE - 3*(N_ITEM_TYPE/3);
	while(temp1->next!= NULL){
		temp1 = temp1->next;
	}
	temp2 = itm;
	
	unsigned long glob_serve_by=temp2->servBy;
	long glob_requst_by = temp2->requestedBy;



	for (i = 0;i < 3; i++){


		
		struct item *head = (struct item*) kmalloc(sizeof(struct item));
		head->item_quantity=0;
		head->i_price = 0;

		for (int j = 0; j < N_ITEM_TYPE/3; j++)
		{
			head->item_quantity +=(temp2->item_quantity);
			head->i_price = ITEM_PRICE;

			temp2++;
		}
		
		if (i==2)
		{
			for (int j=0;j<diff;j++)
			{
				if (temp2 != NULL && temp2->requestedBy == glob_requst_by)
				{
					head->item_quantity +=(temp2->item_quantity);
					head->i_price = ITEM_PRICE;

					temp2++;
				}
			}
		}
		head->servBy = glob_serve_by;
		head->requestedBy =glob_requst_by ;
		head->order_type = REQUEST;
		head->next = NULL;
		
		
		temp1->next = head;
		temp1 = temp1->next;
		V(TO_full);
	}


	V(mutex);
}

/**
 * consume_item() 
 * Customer consume items which were served by the producers.
 * affected variables in the order queue, on item quantity, order type, requested by, served by
 * customer should keep records for his/her spending in shopping
 * and update spending account
 **/
void consume_item(unsigned long customernum){
    (void) customernum; // avoid warning 

	P(mutex);
	P(CONSUME_ITEM_SEM[customernum]);
	struct item *cur_head = req_serv_item;
	struct item *tmp;
	
	
	int tmp1=0,tmp2=0;
	if((cur_head)->order_type == SERVICED && (cur_head)->requestedBy == (long)customernum){
		cur_head->order_type = 5;
        
	}
	while(1){
		if(cur_head->next == NULL) {
			break;
		}
		if((cur_head->next)->order_type == SERVICED && (cur_head->next)->requestedBy == (long)customernum){
			customer_spending_amount[customernum] += ((cur_head->next)->item_quantity) * ((cur_head->next)->i_price);
			tmp1 += (cur_head->next)->i_price;
			tmp2 += (cur_head->next)->item_quantity;
			tmp = cur_head->next;
			cur_head->next = tmp->next;
			
		}
		else
			cur_head = cur_head->next;		
	}
	V(mutex);

	
}

/*
 * end_shoping()
 *
 * This function is called by customers when they go home. It could be
 * used to keep track of the number of remaining customers to allow
 * producer threads to exit when no customers remain.
 */
void end_shoping(){	
	P(CLA);
	stopped_customer--;
	V(CLA);
	if(stopped_customer==0){
		for (int i = 0; i < NPRODUCER; i++)
		{
			V(TO_full);
		}
		return;
		
	}
	V(TO_full);
}


/*
 * **********************************************************************
 * FUNCTIONS EXECUTED BY ITEM PRODUCER THREADS
 * **********************************************************************
 */

/*
 * take_order()
 *
 * This function waits for a new order to be submitted by
 * customers. When submitted, it records the details and returns a
 * pointer to something representing the order.
 *
 * The return pointer type is void * to allow freedom of representation
 * of orders.
 *
 * The function can return NULL to signal the producer thread it can now
 * exit as there are no customers nor orders left.
 */
 
void *take_order(){
	P(TO_full) ;
	P(TO_empty);
    struct item *special;
	while(1){
		if(stopped_customer == 0)
			break;
		if(cur_head_taken == NULL) {
			if(stopped_customer == 0){
				break;
			}
			else{
				cur_head_taken = req_serv_item;
			}
		}
		if(cur_head_taken->order_type == REQUEST){
			cur_head_taken->order_type = TAKEN;
            special = cur_head_taken;
            cur_head_taken = cur_head_taken->next;
			V(TO_empty);
			return special;
		}
		else
			cur_head_taken = cur_head_taken->next;
		
	}
	V(TO_empty);
return NULL;
}


/*
 * produce_item()
 *
 * This function produce an item if the investment is available for the product
 *
 */

void produce_item(void *v){
	(void)v;
	struct item *head = v;
		head->i_price = ITEM_PRICE + (ITEM_PRICE*(PRODUCT_PROFIT+BANK_INTEREST))/100;
        head->order_type = 15;
}


/*
 * serve_order()
 *
 * Takes a produced item and makes it available to the waiting customer.
 */

void serve_order(void *v,unsigned long producernumber){
	(void)v;
	(void)producernumber;
	struct item *head = v;
		P(mutex3);
        if(head->order_type!=SERVICED){
		service_arr[head->requestedBy]++;
		head->servBy = producernumber ;
		head->order_type = SERVICED;

		if(service_arr[head->requestedBy]==3){
            service_arr[head->requestedBy] = 0;
			V(CONSUME_ITEM_SEM[head->requestedBy]);
		}
        }
		
		V(mutex3);	
}

/**
 * calculate_loan_amount()
 * Calculate loan amount
 */
long int calculate_loan_amount(void* itm){
	(void)itm;
	struct item *head = itm;
    return ITEM_PRICE*head->item_quantity;
}

/**
 * void loan_request()
 * Request for loan from bank
 */
void loan_request(void *amount,unsigned long producernumber){
	(void)amount;
	(void)producernumber;
    long int money = *(long int *) amount;
	int bank = random()%NBANK;
	P(WFP);
	bank_account[bank].remaining_cash -= money;
	bank_account[bank].acu_loan_amount += money;
	bank_account[bank].prod_loan[producernumber] += money;
	V(WFP);
}

/**
 * loan_reimburse()
 * Return loan amount and service charge
 */
void loan_reimburse(void * loan,unsigned long producernumber){
	(void)loan;
	(void)producernumber;
    long int repay = *(long int *) loan;
	long int interest = (repay/100)*BANK_INTEREST;
	int i;
	P(WFP);
	for(i=0; i<NBANK ; i++){
		if(bank_account[i].prod_loan[producernumber] !=0){
			bank_account[i].remaining_cash += repay + interest;
			bank_account[i].interest_amount += interest;
			bank_account[i].prod_loan[producernumber] -= repay;
			break;
		}
	}
    producer_income[producernumber] += (repay/100) * PRODUCT_PROFIT;
	V(WFP);
}

/*
 * **********************************************************************
 * INITIALISATION AND CLEANUP FUNCTIONS
 * **********************************************************************
 */


/*
 * initialize()
 *
 * Perform any initialization you need before opening the investor-producer process to
 * producers and customers
 */

void initialize(){

	memset(customer_spending_amount,0,NCUSTOMER*sizeof(long int));
	memset(producer_income,0,NPRODUCER*sizeof(long int));
	memset(service_arr,0,NCUSTOMER*sizeof(int));
	mutex = sem_create("mutex", 1);
	mutex2 = sem_create("mutex2",1);
	TO_full = sem_create("TO_full",0);
	TO_empty = sem_create("TO_empty",1);
	CLA = sem_create("CLA",1);
	WFP = sem_create("WFP",1);   //Bank Lock korte use korchi
	mutex3 = sem_create("mutex3",1);
	print = sem_create("print",1);
	for (int i = 0; i < NCUSTOMER; i++)
	{
		CONSUME_ITEM_SEM[i] =sem_create("CONSUME_ITEM_SEM",0); 
	}
	req_serv_item = (struct item*) kmalloc(sizeof(struct item));
	req_serv_item->item_quantity = 0;
	req_serv_item-> i_price = 0;
	req_serv_item-> servBy = 1000;
	req_serv_item-> requestedBy = 1000;
	req_serv_item->order_type = 10;
	req_serv_item->next = NULL;
	cur_head_taken = req_serv_item;
}

/*
 * finish()
 *
 * Perform any cleanup investor-producer process after the finish everything and everybody
 * has gone home.
 */

void finish(){
	sem_destroy(mutex);
	sem_destroy(mutex2);
	sem_destroy(TO_full);
	sem_destroy(TO_empty);
	sem_destroy(CLA);
	sem_destroy(WFP);
	sem_destroy(print);
	for (int i = 0; i < NCUSTOMER; i++)
	{
		sem_destroy(CONSUME_ITEM_SEM[i]); 
	}
	kfree(req_serv_item);
	kprintf("checksssss");
	
}
