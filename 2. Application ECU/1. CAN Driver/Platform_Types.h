/*
 * Platform_Types.h
 *
 * Created: 9/5/2022 7:28:02 PM
 *  Author: Ahmed
 */ 


#ifndef PLATFORM_TYPES_H_
#define PLATFORM_TYPES_H_

#define CPU_TYPE_8        8
#define CPU_TYPE_16      16
#define CPU_TYPE_32      32

#define MSB_FIRST        0
#define LSB_FIRST        1

#define HIGH_BYTE_FIRST  0
#define LOW_BYTE_FIRST   1

/* Little endian, 32 bit processor */
#define CPU_TYPE			    CPU_TYPE_32
#define CPU_BIT_ORDER   	LSB_FIRST
#define CPU_BYTE_ORDER  	LOW_BYTE_FIRST

typedef unsigned char 		boolean;

#ifndef true
#define true  (boolean) 1
#endif

#ifndef false
#define false (boolean) 0
#endif


typedef signed char								sint8_t;
typedef unsigned char							uint8_t;
//typedef uint8_t									uint8;
typedef volatile signed char					vsint8_t;
typedef volatile unsigned char					vuint8_t;

typedef signed short							sint16_t;
typedef unsigned short							uint16_t;
//typedef uint16_t									uint16;
typedef volatile signed short					vsint16_t;
typedef volatile unsigned short					vuint16_t;

typedef signed int								sint32;
typedef unsigned int							uint32;
//typedef uint32_t									uint32;
typedef volatile signed int						vsint32_t;
typedef volatile unsigned int					vuint32_t;
//typedef vuint32_t									vuint32;

//typedef signed long int							sint32_t;
//typedef unsigned long int						uint32_t;
//typedef volatile signed long int				vsint32_t;
//typedef volatile unsigned long int				vuint32_t;
	
//typedef signed long long int					sint64_t;
//typedef unsigned long long int					uint64_t;
//typedef volatile signed long long int			vsint64_t;
//typedef volatile unsigned long long int			vuint64_t;

typedef float				    float32_t;
typedef double					float64_t;

#endif /* PLATFORM_TYPES_H_ */
