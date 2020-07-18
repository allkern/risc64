.import "impl64_console.d",
        "impl64_math.d";

.sec: rodata:
    #.sb 0x10;
    #.sw 0x2000;
    #.sd 0x30000000;
    #.sq 0x4000000000000000;
    .my_string: .ss "Hello world!", 0;

.sec: static:
    .impl64_libutils_print:
        .str_wloop:
            lb      r1, *r0;
            cmpb    r1, 0x0;
            bz      .str_term;
            call    .impl64_write_char;         # impl64_write_char @ +0x000004cd
            add     r0, r0, 0x1;
            b       .str_wloop;

        .str_term:
            xorq    r0, r0, r0;

.sec: text:
    ld      r0, .my_string;
    call    .impl64_libutils_print;
    .pad (0x00feul): 0xa;