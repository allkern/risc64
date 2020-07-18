.sec rodata:
    ._PBUS_TCTRL_CSEND: #.sq 0x400000008bde01a8; ## pbus terminal controller character send address
    ._PBUS_TCTRL_STATR: #.sq 0x400000008bde01a9; ## pbus terminal controller character send address
    ._PBUS_TCTRL_READY: #.sb 0x04
	._PBUS_TCTRL_FMODE: #.sb 0x0c
    ._bios_awaiting_data: #.ss "Please insert a boot floppy (0x000000ec: no_boot_media)"
	._bios_loading_data: #.ss "Loading boot image..."
	

.sec text:
    lrq %r0, #_PBUS_TCTRL_CSEND;
    lrq %r4, #_PBUS_TCTRL_STATR;

    lrq %r1, #_s_awaiting_data;
    
    .l putchar:
        lh %r2, %r1, %r3;				## Load a char from the data string into %r2 <lsu_t_operand_register_all>
        sh %r2, %r0;					## Store %r2 into the TCTRL char port <lsu_d_operand_register_all>
        inc %r3;						## Increment %r3 (offset to the next char in the string) <alu_s_operand_register_all>
        lh %r5, %r4;					## Load the TCTRL status register (STATR) into %r5 
        or %r5, #_PBUS_TCTRL_READY;		## Set the READY bit
        sh %r5, %r4;					## Store %r5 back into STATR
        cmph %r2, 0x0;					## Check if the current char is 0
        bnz #.putchar;					## Branch while not zero to #.putchar

	.l loop:
		j #.loop;


farj 0xdeadbeefabcdabcd;