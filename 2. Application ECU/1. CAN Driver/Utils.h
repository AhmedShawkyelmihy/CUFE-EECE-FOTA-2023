/*
 * Utils.h
 *
 * Created: 9/5/2022 7:42:01 PM
 *  Author: Ahmed
 */ 


#ifndef UTILS_H_
#define UTILS_H_

#define SET__BIT(reg, bit)		( reg |= ( 1 << (bit) ) )
#define CLEAR__BIT(reg, bit)		( reg &= ~( 1 << (bit) ) )
#define READ__BIT(reg, bit)		( ( reg >> (bit) ) & 1 )



#endif /* UTILS_H_ */
