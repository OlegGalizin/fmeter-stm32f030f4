       AREA    |.text|, CODE, READONLY
uint128addpx PROC
      EXPORT  uint128addpx

        LDR      r3,[r1,#0]
        LDR      r2,[r0,#0]
        ADDS     r2,r3,r2
        STR      r2,[r0,#0]
        LDR      r3,[r1,#4]
        LDR      r2,[r0,#4]
        ADCS     r2,r3,r2
        STR      r2,[r0,#4]
        LDR      r3,[r1,#8]
        LDR      r2,[r0,#8]
        ADCS     r2,r3,r2
        STR      r2,[r0,#8]
        LDR      r2,[r1,#0xc]
        LDR      r1,[r0,#0xc]
        ADCS     r1,r2,r1
        STR      r1,[r0,#0xc]
        BX       lr
        ENDP

uint128add64x PROC
        LDR      r1,[r0,#0]
        ADDS     r1,r1,r2
        STR      r1,[r0,#0]

        LDR      r1,[r0,#4]
        ADCS     r1,r3
        STR      r1,[r0,#4]

        LDR      r3, =0
		LDR      r1,[r0,#8]
        ADCS     r1,r1,r3
        STR      r1,[r0,#8]

        LDR      r1,[r0,#0xc]
        ADCS     r1,r1,r3
        STR      r1,[r0,#0xc]
        BX       lr
        ENDP


		END