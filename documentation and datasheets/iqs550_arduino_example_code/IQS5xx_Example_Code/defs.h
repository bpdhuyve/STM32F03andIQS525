#ifndef defs_h
#define defs_h

#define IQS5xx_ADDR          	0x74
#define RDY_PIN              	2
#define	END_WINDOW				(uint16_t)0xEEEE

#define BitIsSet(VAR,Index)		(VAR & (1<<Index)) != 0
#define BitIsNotSet(VAR,Index)	(VAR & (1<<Index)) == 0

#define SetBit(VAR,Index)		VAR |= (1<<Index)
#define ClearBit(VAR,Index)		VAR &= (uint8_t)(~(1<<Index))

#define FALSE 					0
#define	TRUE 					!FALSE



void Process_XY();
void IQS5xx_AcknowledgeReset(); 
void IQS5xx_CheckVersion();

#endif