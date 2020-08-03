.org 0x0

.equ _PBUS_TCTRL_CSEND, 0x0000000000002000
.equ _PBUS_TCTRL_STATR, 0x0000000000002001
.equ _PBUS_TCTRL_READY, 0x04
.equ _PBUS_TCTRL_FMODE, 0x0c

.sec rodata:
    ._bios_awaiting_data: .ss "Please insert a boot floppy (0x000000ec: no_boot_media)", 13, 0
	._bios_loading_data: .ss "Loading boot image...", 13, 0
	
.sec text:
    lrq %r0, ._PBUS_TCTRL_CSEND;
    lrq %r1, ._s_awaiting_data;
    lrq %r4, ._PBUS_TCTRL_STATR;
    
    .l putchar:
        lb %r2, %r1, %r3;				## Load a char from the data string into %r2 <lsu_t_operand_register_all>
        sb %r2, %r0;					## Store %r2 into the TCTRL char port <lsu_d_operand_register_all>
        id %r3;						    ## Increment %r3 (offset to the next char in the string) <alu_s_operand_register_all>
        lb %r5, %r4;					## Load the TCTRL status register (STATR) into %r5 
        orb %r5, #_PBUS_TCTRL_READY;	## Set the READY bit
        sb %r5, %r4;					## Store %r5 back into STATR
        cmpb %r2, 0x0;					## Check if the current char is 0
        bnz .putchar;					## Branch while not zero to #.putchar
        halt;                           ## Halt the CPU