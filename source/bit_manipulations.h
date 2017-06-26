#ifndef BIT_MANIPULATIONS_H
#define BIT_MANIPULATIONS_H

#define BIT_SET(reg, n)  ((reg) |= (1<<n))
#define BIT_CLEAR(reg,n) ((reg) &= ~(1<<n))
#define BIT_FLIP(reg,n)  ((reg) ^= (1<<n))
#define BIT_READ(reg,n)  ((reg) & (1<<n))

#ifdef loop_until_bit_is_set
#undef loop_until_bit_is_set
#define loop_until_bit_is_set(reg,bit)   do { } while (!BIT_READ(reg, bit))
#endif

#ifdef loop_until_bit_is_clear
#undef loop_until_bit_is_clear
#define loop_until_bit_is_clear(reg,bit) do { } while (BIT_READ(reg, bit))
#endif

#endif