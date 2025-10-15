#ifndef USART_H
#define USART_H

#ifdef __cplusplus
extern "C" {
#endif

/* prototypes */
void USART_Init();
void USART_Init_RXinterrupt();
void USART_Transmit_Char(uint8_t b);
void USART_Transmit_Line(const char* str);
uint8_t USART_Receive(void);

#ifdef __cplusplus
}
#endif

#endif
