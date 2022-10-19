.thumb
.syntax unified

.include "gpio_constants.s"     // Register-adresser og konstanter for GPIO
.include "sys-tick_constants.s" // Register-adresser og konstanter for SysTick

.text
	.global Start
	
Start:

	// Systick constants
	LDR R0, = SYSTICK_BASE + SYSTICK_CTRL
	LDR R1, = SysTick_CTRL_TICKINT_Msk
    LDR R2, = SysTick_CTRL_CLKSOURCE_Msk
    LDR R3, = SYSTICK_BASE + SYSTICK_LOAD	// systick reload value register
    LDR R4, = SYSTICK_BASE + SYSTICK_VAL

	// Systick control:
    ORR R1, R1, R2
    STR R1, [R0]

	// Interrupt frequency
    LDR R1, = FREQUENCY / 10	// 10 interrupts pr second
    STR R1, [R3]
    STR R1, [R4]				// first interrupt

	// GPIO port select
	LDR R5, = GPIO_BASE + GPIO_EXTIPSELH

	LDR R1, =0b1111
	LSL R1, R1, #4
	MVN R1, R1
	LDR R2, [R5]
	AND R1, R1, R2

	LDR R1, = PORT_B
	LSL R1, R1, #4
	STR R1, [R5]

	// Enable falling edge interrupt trigger
	LDR R6, = GPIO_BASE + GPIO_EXTIFALL

	LDR R1, =1
	LSL R1, BUTTON_PIN
	LDR R2,[R6]
	ORR R2, R1, R2
	STR R2,[R6]

	// enable interrupt pin
	LDR R7, = GPIO_BASE + GPIO_IEN

	MOV R1, #1
	LSL R1, R1, BUTTON_PIN
	LDR R2, [R7]
	ORR R2, R2, R1
	STR R2, [R7]

	// Clears interrupt
	LDR R4, = GPIO_BASE + GPIO_IFC

	MOV R1, #1
	LSL R1, R1, BUTTON_PIN
	STR R1, [R4]

	// LED status
	LDR R2, = GPIO_BASE + (PORT_SIZE * LED_PORT) + GPIO_PORT_DOUTTGL

	MOV R3, #1
	LSL R3, R3, LED_PIN

	// Time-variables for the counter loop
	LDR R10, = tenths
	LDR R11, = seconds
	LDR R12, = minutes

Loop:									// loop
  	B Loop

.global SysTick_Handler
.thumb_func
SysTick_Handler:

	B increment_tenths

	BX LR // Back to loop

	increment_tenths:
		LDR R9, [R10]
		ADD R9, R9, #1
		STR R9, [R10]
		CMP R9, #10
		BEQ increment_seconds

		BX LR

	increment_seconds:
		// Toggle the led
		STR R3, [R2]

		PUSH {LR}
			BL reset_tenths
		POP {LR}
		LDR R9, [R11]
		ADD R9, R9, #1
		STR R9, [R11]
		CMP R9, #60
		BEQ increment_minutes

		BX LR

	increment_minutes:
		PUSH {LR}
			BL reset_seconds
			BL reset_tenths
		POP {LR}
		LDR R9, [R12]
		ADD R9, R9, #1
		STR R9, [R12]
		CMP R9, #99
		BEQ reset_minutes

		BX LR


    reset_minutes:
    	MOV R9, #0
        STR R9, [R12]

		BX LR

    reset_seconds:
    	MOV R9, #0
        STR R9, [R11]

		BX LR

    reset_tenths:
    	MOV R9, #0
        STR R9, [R10]

        BX LR

.global GPIO_ODD_IRQHandler
.thumb_func
GPIO_ODD_IRQHandler:

	// Start the clock
    LDR R1, = 0b1
    LDR R2, [R0]
    EOR R2, R1
    STR R2, [R0]

	// Clears interrupt
	MOV R1, #1
	LSL R1, R1, BUTTON_PIN
	STR R1, [R4]

	BX LR

NOP // Behold denne på bunnen av fila
