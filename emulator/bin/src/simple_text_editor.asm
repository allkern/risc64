.equ _mmio_ioctl_base, 0x2000;
.equ _mmio_ioctl_term_char_out, 0x2000;
.equ _mmio_ioctl_term_status, 0x2001;
.equ _mmio_ioctl_keyb_code, 0x2005;
.equ _mmio_ioctl_keyb_status, 0x2006;
.equ _mmio_ioctl_keyb_key_ack, 0x80;
.equ _mmio_ioctl_term_ready, 0x4;

lrq %r0, _mmio_ioctl_keyb_code;
lrq %r1, _mmio_ioctl_keyb_status;
lrq %r2, _mmio_ioctl_term_char_out;
lrq %r3, _mmio_ioctl_term_status;

.loop
	# Wait for a keypress
	lb %r4, %r0;
	cmpb %r4, 0x0;

	# If not zero, there's been a keypress
	bz .loop;

	# Get keyb_status, or KEY_ACK (0x80)
	lb %r5, %r1;
	orb %r5, _mmio_ioctl_keyb_key_ack;

	# Load character into char_out
	sb %r4, %r2;

	# Get term_status, or READY, store it back
	lb %r6, %r3;
	orb %r6, _mmio_ioctl_term_ready;
	sb %r6, %r3;
	
	# Store keyb_status back, aka acknowledge that we've received
	# the keypress, and we don't need the code anymore
	sb %r5, %r1;
	
	# Clear %r4
	xorb %r4, %r4;

	# Branch back to loop unconditionally
	b .loop;

db 05 05 20 00 00 00 00 00 00
db 0d 06 20 00 00 00 00 00 00
db 15 00 20 00 00 00 00 00 00
db 1d 01 20 00 00 00 00 00 00
4b 00 20 00
63 0c 20 00
d0 00 c3 ff 7f
4b 00 28 01
63 08 28 80
4b 01 20 02
4b 00 30 03
63 08 30 04
4b 01 30 03
4b 01 28 01
43 09 20 04
d3 00 9b fe 7f
fb fe




