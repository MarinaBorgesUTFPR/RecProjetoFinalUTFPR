//=====================================================
// Data: 13/12/2021
// Autora: Marina Lourenço Borges
// Projeto: Projeto Caixa de Desinfeccao
//====================================================
// LED vermelho - PF1 - Lampada UV ligada
// LED azul - PF2 - Sinalização
// LED verde - PF3 - Sinalização
// Chave fim de curso SW2 - PF0
// Botão SW1 - PF4
// Relé - PB4

#include <stdio.h>
#include <stdint.h> // C99 variable types
#include "ADCSWTrigger.h"
#include "portb.h"
#include "portf.h"
#include "tm4c123gh6pm.h"
#include "SysTick.h"
#include "UART.h"
#include "Timer0.h"

void Output_Init(void);

unsigned char estado = 0; // variavel que representa o estado que está sendo executado
char inf_serial; // Informação recebida pelo serial
unsigned int flash_Red = 0;
unsigned int flash_All = 0;
unsigned long OutGreen;	 	// Visor para LED green
unsigned long OutBlue; 		// Visor para LED blue
unsigned long OutRed; 		// Visor para LED red
unsigned long Rele; 		// Visor para LED red
volatile uint32_t T1 = 0;




void Timer0A_Handler(void){
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;		// acknowledge TIMER0A timeout
	T1 = 0;
	TIMER0_ICR_R = 0x00000001;    			// clear TIMER0A timeout flag
	Timer0_off();
	GPIO_PORTB_DATA_R = 0x0; // Desliga o relé
}



