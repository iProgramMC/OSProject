#ifndef _IDT_H
#define _IDT_H

typedef struct {
	unsigned short offset_lowerbits;
	unsigned short int selector;
	unsigned char zero;
	unsigned char type_attr;
	unsigned short offset_higherbits;
}
__attribute__((packed)) 
IdtEntry;

typedef struct
{
	uint32_t limit;
	uint32_t base;
}
__attribute__((packed))
IdtPointer;

extern void InitIDT();
extern void InitPIT();
extern void OnKeyboardInterrupt(void);
extern void OnTaskInterrupt(void);
extern void LoadIDT(unsigned long *idt_ptr);

#endif//_IDT_H