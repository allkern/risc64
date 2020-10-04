bw      #0x100;

lrw     %r31,   #0x2005;
lrw     %r30,   #0x2006;

lb      %r10,   %r31;
cmpb    %r10,   #0x0;
bz      #-8;

lb      %r31,   %r30;
orb     %r31,   #0x80;
sb      %r31,   %r30;
ret;
haltb;

lrw     %r31,   #0x2000;
lrw     %r30,   #0x2001;
sb      %r5,    %r31;

lb      %r31,   %r30;
orb     %r31,   #0x4;
sb      %r31,   %r30;
ret;
haltb;

lb      %r31,   %r5;
cmpb    %r31,   #0x0;
bbz     #25;
lb      %r30,   %r6;
cmpb    %r30,   %r31;
bbnz    #13;
id      %r5;
id      %r6;
bb      #-20;
cmpd    %r5,    %r6;
subdnzs %r10,   %r5,   %r6;
ret;
haltb;

lrd     %r0,    #0x10000;
lspd    #0x1ffff;
lr      %r5,    #0x3e;
call    #0x2c;
lr      %r5,    #0x20;
call    #0x2c;
call    #0x5;
lr      %r5,    %r10;
cmpb    %r5,    #0xa;
callwz  #0x200;

sb      %r5,    %r0;
ib      %r0;
ib      %r1;
call    #0x2c;
b       #-33;
haltb;

lrd     %r5,    %r0;
subd    %r5,    %r1;
lrw     %r6,    #0x300;
callb   #0x4a;
