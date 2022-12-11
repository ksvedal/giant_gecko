#include "o3.h"
#include "gpio.h"
#include "systick.h"

// Define ports for buttons and led
#define LED_PORT GPIO_PORT_E
#define PB_PORT GPIO_PORT_B
#define LED_PIN 2
#define PB0_PIN 9
#define PB1_PIN 10

enum {setting_seconds, setting_minutes, setting_hours, counting_down, alarming} state;

// Time variables and state
int seconds = 0;
int minutes = 0;
int hours = 0;

static char str[8] = "0000000\0";

struct gpio_port_t {
  word CTRL, MODEL, MODEH, DOUT, DOUTSET, DOUTCLR, DOUTTGL, DOUTIN, PINLOCKN;
};

struct gpio_t {
  struct gpio_port_t ports[6];
  word __unused[10], EXTIPSELL, EXTIPSELH, EXTIRISE, EXTIFALL, IEN, IF, IFS, IFC, ROUTE, INSENSE, LOCK,
  CTRL, CMD, EM4WUEN, EM4WUPOL, EM4WUCAUSE;
} *GPIO = (struct gpio_t*)GPIO_BASE;

struct systick_t {
  word CTRL, LOAD, VAL, CALIB;
} *SYSTICK = (struct systick_t*)SYSTICK_BASE;

int main(void) {
    init();
    setup();
    reset();

    while (true);
}

/**************************************************************************//**
 * @brief Konverterer nummer til string
 * Konverterer et nummer mellom 0 og 99 til string
 *****************************************************************************/
void int_to_string(char *timestamp, unsigned int offset, int i) {
    if (i > 99) {
        timestamp[offset]   = '9';
        timestamp[offset+1] = '9';
        return;
    }

    while (i > 0) {
        if (i >= 10) {
            i -= 10;
            timestamp[offset]++;

        } else {
            timestamp[offset+1] = '0' + i;
            i=0;
        }
    }
}

/**************************************************************************//**
 * @brief Konverterer 3 tall til en timestamp-string
 * timestamp-argumentet må være et array med plass til (minst) 7 elementer.
 * Det kan deklareres i funksjonen som kaller som "char timestamp[7];"
 * Kallet blir dermed:
 * char timestamp[7];
 * time_to_string(timestamp, h, m, s);
 *****************************************************************************/
void time_to_string(char *timestamp, int h, int m, int s) {
    timestamp[0] = '0';
    timestamp[1] = '0';
    timestamp[2] = '0';
    timestamp[3] = '0';
    timestamp[4] = '0';
    timestamp[5] = '0';
    timestamp[6] = '\0';

    int_to_string(timestamp, 0, h);
    int_to_string(timestamp, 2, m);
    int_to_string(timestamp, 4, s);
}

/**
 * Sets let to on or off.
 * @param on Decides if the led should be on or off.
 */
void set_LED(bool on){
  if (on)
    GPIO->ports[LED_PORT].DOUTSET = 1 << LED_PIN;
  else
    GPIO->ports[LED_PORT].DOUTCLR = 1 << LED_PIN;
}

/**
 * Updates the display to reflect the time values in memory.
 */
void update_display(){
  time_to_string(str, hours, minutes, seconds);
  lcd_write(str);
}

/**
 * Set up the IO and Systick.
 */
void setup(){
    // Set LED, PB1, PB2 IO mode
    GPIO->ports[LED_PORT].MODEL = ((~(0b1111<<LED_PIN*4))&GPIO->ports[LED_PORT].MODEL)|(GPIO_MODE_OUTPUT<<LED_PIN*4);
    GPIO->ports[PB_PORT].MODEH = ((~(0b1111<<4))&GPIO->ports[PB_PORT].MODEH)|(GPIO_MODE_INPUT<<4);
    GPIO->ports[PB_PORT].MODEH = ((~(0b1111<<8))&GPIO->ports[PB_PORT].MODEH)|(GPIO_MODE_INPUT<<8);

    // Set select port B PB0_PIN/PB1_PIN
    GPIO->EXTIPSELH = ((~(0b1111<<4))&GPIO->EXTIPSELH)|(0b0001<<4);
    GPIO->EXTIPSELH = ((~(0b1111<<8))&GPIO->EXTIPSELH)|(0b0001<<8);

    // Falling edge trigger on PB0_PIN/PB1_PIN
    GPIO->EXTIFALL |= 1 << PB0_PIN;
    GPIO->EXTIFALL |= 1 << PB1_PIN;

    // Enable interrupts on PB0_PIN/PB1_PIN
    GPIO->IEN |= 1 << PB0_PIN;
    GPIO->IEN |= 1 << PB1_PIN;

    // Systick :O
    SYSTICK->LOAD = FREQUENCY;
    SYSTICK->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk;
}

/**
 * Start the clock
 */
void start() {
    SYSTICK->VAL = SYSTICK->LOAD;
    SYSTICK->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

/**
 * Stop the clock
 */
void stop() {
    SYSTICK->CTRL &= ~(SysTick_CTRL_ENABLE_Msk);
}

/**
 * Reset the display and led.
 */
void reset() {
    seconds = 0;
    minutes = 0;
    hours = 0;
    state = setting_seconds;
    update_display();
    set_LED(0);
}

/**
 * Increment seconds.
 * Sets to 0 and calls increment_minutes if reaches 60.
 */
void increment_seconds() {
    seconds++;

    if (seconds >= 60){
        seconds = 0;
        increment_minutes();
    }
}

/**
 * Increment minutes.
 * Sets to 0 and calls increment_hours if reaches 60.
 */
void increment_minutes() {
    minutes++;

    if (minutes >= 60) {
        minutes = 0;
        increment_hours();
    }
}

/**
 * Increment hours.
 */
void increment_hours() {
    hours++;
}

/**
 * Decrement seconds.
 * Moves on to minutes then hours when timer reaches 0.
 * Unlike the increment functions, this dont have to be separated because there is no individual controls.
 */
void decrement_seconds() {

    if (seconds <= 0) {

        if (minutes <= 0) {

            if (hours <= 0) {
                state = alarming;
                set_LED(1);
                return;
            }

            hours--;
            minutes = 60;
        }

        minutes--;
        seconds = 60;
    }

    seconds--;
}

/**
 * Pressing the button PB0.
 */
void GPIO_ODD_IRQHandler(){

    switch (state) {

        case setting_seconds: {
            increment_seconds();
            update_display();
        } break;

        case setting_minutes: {
            increment_minutes();
            update_display();
        } break;

        case setting_hours: {
            increment_hours();
            update_display();
        } break;

        case counting_down: {
            reset();
        } break;

        case alarming: {
            reset();
        } break;
    }

    GPIO->IFC = 1 << PB0_PIN;
}

/**
 * Pressing the button PB1.
 */
void GPIO_EVEN_IRQHandler(){

    switch (state) {

        case setting_seconds: {
            state = setting_minutes;
        } break;

        case setting_minutes: {
            state = setting_hours;
        } break;

        case setting_hours: {
            state = counting_down;
            start();
        } break;

        case counting_down: {
            reset();
        } break;

        case alarming: {
            reset();
        } break;

    }
    GPIO->IFC = 1 << PB1_PIN;
}

/**
 * The Systick handler :O
 */
void SysTick_Handler(){
    if (state == counting_down) {
        decrement_seconds();
        update_display();
    } else {
        stop();
    }
}
