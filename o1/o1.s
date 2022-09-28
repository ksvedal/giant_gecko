.thumb
.syntax unified

.include "gpio_constants.s"     // Register-adresser og konstanter for GPIO

.text
	.global Start
	
Start:
	//LED
	LDR R0, =GPIO_BASE ; Set R0 to General Purpose IO base port
	LDR R1, =PORT_SIZE ; 
	LDR R2, =LED_PORT
	MUL R1, R1, R2 				// Save in R1 times R2 in R1.
	ADD R0, R0, R1				// Save in R0 plus R1 in R0.

	ldr r1, =GPIO_PORT_DOUTCLR
	add r1, r0, r1      // r1 er base + portsize*ledport

	ldr r2, =GPIO_PORT_DOUTSET
	add r0, r0, r2      // r0 er base + portsize*ledport + port_doutset

	ldr r2, =GPIO_BASE
	ldr r3, =PORT_SIZE
	ldr r4, =BUTTON_PORT
	mul r3, r3, r4
	add r2, r2, r3
	ldr r5, =GPIO_PORT_DIN
	add r2, r2, r5	   //adressen til knappen

	ldr r3, =1         //setter r3 til ledpin output
	lsl r3, LED_PIN

	ldr r4, =1         //setter r4 til button pin output
	lsl r4, BUTTON_PIN

Loop:
	ldr r6, [r2]    //setter r6 lik verdien på r2, altså knappens tilstand
	and r6, r6, r4  //and-er med button output
	cmp r6, r4      //ser om r6 && r4 er lik r5
	bne LedOn       //branch if not equal
	str r3, [r1]    //led av
	b Loop          //tilbake til loop (idle loop)
LedOn:
	str r3, [r0]    //led på
	b Loop          //tilbake til loop

NOP // Behold denne paa bunnen av fila
