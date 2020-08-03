.org 0;

.equ _mmio_ioctl_term_char_out,   0x2000;
.equ _mmio_ioctl_term_status,     0x2001;
.equ _mmio_ioctl_term_ready,      0x4;
.equ _mmio_ioctl_keyb_code,   0x2005;
.equ _mmio_ioctl_keyb_status,     0x2006;

# Function prototype:
# void _bios_putchar(char c);
# Registers:
# %r5 -> c
# Clobbers %r31, %r30

._bios_putchar:
    l %r31, _mmio_ioctl_term_char_out;
    l %r30, _mmio_ioctl_term_status;

    sb %r5, %r31;
    lb %r31, %r30;
    orb %r31, 0x4;
    sb %r31, %r0;

    ret;

# Function prototype:
# char _bios_getchar();
# Registers:
# %r10 -> return
# Clobbers %r31, %r30

._bios_getchar:
    l %r31, _mmio_ioctl_keyb_code;
    l %r30, _mmio_ioctl_keyb_status;

    .loop
        lb %r10, %r31;
        cmpb %r10, 0x0;
        bz .loop;

    lb %r31, %r30;
    orb %r31, 0x80;
    sb %r31, %r30;

    ret;

._bios_puts:
    .loop:
        push %r5;
        lb %r5, %r5;
        cmpb %r5, 0x0;
        bz +0xf;
        call _bios_putchar;
        pop %r5;
        d %r6;
        cmpq %r6, 

._bios_strcmp:
    .cmp_loop:
        lb %r31, %r5;
        lb %r30, %r6;

        cmpb %r31, %r30;
        bnz .dif;
        
        b .cmp_loop;

    .dif:
        lb %r10, 1;
        ret;


.loop:
    lrb %r5, '>';
    call _bios_putchar;
    lrb %r5, ' ';
    call _bios_putchar;
    lrq %r5, input_buffer;
    lrb %r6, 0xc;
    call _bios_gets;
