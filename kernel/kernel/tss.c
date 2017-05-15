#include <kernel/tty.h>
#include <kernel/gdt.h>

struct tss_p {
    uint16_t reserved1;
    uint16_t LINK;

    uint32_t ESP0;

    uint16_t reserved2;
    uint16_t SS0;
    
    uint32_t ESP1;

    uint16_t reserved3;
    uint16_t SS1;
    
    uint32_t ESP2;

    uint16_t reserved4;
    uint16_t SS2;

    uint32_t CR3;
    uint32_t EIP;
    uint32_t EFLAGS;
    uint32_t EAX;
    uint32_t ECX;
    uint32_t EDX;
    uint32_t EBX;

    uint32_t ESP;
    uint32_t EBP;
    uint32_t ESI;
    uint32_t EDI;

    uint16_t reserved5;
    uint16_t ES;
    uint16_t reserved6;
    uint16_t CS;
    uint16_t reserved7;
    uint16_t SS;
    uint16_t reserved8;
    uint16_t DS;
    uint16_t reserved9;
    uint16_t FS;
    uint16_t reserved10;
    uint16_t GS;
    uint16_t reserved11;
    uint16_t LDTR;

    uint16_t IOPBoffset;
    uint16_t reserved12;


} __attribute__ ((packed));

//http://wiki.osdev.org/Task_State_Segment#TSS_in_software_multitasking
//http://forum.osdev.org/viewtopic.php?t=13678
void tss_init(void) {
    MODULE_INFO tty_writestring("init start\n");
    struct tss_p tss;
    //GDT entry to store the tss
    set_gdt(5, (int)&tss, sizeof(tss), 0x89, 0x40);
    asm volatile("ltr %%ax": : "a" (0x28));
    MODULE_INFO tty_writestring("init finished\n");
}