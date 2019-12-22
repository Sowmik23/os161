#ifndef _PRIME_GEN_H_
#define _PRIME_GEN_H_
#define PRIMEGENERATOR 10 // Number of prime generator
#define STARTNUMBER 1000000 // initial number range
#define SIZE 10000 // each process the range
/**
 * Prime Number Generator -- Generates prime numbers between a range
 */
void prime_generator(void *,unsigned long);// start range, id
/**
 * Square Root Function -- integer value
 */
int floorSqrt(int);

#endif /* _PRIME_GEN_H_ */
