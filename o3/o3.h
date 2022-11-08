/* En rask måte å unngå header recursion på er å sjekke om verdi, f.eks. 'O3_H',
   er definert. Hvis ikke, definer 'O3_H' og deretter innholdet av headeren 
   (merk endif på bunnen). Nå kan headeren inkluderes så mange ganger vi vil 
   uten at det blir noen problemer. */
#ifndef O3_H
#define O3_H

// Type-definisjoner fra std-bibliotekene
#include <stdint.h>
#include <stdbool.h>

// Type-aliaser
typedef uint32_t word;
typedef uint8_t  byte;

// Prototyper for bibliotekfunksjoner
void init(void);
void lcd_write(char* string);
void int_to_string(char *timestamp, unsigned int offset, int i);
void time_to_string(char *timestamp, int h, int m, int s);

// Prototyper
void setup();
void start();
void stop();
void reset();
void increment_seconds();
void increment_minutes();
void increment_hours();
void decrement_seconds();

void set_LED(bool);
void update_display();

void GPIO_EVEN_IRQHandler();
void GPIO_ODD_IRQHandler();
void SysTick_Handler();

#endif
