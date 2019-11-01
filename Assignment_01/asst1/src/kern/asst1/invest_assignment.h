/*
 * **********************************************************************
 *
 * Define function prototypes, types, and constants needed by both the
 * invest assignment (invest_assignment.c) and the code you need to write
 * (investor_producer.c)
 *
 * YOU SHOULD NOT RELY ON ANY CHANGES YOU MAKE TO THIS FILE
 *
 * We will use our own version of this file for testing
 */

#ifndef _INVEST_ASSIGNMENT_H_
#define _INVEST_ASSIGNMENT_H_

/*
 * Define the number of bank, producers, and customers available and their
 * symbolic constants. 
 * Note: in os161 -- currently you cannot use double, no coprocessor is available
 */
#define NBANK    3 		// number of bank
#define NPRODUCER   10 		// number of producer
#define NCUSTOMER 20 		// number of customer
#define BANK_INTEREST 5 	/* Bank interest in percent */
#define PRODUCT_PROFIT 10 	/* profit in percent -- must be added to the item price*/
#define BAC_CASH 100000000 	// Cash for investment
#define ITEM_PRICE 100 		// item unit price
#define REQUEST 0 		// order type request
#define SERVICED 1 		// order type served

/*
 * The maximum number of item
 */
#define N_ITEM_TYPE 100 	// Number of item types
#define MAX_ITEM_BUY 10 	// A customer can buy 1 to MAX_ITEM_BUY item at a time.

/*
 * The data type representing a bank account/financial information avaialble 
 */ 
struct bankdata {
  long int org_total_cash;	/* the orginal total cash for investment */
  long int remaining_cash;	/* the actual cash ready for investment */
  long int interest_amount;	/* Total Accumulated Interest Amount */
  long int acu_loan_amount;	/* Accumulated loan amount */
  long int prod_loan[NPRODUCER];/* Producer's current loan amount for this bank */
};


/*
 * Item ordered/served
 */
struct item{ 			// customer order/producer when serve/produce
    unsigned int item_quantity; // quantity intended to purchase by a customer; 1 to MAX_ITEM_BUY or amount of item produced
    unsigned int  i_price; 	// item-unit price; given as ITEM_PRICE
    unsigned long int servBy;	// producer id
    long int requestedBy; 	// customer id
    unsigned int order_type;	// REQUEST or SERVICE in the order queue
    struct item *next;		// Link to next order	
};

/**
 * Investor test, Customer and Producer funtion
 **/
void customer(void *, unsigned long);
void producer(void *, unsigned long);
void print_statistics(void);    // Testing tools 
/*
 * FUNCTION PROTOTYPES FOR THE FUNCTIONS YOU MUST WRITE
 *
 * YOU CANNOT MODIFY THESE PROTOTYPES
 *  
 */

/**
 * Customer functions
 * Customer places Order items in the order/service queue
 * Customer removes and consumed items from the order/service queue when served
 * At the time of consumption customer must keep track of his/her spending
 **/
extern void order_item(void *);		// order one or more items
extern void consume_item(unsigned long);	//consume items and pay bills for items (if any) in the service queue of a customer
extern void end_shoping(void); 		//finish shoping and go home


/* Producer functions */
extern void * take_order(void); 			//take order from the customer (order queue)
extern void produce_item(void *); 			// producer produce one or items
extern long int calculate_loan_amount(void *); 		//calculate the loan amount
extern void loan_request(void *,unsigned long); 	// producer request loan to one or banks if needed
extern void serve_order(void *,unsigned long);		// producer serve the requested items of a customer
extern void loan_reimburse(void *,unsigned long);	// return loan money with a service charge


/**
 *  Investor-Producer process opening and closing functions
 *  You may add your variables and tools at the begining of the
 *  shoping. Initilize for today's activities. 
 * */
extern void initialize(void);
/* finish the process; clean the system for the next day */
extern void finish(void);

#endif
