#ifndef __ERROR_HANDLER_H
#define __ERROR_HANDLER_H


void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)

#endif /* __ERROR_HANDLER_H */
