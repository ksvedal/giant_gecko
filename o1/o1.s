.thumb
.syntax unified

.include "gpio_constants.s"     // Register-adresser og konstanter for GPIO

.text
	.global Start
	
Start:
	// Led
	LDR R0, =GPIO_BASE
	LDR R1, =PORT_SIZE
	LDR R2, =LED_PORT

	MUL R1, R1, R2
	ADD R0, R0, R1

	// Address of led (on/off)
	ADD R1, R0, GPIO_PORT_DOUTCLR   // r1 to base + portsize * ledport + port_doutclr
	ADD R0, R0, GPIO_PORT_DOUTSET   // r0 to base + portsize * ledport + port_doutset

	// Button
	LDR R2, =GPIO_BASE
	LDR R3, =PORT_SIZE
	LDR R4, =BUTTON_PORT

	MUL R3, R3, R4
	ADD R2, R2, R3

	// Address of button
	ADD R2, R2, GPIO_PORT_DIN	// r2 to base + portsize * buttonport + port_din

	LDR R3, =#1				// Set R3 to led pin
	LSL R3, LED_PIN

	LDR R4, =#1				// Set R4 to button pin
	LSL R4, BUTTON_PIN

Loop:
	LDR R6, [R2]    	// Set R6 to the value of R2 (Button condition)
	AND R6, R6, R4  	// And with button pin
	CMP R6, R4      	// Sets condition flags by comparing R6 & R4
	BNE LedOn       	// branch to led on if not equal
	B LedOff
LedOn:
	STR R3, [R0]    	// led on (condifion of R0)
	B Loop          	// Branch back to loop
LedOff:
	STR R3, [R1]		// led off (condition of R1)
	B Loop				// Branch back to loop

NOP // Behold denne paa bunnen av fila