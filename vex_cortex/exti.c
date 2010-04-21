#include <hax.h>

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_misc.h"

/* PD0(10),PD1(11),2(X),3(X),4(X),5(X),
 * PC6(2),PC7(3),PE8(6),PE9(0),PE10(7),
 * PE11(1),PE12(8),PE13(4),PE14(5)
 */
/* Not used. (replace PC7(3))
 * PE7(9)
 */
static uint8_t exti_to_pin [16] =
	{ 10, 11, 0xFF, 0xFF, 0xFF, 0xFF,
	  2, 3, 6, 0, 7, 1, 8, 4, 5, 0xFF};

static InterruptServiceRoutine isr_callback[12];

#define __isr __attribute__((interrupt))

#define CALL_ISR(_i_)               \
	if (EXTI->PR & (1<<_i_)) {      \
		if (isr_callback[_i_]) {    \
			isr_callback[_i_](      \
				interrupt_get(      \
					exti_to_pin[_i_]\
					)               \
				);                  \
		}                           \
		EXTI->PR &= ~(1<<_i_);      \
	}

__isr void EXTI0_IRQHandler(void) {
	CALL_ISR(0);
}

__isr void EXTI1_IRQHandler(void) {
	CALL_ISR(1);
}

__isr void EXTI9_5_IRQHandler(void) {
	// CALL_ISR(5); // not connected
	CALL_ISR(6);
	CALL_ISR(7);
	CALL_ISR(8);
	CALL_ISR(9);
}

__isr void EXTI15_10_IRQHandler(void) {
	CALL_ISR(10);
	CALL_ISR(11);
	CALL_ISR(12);
	CALL_ISR(13);
	CALL_ISR(14);
	CALL_ISR(15);
}

	/* PE9, PE11,  PC6,  PC7, PE13, PE14,  PE8, PE10, PE12,  PE7,  PD0,  PD1*/
static const GPIO_TypeDef *gpio_ports[12] = 
	{GPIOE,GPIOE,GPIOC,GPIOC,GPIOE,GPIOE,GPIOE,GPIOE,GPIOE,GPIOE,GPIOD,GPIOD};
	
// Maps ifi labels to interrupt/pin indexes
static const int8_t   gpio_ifipin_to_pin[12] =
	{    9,   11,    6,    7,   13,   14,    8,   10,   12,    7,    0,    1};

bool digital_get(PinIx index) {
	uint8_t ret = GPIO_ReadInputDataBit(
		(GPIO_TypeDef*)gpio_ports[index],
		1 << gpio_ifipin_to_pin[index]);
		
	return ret != 0;
}


void pin_set_io(PinIx pin_index, PinMode pin_mode) {	
	GPIO_InitTypeDef GPIO_param;

	if (pin_index >= 12) {
		return;
	}
	
	GPIO_param.GPIO_Pin =
		(uint16_t)(1 << gpio_ifipin_to_pin[pin_index]);
	
	if (pin_mode == kInput) {
		GPIO_param.GPIO_Mode = GPIO_Mode_IPU;
	} else if (pin_mode == kOutput) {
		GPIO_param.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_param.GPIO_Mode = GPIO_Mode_Out_PP;
	}
	
	GPIO_Init((GPIO_TypeDef *)gpio_ports[pin_index],
		&GPIO_param);	
}

void interrupt_reg_isr(InterruptIx index,
	InterruptServiceRoutine isr) {
	isr_callback[gpio_ifipin_to_pin[index]] = isr;
}

bool interrupt_get(InterruptIx index) {
	return digital_get(index);
}

void interrupt_enable(InterruptIx index) {
	uint8_t ri = gpio_ifipin_to_pin[index];

	// unmask the interrupt.
	EXTI->IMR |= (1 << ri);

	// mask the event reqest.
	EXTI->EMR &= ~(1 << ri);
	
	// enable rising trigger and falling trigger.
	EXTI->RTSR |= (1 << ri);
	EXTI->FTSR |= (1 << ri);
}

void interrupt_disable(InterruptIx index) {
	EXTI->IMR &= ~(1 << gpio_ifipin_to_pin[index]);
}



void exti_init(void) {
	NVIC_InitTypeDef NVIC_param;

	/* PE9(0),PE11(1),PC6(2),PC7(3),PE13(4),PE14(5),
	 * PE8(6),PE10(7),PE12(8),PE7(9),PD0(10),PD1(11)
	 */
	AFIO->EXTICR[0] = 
		( AFIO_EXTICR1_EXTI0_PD // PD0 (10)
		| AFIO_EXTICR1_EXTI1_PD // PD1 (11)
		/* | AFIO_EXTCR1_EXTI2_Px */
		/* | AFIO_EXTCR1_EXTI3_Px */
		);
		
	/* Note: PE7(9) is not connected to
	 * EXTI interrupt due to hardware
	 * design limitations
	 */
	AFIO->EXTICR[1] = 
		( /* AFIO_EXTICR2_EXTI4_Px */
		/* | AFIO_EXTICR2_EXTI5_Px */
		  AFIO_EXTICR2_EXTI6_PC // PC6 (2)
		| AFIO_EXTICR2_EXTI7_PC // PC7 (3)
		);
		
	AFIO->EXTICR[2] =
		( AFIO_EXTICR3_EXTI8_PE // PE8 (6)
		| AFIO_EXTICR3_EXTI9_PE // PE9 (0)
		| AFIO_EXTICR3_EXTI10_PE // PE10 (7)
		| AFIO_EXTICR3_EXTI11_PE // PE11 (1)
		);
	
	AFIO->EXTICR[3] = 
		( AFIO_EXTICR4_EXTI12_PE // PE12 (8)
		| AFIO_EXTICR4_EXTI13_PE // PE13 (4)
		| AFIO_EXTICR4_EXTI14_PE // PE14 (5)
		/*| AFIO_EXTICR4_EXTI15_Px */
		);
	
	EXTI->IMR =
		( EXTI_IMR_MR0
		| EXTI_IMR_MR1
		| EXTI_IMR_MR6
		| EXTI_IMR_MR7
		| EXTI_IMR_MR8
		| EXTI_IMR_MR9
		| EXTI_IMR_MR10
		| EXTI_IMR_MR11
		| EXTI_IMR_MR12
		| EXTI_IMR_MR13
		| EXTI_IMR_MR14
		);
	
	
	
	/* Enable the EXTI0 Interrupt */
	NVIC_param.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_param.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_param.NVIC_IRQChannelSubPriority = 2;
	NVIC_param.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_param);

	/* Enable the EXTI1 Interrupt */
	NVIC_param.NVIC_IRQChannel = EXTI1_IRQn;
	NVIC_param.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_param.NVIC_IRQChannelSubPriority = 2;
	NVIC_param.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_param);

	/* Enable the EXTI9_5 Interrupt */
	NVIC_param.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_param.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_param.NVIC_IRQChannelSubPriority = 2;
	NVIC_param.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_param);

	/* Enable the EXTI15_10 Interrupt */
	NVIC_param.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_param.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_param.NVIC_IRQChannelSubPriority = 2;
	NVIC_param.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_param);
}