int main(void){ 
	int32_t SW2, SW1; // Variavel para receber a informação da chave SW2 - PF0
  Output_Init(); // initialize output device
  PortF_Init();
	PortB_Init();
	SysTick_Init();							
	printf("Caixa de Desinfeccao\n");
	printf("By Marina Borges\n");

	
	
	while(1){				
		switch(estado){
			case(0):
				printf("Iniciando o processo\n");
				GPIO_PORTF_DATA_R = 0x0; // LEDs desligados
				GPIO_PORTB_DATA_R = 0x0; // Relé desligado
				
				OutBlue = GPIO_PORTF_DATA_R&0x04; //Visualization
				OutGreen = GPIO_PORTF_DATA_R&0x08; //Visualization
				OutRed = GPIO_PORTF_DATA_R&0x02; //Visualization
				Rele = GPIO_PORTB_DATA_R&0x10; // Indicar nivel do Relé
			
				SW2 = GPIO_PORTF_DATA_R & 0x01; // A variavel recebe a informação do pino PF0 por meio de um mascara
			
			
				if(SW2){ // Entra no if caso o valor seja diferente de 0, ou seja, SW2 = 0x01
						estado = 0;  	 // A caixa esta aberta
						printf("Estado 0: A caixa continua aberta!\n");
				} else {  
						estado = 1; // A caixa esta fechada
				}
				break;
			
	
			case(1):
				printf("Estado 1: Caixa fechada, inializando\n");
			
				GPIO_PORTF_DATA_R = 0x04; // Acende o LED azul
				OutBlue = GPIO_PORTF_DATA_R&0x04; //Visualization		
			
				SysTick_Wait10ms(100); // Função para esperar 1segundo
			
				GPIO_PORTF_DATA_R = 0x0; // Apaga o LED azul
				OutBlue = GPIO_PORTF_DATA_R&0x04; //Visualization
			
				SysTick_Wait10ms(100); // Função para esperar 1segundo
			
				estado = 2; // No próximo loop seguimos o programa
				break;
			
			
		
			case(2):
				SW1 = GPIO_PORTF_DATA_R & 0x10;
			
				if(SW1 == 0x0){		//Testa se o valor é zero, se sim o botão está pressionado vamos para o estado 4
					printf("Estado 2: SW1 pressionada\n");
					estado = 4; 
					
				} else{		// Caso o valor seja 1 o botão não está pressionado vamos para o estado 3
					printf("Estado 2: SW1 não pressionada\n");	
					estado = 3; 
				}
				break;
				
			case(3):
				GPIO_PORTF_DATA_R = 0x08;  // 0b0000_1000 LED acende Verde
				OutGreen = GPIO_PORTF_DATA_R&0x08; // Indicar nível no LED green
			
			  printf("Caixa Aberta: Retorna ao processo inicial em 4s.\n");
			
			  //Aguarda 4 segundos
			  SysTick_Wait10ms(100); // Função para esperar 1segundo
				printf("1\n");
				SysTick_Wait10ms(100); // Função para esperar 1segundo
				printf("2\n");
				SysTick_Wait10ms(100); // Função para esperar 1segundo
				printf("3\n");
				SysTick_Wait10ms(100); // Função para esperar 1segundo
				printf("4\n");
			
				GPIO_PORTF_DATA_R = 0x0; // Apaga o LED verde, retorno do case 3
			  OutGreen = GPIO_PORTF_DATA_R&0x0; //Visualization, retorno 3
			
				estado = 1;
		   	break;
			
			case(4):
				printf("Estado 4: Em operação, objeto será desinfectado.\n");
				
				GPIO_PORTF_DATA_R = 0x02;  // Inicializa o led Red
				OutRed = GPIO_PORTF_DATA_R&0x02; //Visualization
				
				GPIO_PORTB_DATA_R = 0x10; // Liga o relé
				Rele = GPIO_PORTB_DATA_R = 0x10; //Visualization
				
			//Timer 8 segundos
				T1 = 1 ;
				Timer0_Init(640000000); 	
				while(T1 != 0)
				{
					estado =  4;
				}			

				estado = 5; // Seguimos para o próximo passo da máquina de estados
			
				break;
			
			case(5):
				GPIO_PORTB_DATA_R = 0x0; // Desliga o relé
				Rele = GPIO_PORTB_DATA_R&0x10; // Indicar nivel do Relé
				GPIO_PORTF_DATA_R = 0x0;  // 0b0000_0000 LED 
				OutRed = GPIO_PORTF_DATA_R&0x02; // Indicar nível no LED red
			 
				printf("Estado 5: Finalizando a desinfectacao.\n");
				while (flash_Red < 6) 
				{
					GPIO_PORTF_DATA_R = 0x02; //Acende o Led Red
					OutRed = GPIO_PORTF_DATA_R&0x02; //Visualization
					SysTick_Wait10ms(50); //Função para esperar 0,5s
					
					GPIO_PORTF_DATA_R = 0x0; //Apaga o LED vermelho
					OutRed = GPIO_PORTF_DATA_R&0x02; //Visualization
					SysTick_Wait10ms(50); //Função para esperar 0,5s
					
					flash_Red = flash_Red + 1;
				}
				
				estado = 6; 
				break;
			
			case(6):
				SW1 = GPIO_PORTF_DATA_R & 0x10; // A variavel recebe a informação do pino PF4 por meio de um mascara
				
				if(SW1){ //Entra no if caso o valor seja diferente de 0, ou seja, SW1 = 0x10	
					printf("Estado 6: Caixa foi aberta!\n");
					estado = 1; // A caixa esta aberta novamente, portanto não podemos dar sequencia
					
				}else{   
					printf("Estado 6: Finalizando a operação\n");
					GPIO_PORTF_DATA_R =0x0C ;  // Liga os LEDs Azul e Verde
					OutBlue = GPIO_PORTF_DATA_R&0x04; //Visualization
					OutGreen = GPIO_PORTF_DATA_R&0x08; //Visualization
					SysTick_Wait10ms(60) ; 	//Espera 0,6 segund0s
					

					estado = 7;
				}
				break;
				
			case(7):
				GPIO_PORTF_DATA_R &= ~0x08 ;  // Apaga o LED Verde e mantem o azul ligado	
				OutGreen = GPIO_PORTF_DATA_R&0x08; //Visualization
				OutBlue = GPIO_PORTF_DATA_R&0x04; //Visualization
			
				SW1 = GPIO_PORTF_DATA_R & 0x10; // A variavel recebe a informação do pino PF0 por meio de um mascara
				
				if(SW1)
				{ 
					printf("Estado 7: Reinicializando o processo\n");
					estado = 2; // seguimos para o próximo estado 
					
				} else {
					printf("Estado 7: Finalizando\n");
					estado = 8; 
				}
				break;
			
			case(8):
				printf("Estado 8: Produto desinfectado!\n");
			
				while (flash_All < 4){
					GPIO_PORTF_DATA_R =0x0E ;  // Os 3 LED acendem
					OutBlue = GPIO_PORTF_DATA_R&0x04; //Visualization
					OutGreen = GPIO_PORTF_DATA_R&0x08; //Visualization
					OutRed = GPIO_PORTF_DATA_R&0x02; //Visualization
					SysTick_Wait10ms(100) ; 	//Espera 1 segundo
					
					GPIO_PORTF_DATA_R =0x00 ;  // Os 3 LED apagam
					OutBlue = GPIO_PORTF_DATA_R&0x04; //Visualization
					OutGreen = GPIO_PORTF_DATA_R&0x08; //Visualization
					OutRed = GPIO_PORTF_DATA_R&0x02; //Visualization
					SysTick_Wait10ms(100) ; 	//Espera 1 segundo
					
					flash_All = flash_All + 1;
				}
				estado = 0; //Reiniciamos a maquina
				break;
		}			
  }
}
