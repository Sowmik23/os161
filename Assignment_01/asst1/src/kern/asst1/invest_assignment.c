#include <types.h>
#include <lib.h>
#include <synch.h>
#include <test.h>
#include <thread.h>

#include "invest_assignment.h"


/*
 * DEFINE THIS MACRO TO SWITCH ON MORE PRINTING
 *
 * Note: Your solution should work whether printing is on or off
 *  
 */

/* #define PRINT_ON */

/* this semaphore is for cleaning up at the end. */
static struct semaphore *alldone;


/* request and service queue; order type will tell either it is a request or a service */
struct item *req_serv_item;
/* bank account that holds the information for loan and income */
struct bankdata bank_account[NBANK];
/* customers spending in shoping */
long int customer_spending_amount[NCUSTOMER];
/* this should contain producer income */
long int producer_income[NPRODUCER];

/*
 * **********************************************************************
 * CUSTOMERS
 *
 * Customers are rather simple, they request to buy items,
 * submit their orders as an order to the producer and wait.
 *
 * Eventually their order returns with the requested contents (exactly
 * as requested), they consume items, take a short
 * break, and do it all again until they have consumed all items according to the demand
 *
 */

void customer(void *unusedpointer, unsigned long customernum)
{
	struct item orderItem[N_ITEM_TYPE];
	int i,j;

	(void) unusedpointer; /* avoid compiler warning */

	customer_spending_amount[customernum]=0; //initialize customer spending
	i = 0; /* count number of interations */
	do {


#ifdef PRINT_ON
		kprintf("C %ld is ordering\n", customernum);
#endif

		//choose the even item first
		for (j = 0; j < N_ITEM_TYPE; j++) {
			orderItem[j].item_quantity=random()%MAX_ITEM_BUY+1;
			orderItem[j].requestedBy=customernum;
			orderItem[j].servBy=-1;
		}
		/* order the items, this blocks until the order is forfilled */
		order_item(&(orderItem));
#ifdef PRINT_ON
		kprintf("C %ld consuming items ordered with the following:\n", customernum);
#endif

		/**
		 *  Consume item: it sould be deleted (quantity=0) from request/service queue
		 *  consume items which were served by the producers
		**/
		consume_item(customernum);

		/* I needed that break.... */
		thread_yield();

		i++;
	} while (i < 10); /* keep going until .... */

#ifdef PRINT_ON  
	kprintf("C %ld end shoping\n", customernum);
#else
	(void)customernum;
#endif

	/*
	 * Now customer goes home.
	 */
	end_shoping();
	V(alldone);
}


/*
 * **********************************************************************
 * PRODUCER
 * producer are only slightly more complicated than the customers.
 * They take_orders, and if valid, they calculate loan, request loan, produce items,
 * determine item price and serve them.
 * When all the customers have left, the producer goes home.
 *
 * An invalid order signals that the producer should go home.
 *
 */

void producer(void *unusedpointer, unsigned long prod)
{

	void *o;
	int i;
	(void)unusedpointer; /* avoid compiler warning */

	i = 0; /* count orders served for stats */
	producer_income[prod]=0; // clean income at the beginning
	while (1) {

#ifdef PRINT_ON
		kprintf("P %ld taking order\n", prod);
#endif
		o = take_order();
		if (o != NULL) {
#ifdef PRINT_ON
			kprintf("S %ld calculating loan amount\n", prod);
#endif
			long int amount=calculate_loan_amount(o);
#ifdef PRINT_ON
			kprintf("S %ld request for loan\n", prod);
#endif
			loan_request(&amount,prod);
#ifdef PRINT_ON
			kprintf("S %ld producing item\n", prod);
#endif
			i++;
			produce_item(o);

#ifdef PRINT_ON
			kprintf("S %ld serving\n", prod);
#endif

			serve_order(o,prod);

			loan_reimburse(&amount,prod);
		}
		else {
			break;
		}

	};

	kprintf("S %ld going home after serving %d orders\n", prod, i);
	V(alldone);
}


