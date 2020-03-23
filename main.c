#include "stm32f10x.h"
//https://www.st.com/content/ccc/resource/technical/document/reference_manual/59/b9/ba/7f/11/af/43/d5/CD00171190.pdf/files/CD00171190.pdf/jcr:content/translations/en.CD00171190.pdf

int rand(void);
void configure_gpio_pa5(void) ;
void configure_gpio_pc13(void) ;
void configure_afio_exti_pc13(void);
void set_gpio(GPIO_TypeDef *GPIO, int n) ;
void reset_gpio(GPIO_TypeDef *GPIO, int n) ;
void configure_timer(TIM_TypeDef *TIM, int psc, int arr) ;
void configure_it(void) ;
void start_timer(TIM_TypeDef *TIM) ;
void stop_timer(TIM_TypeDef *TIM) ;



int main(void){
	

	configure_gpio_pa5();
	configure_gpio_pc13();
	configure_afio_exti_pc13();
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN | RCC_APB1ENR_TIM3EN | RCC_APB1ENR_TIM4EN;
  configure_timer(TIM2,7199,2999); //300 ms
	configure_timer(TIM3,7199,10*rand()); //Random time 800ms-1800ms
	/* To calcul the TIM you have to use this frequency = TIM_CLK/(TIM_PSC+1)/(TIM_ARR + 1)
			TIM_CLK is the time desire
			PSC is fix at 7200
			And the frequency for the STM32F103 is 72Mhz so 72.10^6
	*/
	configure_timer(TIM4, 7199, 2499); // Blink every second in case of win 
	configure_it();
	start_timer(TIM3);

	while (1);
    
	return 0;
}


void configure_gpio_pa5(void){
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; //Power On the GPIOA's clock tree
	GPIOA->CRL = GPIOA->CRL & ~(0xF<<20);
	GPIOA ->CRL = GPIOA->CRL | (1<<20);
}


void configure_gpio_pc13(void) {
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; //Power On the GPIOC's clock tree
	GPIOC->CRH = GPIOC->CRH & ~(0xF<<20);
	GPIOC->CRH = GPIOC->CRH | (0x04<<20);
}


void set_gpio(GPIO_TypeDef *GPIO, int n) {
	GPIO->ODR = GPIO->ODR | (1<<n);
}

void reset_gpio(GPIO_TypeDef *GPIO, int n) {
	GPIO->ODR = GPIO->ODR & ~(1<<n);
}


void configure_timer(TIM_TypeDef *TIM, int psc, int arr) {
	
	TIM->ARR = arr;
	TIM-> PSC = psc;

}

void start_timer(TIM_TypeDef *TIM) {
	TIM->CR1=TIM->CR1 | (1<<0); //put 1 on the first bit of CR1 register (CEN) to start a timer
}

void stop_timer(TIM_TypeDef *TIM) {
	TIM->CR1=TIM->CR1 &~(1<<0); //put 0 on the first bit of CR1 register (CEN) to stop a timer
}


void configure_it(void) {
	NVIC->ISER[0] = NVIC->ISER[0] | (1 << 28); //Allow interuption in NVIC TIM2
	NVIC->ISER[0] = NVIC->ISER[0] | (1 << 29); //Allow interuption in NVIC TIM3
	TIM2->DIER = TIM2->DIER | (1 << 0); //Interupt if overflow
	TIM3->DIER = TIM3->DIER | (1 << 0); //Interupt if overflow
	EXTI->IMR = EXTI->IMR | (1<<13); //Allow the pin 13 to interupt, befor that use must configure it at an alternate func
	EXTI -> FTSR = EXTI->FTSR | (1<<13); // Falling trigger selection register, user button send falling signal
	NVIC->ISER[1] = NVIC->ISER[1]| (1 << 8); //Activate EXTI 15-10
	//All exti are multiplex, EXTI1 for pin1, EXTI13 for pin 13
	//p210-211

}
void configure_afio_exti_pc13(void) {
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
	AFIO->EXTICR[3] = AFIO->EXTICR[3]& ~(0x0F << 4); //reseting the alternate fonction for the port C
	AFIO->EXTICR[3] = AFIO->EXTICR[3]| (0x02 << 4); //Configuring the PC, see December 2018RM0008 Rev 201/1134 p183
}


void TIM2_IRQHandler(void){
	reset_gpio(GPIOA,5);
	stop_timer(TIM2);
	configure_timer(TIM3,7199,10*rand()); 
	start_timer(TIM3);
	TIM2->SR = TIM2->SR &~TIM_SR_UIF; //Put 0 on the uif bit of the timer
}
void TIM3_IRQHandler(void){
	set_gpio(GPIOA,5); //power on the led
	stop_timer(TIM3); // stop the timer
	start_timer(TIM2); //start the 300ms timer
	TIM3->SR = TIM3->SR &~TIM_SR_UIF; //Put 0 on the uif bit of the timer 
}
void TIM4_IRQHandler(void){
	if (GPIOA->ODR & (1<<5))
	{
		reset_gpio(GPIOA,5);
	}
	else
	{
		set_gpio(GPIOA,5);
	}
	TIM4->SR = TIM4->SR &~TIM_SR_UIF; //You have to reset to replay
}
void EXTI15_10_IRQHandler(void){
	if (GPIOA->ODR & (1<<5)){ 
		stop_timer(TIM2); 
		stop_timer(TIM3); // Stop all tim
		start_timer(TIM4); // Blinking victory
	}
	EXTI->PR = EXTI->PR | (1 << 13); // Validation
}
//TAKE ON OPENCLASSROOM
int rand(){
	static int randomseed = 0;
	randomseed = (randomseed * 9301 + 49297) % 233280;
	return 800 + (randomseed % 1800);
}

