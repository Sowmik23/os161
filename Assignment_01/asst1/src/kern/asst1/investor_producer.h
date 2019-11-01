/*
 * **********************************************************************
 * You are free to add or modify anything you think you require to this file
 */

#ifndef _INVESTOR_PRODUCER_H_
#define _INVESTOR_PRODUCER_H_

/**
 * FUNCTIONs Prototype declares here
 * Add or modify more funtion if you need more
 **/
extern struct item *req_serv_item;
//extern struct bankdata bank_account[NBANK];
/*struct item{ 			// customer order/producer when serve/produce
    unsigned int item_quantity; // quantity intended to purchase by a customer; 1 to MAX_ITEM_BUY or amount of item produced
    unsigned int  i_price; 	// item-unit price; given as ITEM_PRICE
    unsigned long int servBy;	// producer id
    long int requestedBy; 	// customer id
    unsigned int order_type;	// REQUEST or SERVICE in the order queue
    struct item *next;		// Link to next order	
};*/

void order_item(void *); 			// customer orders one or more items
void consume_item(unsigned long); 			// consume items and update spending
void end_shoping(void); 			//finish shoping and go home


/* Producer functions */
void *take_order(void); 			//take order from the customers
void produce_item(void *); 			// producer produce an item or product
long int calculate_loan_amount(void*); 		//calculate loan amount
void loan_request(void *,unsigned long); 			// producer request loan to a bank
void serve_order(void *,unsigned long); 			// producer serve the request of a customer
void loan_reimburse(void *,unsigned long); 			// return loan money with service charge


/* Investor-Producer process opening and closing functions */

void initialize(void);
void finish(void);

#endif