/*
 * **********************************************************************
 * RUN THE Investor Producer
 * This routine sets up the invester producer prior to opening and cleans up
 * after closing.
 *
 * It calls two routines (initialize() and finish()) in
 * which you can insert your own initialisation code.
 *
 * It also prints some statistics at the end.
 *
 */

int runInvestorProducer(int nargs, char **args)
{
	int i, j, result;

	(void) nargs; /* avoid compiler warnings */
	(void) args;
	/* this semaphore indicates everybody has gone home */
	alldone = sem_create("alldone", 0);
	if (alldone==NULL) {
		panic("runInvestorProducer: out of memory\n");
	}
	/* initialize the bank account, interest, loan etc */
	for (i =0 ; i < NBANK; i++) {
		bank_account[i].interest_amount=0;
		bank_account[i].org_total_cash=BAC_CASH;
		bank_account[i].remaining_cash=BAC_CASH;
		bank_account[i].acu_loan_amount=0;
		for(j=0;j<NPRODUCER;j++){
			bank_account[i].prod_loan[j]=0;
		}
	}

	/***********************************************************************
	 * call your routine that initializes the rest of the Investor Producer
	 */
	initialize();

	/* Start the producer */
	for (i=0; i<NPRODUCER; i++) {
		result = thread_fork("Producer Thread", NULL,producer, (void*)NULL, i);
		if (result) {
			panic("runInvestorProducer: Producer Thread thread_fork failed: %s\n",
					strerror(result));
		}
	}

	/* Start the customers */
	for (i=0; i<NCUSTOMER; i++) {
		result = thread_fork("customer thread", NULL,customer, (void*)NULL,i);
		if (result) {
			panic("runInvestorProducer: customer thread_fork failed: %s\n",
					strerror(result));
		}
	}

	/* Wait for everybody to finish. */
	for (i=0; i< NCUSTOMER+NPRODUCER; i++) {
		P(alldone);
	}

	/***********************************************************************
	 * Call your Investor Producer clean up routine
	 * you can insert your code as needed to clean up
	 */
	finish();
	sem_destroy(alldone);
	//must print the statistics and validate -- need more codes
	/*for (i =0 ; i < NBANK; i++) {
			kprintf("Bank %d total  %ld income\n", i+1, bank_account[i].interest_amount);
	}*/
	print_statistics();
	kprintf("The InvestorProducer is closed, bye!!!\n");
	return 0;
}

/****
 * Test Suit -- Statistics
 *
 ***/
void print_statistics(void){
	int i;
	long int t_bank=0,t_producer=0,t_customer=0;
	kprintf("=====================================================================================\n");
	kprintf("Bank Income ... \n");
	for (i =0 ; i < NBANK; i++){
		kprintf("[%d]: %ld ",i,bank_account[i].interest_amount);
		t_bank+=bank_account[i].interest_amount+bank_account[i].acu_loan_amount;
	}
	kprintf("\n");
	kprintf("Producer Income ... \n");
	for (i =0 ; i < NPRODUCER; i++){
		kprintf("[%d]: %ld ",i,producer_income[i]);
		t_producer+=producer_income[i];
	}
	kprintf("\n");
	kprintf("Customer Spending ... \n");
	for (i =0 ; i < NCUSTOMER; i++){
		kprintf("[%d]: %ld ",i,customer_spending_amount[i]);
		t_customer+=customer_spending_amount[i];
	}
	kprintf("\n");
	kprintf("=====================================================================================\n");
	kprintf("Verification ....\n");
	long int t_in=0l;
	for(i=0;i<NBANK;i++){
		t_in=bank_account[i].acu_loan_amount*BANK_INTEREST/100;
		if(t_in!=bank_account[i].interest_amount){
			kprintf("[%d] calculated interest(t_in) == %ld interest ==  %ld\n", i,t_in,bank_account[i].interest_amount);
		}
	}
	if(t_customer!=t_bank+t_producer){
		kprintf("%ld <?> %ld\n",t_customer,(t_bank+t_producer));
	}

}
