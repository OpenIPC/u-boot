
u-boot-hi3516cv608.elf:     file format elf32-littlearm


Disassembly of section .text:

41700000 <__image_copy_start>:
41700000:	ea000010 	b	41700048 <reset>
41700004:	eafffffe 	b	41700004 <__image_copy_start+0x4>
41700008:	eafffffe 	b	41700008 <__image_copy_start+0x8>
4170000c:	eafffffe 	b	4170000c <__image_copy_start+0xc>
41700010:	eafffffe 	b	41700010 <__image_copy_start+0x10>
41700014:	eafffffe 	b	41700014 <__image_copy_start+0x14>
41700018:	eafffffe 	b	41700018 <__image_copy_start+0x18>
4170001c:	eafffffe 	b	4170001c <__image_copy_start+0x1c>
41700020:	deadbeef 	.word	0xdeadbeef
41700024:	deadbeef 	.word	0xdeadbeef
41700028:	deadbeef 	.word	0xdeadbeef
4170002c:	deadbeef 	.word	0xdeadbeef
41700030:	deadbeef 	.word	0xdeadbeef
41700034:	deadbeef 	.word	0xdeadbeef
41700038:	deadbeef 	.word	0xdeadbeef
4170003c:	deadbeef 	.word	0xdeadbeef

41700040 <_TEXT_BASE>:
41700040:	41700000 	.word	0x41700000

41700044 <_start_armboot>:
41700044:	417006a8 	.word	0x417006a8

41700048 <reset>:
41700048:	ea00002d 	b	41700104 <save_boot_params>

4170004c <save_boot_params_ret>:
4170004c:	e10f0000 	mrs	r0, CPSR
41700050:	e200101f 	and	r1, r0, #31
41700054:	e331001a 	teq	r1, #26
41700058:	13c0001f 	bicne	r0, r0, #31
4170005c:	13800013 	orrne	r0, r0, #19
41700060:	e38000c0 	orr	r0, r0, #192	@ 0xc0
41700064:	e129f000 	msr	CPSR_fc, r0
41700068:	ee110f10 	mrc	15, 0, r0, cr1, cr0, {0}
4170006c:	e3c00a02 	bic	r0, r0, #8192	@ 0x2000
41700070:	ee010f10 	mcr	15, 0, r0, cr1, cr0, {0}
41700074:	e24f007c 	sub	r0, pc, #124	@ 0x7c
41700078:	e1a00000 	nop			@ (mov r0, r0)
4170007c:	ee0c0f10 	mcr	15, 0, r0, cr12, cr0, {0}
41700080:	e59fd0d4 	ldr	sp, [pc, #212]	@ 4170015c <cpu_init_cp15+0x54>

41700084 <print_startup>:
41700084:	eb000010 	bl	417000cc <msg_main_cpu_startup>
41700088:	e59f00d0 	ldr	r0, [pc, #208]	@ 41700160 <cpu_init_cp15+0x58>
4170008c:	e590f000 	ldr	pc, [r0]

41700090 <bug>:
41700090:	e320f000 	nop	{0}
41700094:	e320f000 	nop	{0}
41700098:	e320f000 	nop	{0}
4170009c:	e320f000 	nop	{0}
417000a0:	e320f000 	nop	{0}
417000a4:	e320f000 	nop	{0}
417000a8:	e320f000 	nop	{0}
417000ac:	e320f000 	nop	{0}
417000b0:	eafffffe 	b	417000b0 <bug+0x20>

417000b4 <memcpy>:
417000b4:	e0802002 	add	r2, r0, r2

417000b8 <memcpy_loop>:
417000b8:	e8b007f8 	ldm	r0!, {r3, r4, r5, r6, r7, r8, r9, sl}
417000bc:	e8a107f8 	stmia	r1!, {r3, r4, r5, r6, r7, r8, r9, sl}
417000c0:	e1500002 	cmp	r0, r2
417000c4:	dafffffb 	ble	417000b8 <memcpy_loop>
417000c8:	e1a0f00e 	mov	pc, lr

417000cc <msg_main_cpu_startup>:
417000cc:	e1a0500e 	mov	r5, lr
417000d0:	e28f0004 	add	r0, pc, #4
417000d4:	eb00002f 	bl	41700198 <uart_early_puts>
417000d8:	e1a0f005 	mov	pc, r5

417000dc <L10>:
417000dc:	0a0d0a0d 	.word	0x0a0d0a0d
417000e0:	74737953 	.word	0x74737953
417000e4:	73206d65 	.word	0x73206d65
417000e8:	74726174 	.word	0x74726174
417000ec:	0a0d7075 	.word	0x0a0d7075
417000f0:	00          	.byte	0x00
417000f1:	00          	.byte	0x00
	...

417000f4 <c_runtime_cpu_setup>:
417000f4:	ee070f15 	mcr	15, 0, r0, cr7, cr5, {0}
417000f8:	ee070f9a 	mcr	15, 0, r0, cr7, cr10, {4}
417000fc:	ee070f95 	mcr	15, 0, r0, cr7, cr5, {4}
41700100:	e12fff1e 	bx	lr

41700104 <save_boot_params>:
41700104:	eaffffd0 	b	4170004c <save_boot_params_ret>

41700108 <cpu_init_cp15>:
41700108:	e3a00000 	mov	r0, #0
4170010c:	ee080f17 	mcr	15, 0, r0, cr8, cr7, {0}
41700110:	ee070f15 	mcr	15, 0, r0, cr7, cr5, {0}
41700114:	ee070fd5 	mcr	15, 0, r0, cr7, cr5, {6}
41700118:	ee070f9a 	mcr	15, 0, r0, cr7, cr10, {4}
4170011c:	ee070f95 	mcr	15, 0, r0, cr7, cr5, {4}
41700120:	ee110f10 	mrc	15, 0, r0, cr1, cr0, {0}
41700124:	e3c00a02 	bic	r0, r0, #8192	@ 0x2000
41700128:	e3c00007 	bic	r0, r0, #7
4170012c:	e3800002 	orr	r0, r0, #2
41700130:	e3800b02 	orr	r0, r0, #2048	@ 0x800
41700134:	e3800a01 	orr	r0, r0, #4096	@ 0x1000
41700138:	ee010f10 	mcr	15, 0, r0, cr1, cr0, {0}
4170013c:	e1a0500e 	mov	r5, lr
41700140:	ee101f10 	mrc	15, 0, r1, cr0, cr0, {0}
41700144:	e1a03a21 	lsr	r3, r1, #20
41700148:	e203300f 	and	r3, r3, #15
4170014c:	e201400f 	and	r4, r1, #15
41700150:	e1a02203 	lsl	r2, r3, #4
41700154:	e1842002 	orr	r2, r4, r2
41700158:	e1a0f005 	mov	pc, r5
4170015c:	41700000 	.word	0x41700000
41700160:	41700044 	.word	0x41700044

41700164 <uart_early_init>:
41700164:	e59f3028 	ldr	r3, [pc, #40]	@ 41700194 <uart_base_addr_L0>
41700168:	e3a02000 	mov	r2, #0
4170016c:	e5832030 	str	r2, [r3, #48]	@ 0x30
41700170:	e282200d 	add	r2, r2, #13
41700174:	e5832024 	str	r2, [r3, #36]	@ 0x24
41700178:	e3a02001 	mov	r2, #1
4170017c:	e5832028 	str	r2, [r3, #40]	@ 0x28
41700180:	e3a02070 	mov	r2, #112	@ 0x70
41700184:	e583202c 	str	r2, [r3, #44]	@ 0x2c
41700188:	e59f2098 	ldr	r2, [pc, #152]	@ 41700228 <uart_base_addr_L3+0x4>
4170018c:	e5832030 	str	r2, [r3, #48]	@ 0x30
41700190:	e12fff1e 	bx	lr

41700194 <uart_base_addr_L0>:
41700194:	11040000 	.word	0x11040000

41700198 <uart_early_puts>:
41700198:	e59f1024 	ldr	r1, [pc, #36]	@ 417001c4 <uart_base_addr_L1>
4170019c:	ea000004 	b	417001b4 <next_char>

417001a0 <output>:
417001a0:	e5913018 	ldr	r3, [r1, #24]
417001a4:	e3130020 	tst	r3, #32
417001a8:	1afffffc 	bne	417001a0 <output>
417001ac:	e5812000 	str	r2, [r1]
417001b0:	e2800001 	add	r0, r0, #1

417001b4 <next_char>:
417001b4:	e5d02000 	ldrb	r2, [r0]
417001b8:	e3520000 	cmp	r2, #0
417001bc:	1afffff7 	bne	417001a0 <output>
417001c0:	e12fff1e 	bx	lr

417001c4 <uart_base_addr_L1>:
417001c4:	11040000 	.word	0x11040000

417001c8 <uart_early_put_hex>:
417001c8:	e59f1038 	ldr	r1, [pc, #56]	@ 41700208 <uart_base_addr_L2>
417001cc:	e3a0201c 	mov	r2, #28

417001d0 <wait2>:
417001d0:	e5913018 	ldr	r3, [r1, #24]
417001d4:	e3130020 	tst	r3, #32
417001d8:	1afffffc 	bne	417001d0 <wait2>
417001dc:	e3a0300f 	mov	r3, #15
417001e0:	e0033230 	and	r3, r3, r0, lsr r2
417001e4:	e3530009 	cmp	r3, #9
417001e8:	d2833030 	addle	r3, r3, #48	@ 0x30
417001ec:	c2833037 	addgt	r3, r3, #55	@ 0x37
417001f0:	e5813000 	str	r3, [r1]
417001f4:	e3520000 	cmp	r2, #0
417001f8:	0a000001 	beq	41700204 <exit2>
417001fc:	e2422004 	sub	r2, r2, #4
41700200:	eafffff2 	b	417001d0 <wait2>

41700204 <exit2>:
41700204:	e12fff1e 	bx	lr

41700208 <uart_base_addr_L2>:
41700208:	11040000 	.word	0x11040000

4170020c <uart_early_putc>:
4170020c:	e59f1010 	ldr	r1, [pc, #16]	@ 41700224 <uart_base_addr_L3>

41700210 <wait3>:
41700210:	e5913018 	ldr	r3, [r1, #24]
41700214:	e3130020 	tst	r3, #32
41700218:	1afffffc 	bne	41700210 <wait3>
4170021c:	e5810000 	str	r0, [r1]
41700220:	e12fff1e 	bx	lr

41700224 <uart_base_addr_L3>:
41700224:	11040000 	.word	0x11040000
41700228:	00000301 	.word	0x00000301

4170022c <hw_dec_sop_eop_first_set>:
4170022c:	e59f3030 	ldr	r3, [pc, #48]	@ 41700264 <hw_dec_sop_eop_first_set+0x38>
41700230:	e3a02001 	mov	r2, #1
41700234:	e5832000 	str	r2, [r3]
41700238:	e0402002 	sub	r2, r0, r2
4170023c:	e59f3024 	ldr	r3, [pc, #36]	@ 41700268 <hw_dec_sop_eop_first_set+0x3c>
41700240:	e16f2f12 	clz	r2, r2
41700244:	e1a022a2 	lsr	r2, r2, #5
41700248:	e5832000 	str	r2, [r3]
4170024c:	e3a02000 	mov	r2, #0
41700250:	e59f3014 	ldr	r3, [pc, #20]	@ 4170026c <hw_dec_sop_eop_first_set+0x40>
41700254:	e5832000 	str	r2, [r3]
41700258:	e59f3010 	ldr	r3, [pc, #16]	@ 41700270 <hw_dec_sop_eop_first_set+0x44>
4170025c:	e5830000 	str	r0, [r3]
41700260:	e12fff1e 	bx	lr
41700264:	417209a0 	.word	0x417209a0
41700268:	4172099c 	.word	0x4172099c
4170026c:	41720998 	.word	0x41720998
41700270:	41720994 	.word	0x41720994

41700274 <hw_dec_intr_proc>:
41700274:	e59f30a4 	ldr	r3, [pc, #164]	@ 41700320 <hw_dec_intr_proc+0xac>
41700278:	e5932124 	ldr	r2, [r3, #292]	@ 0x124
4170027c:	f57ff05f 	dmb	sy
41700280:	e6ef1072 	uxtb	r1, r2
41700284:	e3120002 	tst	r2, #2
41700288:	0a00000c 	beq	417002c0 <hw_dec_intr_proc+0x4c>
4170028c:	e59f2090 	ldr	r2, [pc, #144]	@ 41700324 <hw_dec_intr_proc+0xb0>
41700290:	e5920080 	ldr	r0, [r2, #128]	@ 0x80
41700294:	f57ff05f 	dmb	sy
41700298:	e3500000 	cmp	r0, #0
4170029c:	aa000002 	bge	417002ac <hw_dec_intr_proc+0x38>
417002a0:	f57ff05f 	dmb	sy
417002a4:	e3a00001 	mov	r0, #1
417002a8:	e5820090 	str	r0, [r2, #144]	@ 0x90
417002ac:	e5932130 	ldr	r2, [r3, #304]	@ 0x130
417002b0:	f57ff05f 	dmb	sy
417002b4:	e3822002 	orr	r2, r2, #2
417002b8:	f57ff05f 	dmb	sy
417002bc:	e5832130 	str	r2, [r3, #304]	@ 0x130
417002c0:	e3110001 	tst	r1, #1
417002c4:	0a000013 	beq	41700318 <hw_dec_intr_proc+0xa4>
417002c8:	e59f2054 	ldr	r2, [pc, #84]	@ 41700324 <hw_dec_intr_proc+0xb0>
417002cc:	e5923084 	ldr	r3, [r2, #132]	@ 0x84
417002d0:	f57ff05f 	dmb	sy
417002d4:	e3530000 	cmp	r3, #0
417002d8:	a3a00000 	movge	r0, #0
417002dc:	aa000006 	bge	417002fc <hw_dec_intr_proc+0x88>
417002e0:	e6ef3073 	uxtb	r3, r3
417002e4:	e3530000 	cmp	r3, #0
417002e8:	13e00001 	mvnne	r0, #1
417002ec:	03a00000 	moveq	r0, #0
417002f0:	f57ff05f 	dmb	sy
417002f4:	e3a03001 	mov	r3, #1
417002f8:	e5823094 	str	r3, [r2, #148]	@ 0x94
417002fc:	e59f201c 	ldr	r2, [pc, #28]	@ 41700320 <hw_dec_intr_proc+0xac>
41700300:	e5923130 	ldr	r3, [r2, #304]	@ 0x130
41700304:	f57ff05f 	dmb	sy
41700308:	e3833001 	orr	r3, r3, #1
4170030c:	f57ff05f 	dmb	sy
41700310:	e5823130 	str	r3, [r2, #304]	@ 0x130
41700314:	e12fff1e 	bx	lr
41700318:	e3e00000 	mvn	r0, #0
4170031c:	e12fff1e 	bx	lr
41700320:	170f0000 	.word	0x170f0000
41700324:	170f2000 	.word	0x170f2000

41700328 <hw_dec_start>:
41700328:	e92d4010 	push	{r4, lr}
4170032c:	e1a04002 	mov	r4, r2
41700330:	e59f2098 	ldr	r2, [pc, #152]	@ 417003d0 <hw_dec_start+0xa8>
41700334:	e59dc010 	ldr	ip, [sp, #16]
41700338:	e592e000 	ldr	lr, [r2]
4170033c:	e35e0000 	cmp	lr, #0
41700340:	0a000006 	beq	41700360 <hw_dec_start+0x38>
41700344:	e59fe088 	ldr	lr, [pc, #136]	@ 417003d4 <hw_dec_start+0xac>
41700348:	e35c0000 	cmp	ip, #0
4170034c:	1a000016 	bne	417003ac <hw_dec_start+0x84>
41700350:	f57ff05f 	dmb	sy
41700354:	e58e1020 	str	r1, [lr, #32]
41700358:	f57ff05f 	dmb	sy
4170035c:	e58e3024 	str	r3, [lr, #36]	@ 0x24
41700360:	f57ff05f 	dmb	sy
41700364:	e59f1068 	ldr	r1, [pc, #104]	@ 417003d4 <hw_dec_start+0xac>
41700368:	e59f3068 	ldr	r3, [pc, #104]	@ 417003d8 <hw_dec_start+0xb0>
4170036c:	e5810040 	str	r0, [r1, #64]	@ 0x40
41700370:	e5922000 	ldr	r2, [r2]
41700374:	e5933000 	ldr	r3, [r3]
41700378:	e1a02e82 	lsl	r2, r2, #29
4170037c:	e1822e03 	orr	r2, r2, r3, lsl #28
41700380:	e16f3f1c 	clz	r3, ip
41700384:	e1822004 	orr	r2, r2, r4
41700388:	e1a032a3 	lsr	r3, r3, #5
4170038c:	e1822f83 	orr	r2, r2, r3, lsl #31
41700390:	f57ff05f 	dmb	sy
41700394:	e5812044 	str	r2, [r1, #68]	@ 0x44
41700398:	f57ff05f 	dmb	sy
4170039c:	e59f3038 	ldr	r3, [pc, #56]	@ 417003dc <hw_dec_start+0xb4>
417003a0:	e59d2008 	ldr	r2, [sp, #8]
417003a4:	e5832000 	str	r2, [r3]
417003a8:	e8bd8010 	pop	{r4, pc}
417003ac:	f57ff05f 	dmb	sy
417003b0:	e2833eff 	add	r3, r3, #4080	@ 0xff0
417003b4:	e58e1028 	str	r1, [lr, #40]	@ 0x28
417003b8:	e283300f 	add	r3, r3, #15
417003bc:	e1a03623 	lsr	r3, r3, #12
417003c0:	e1a03103 	lsl	r3, r3, #2
417003c4:	f57ff05f 	dmb	sy
417003c8:	e58e302c 	str	r3, [lr, #44]	@ 0x2c
417003cc:	eaffffe3 	b	41700360 <hw_dec_start+0x38>
417003d0:	417209a0 	.word	0x417209a0
417003d4:	170f2000 	.word	0x170f2000
417003d8:	4172099c 	.word	0x4172099c
417003dc:	170f4000 	.word	0x170f4000

417003e0 <hw_dec_wait_finish>:
417003e0:	e92d4073 	push	{r0, r1, r4, r5, r6, lr}
417003e4:	e3a06000 	mov	r6, #0
417003e8:	e59f5058 	ldr	r5, [pc, #88]	@ 41700448 <hw_dec_wait_finish+0x68>
417003ec:	e3a01000 	mov	r1, #0
417003f0:	e3a00056 	mov	r0, #86	@ 0x56
417003f4:	ebffff9e 	bl	41700274 <hw_dec_intr_proc>
417003f8:	e2555001 	subs	r5, r5, #1
417003fc:	e1a04000 	mov	r4, r0
41700400:	1a000004 	bne	41700418 <hw_dec_wait_finish+0x38>
41700404:	e59f0040 	ldr	r0, [pc, #64]	@ 4170044c <hw_dec_wait_finish+0x6c>
41700408:	ebffff62 	bl	41700198 <uart_early_puts>
4170040c:	e1a00004 	mov	r0, r4
41700410:	e28dd008 	add	sp, sp, #8
41700414:	e8bd8070 	pop	{r4, r5, r6, pc}
41700418:	e58d6004 	str	r6, [sp, #4]
4170041c:	e59d3004 	ldr	r3, [sp, #4]
41700420:	e3530063 	cmp	r3, #99	@ 0x63
41700424:	9a000002 	bls	41700434 <hw_dec_wait_finish+0x54>
41700428:	e3740001 	cmn	r4, #1
4170042c:	0affffee 	beq	417003ec <hw_dec_wait_finish+0xc>
41700430:	eafffff5 	b	4170040c <hw_dec_wait_finish+0x2c>
41700434:	e320f000 	nop	{0}
41700438:	e59d3004 	ldr	r3, [sp, #4]
4170043c:	e2833001 	add	r3, r3, #1
41700440:	e58d3004 	str	r3, [sp, #4]
41700444:	eafffff4 	b	4170041c <hw_dec_wait_finish+0x3c>
41700448:	001e8481 	.word	0x001e8481
4170044c:	417208cc 	.word	0x417208cc

41700450 <hw_dec_decompress>:
41700450:	e92d407f 	push	{r0, r1, r2, r3, r4, r5, r6, lr}
41700454:	e1a04001 	mov	r4, r1
41700458:	e1a06000 	mov	r6, r0
4170045c:	e3a00001 	mov	r0, #1
41700460:	e1a05002 	mov	r5, r2
41700464:	e1a01003 	mov	r1, r3
41700468:	ebffff6f 	bl	4170022c <hw_dec_sop_eop_first_set>
4170046c:	e3540000 	cmp	r4, #0
41700470:	03e00000 	mvneq	r0, #0
41700474:	0a000011 	beq	417004c0 <hw_dec_decompress+0x70>
41700478:	e59f3048 	ldr	r3, [pc, #72]	@ 417004c8 <hw_dec_decompress+0x78>
4170047c:	e1a02001 	mov	r2, r1
41700480:	e1a01006 	mov	r1, r6
41700484:	e5933000 	ldr	r3, [r3]
41700488:	e58d3008 	str	r3, [sp, #8]
4170048c:	e3a03000 	mov	r3, #0
41700490:	e88d0009 	stm	sp, {r0, r3}
41700494:	e1a00005 	mov	r0, r5
41700498:	e5943000 	ldr	r3, [r4]
4170049c:	ebffffa1 	bl	41700328 <hw_dec_start>
417004a0:	ebffffce 	bl	417003e0 <hw_dec_wait_finish>
417004a4:	e59f3020 	ldr	r3, [pc, #32]	@ 417004cc <hw_dec_decompress+0x7c>
417004a8:	e5933088 	ldr	r3, [r3, #136]	@ 0x88
417004ac:	f57ff05f 	dmb	sy
417004b0:	e2500000 	subs	r0, r0, #0
417004b4:	e5843000 	str	r3, [r4]
417004b8:	13a00001 	movne	r0, #1
417004bc:	e2600000 	rsb	r0, r0, #0
417004c0:	e28dd010 	add	sp, sp, #16
417004c4:	e8bd8070 	pop	{r4, r5, r6, pc}
417004c8:	417209a4 	.word	0x417209a4
417004cc:	170f2000 	.word	0x170f2000

417004d0 <hw_dec_decompress_ex>:
417004d0:	e92d407f 	push	{r0, r1, r2, r3, r4, r5, r6, lr}
417004d4:	e1a06002 	mov	r6, r2
417004d8:	e3560000 	cmp	r6, #0
417004dc:	13530000 	cmpne	r3, #0
417004e0:	e1a05000 	mov	r5, r0
417004e4:	e1a00003 	mov	r0, r3
417004e8:	03a03001 	moveq	r3, #1
417004ec:	13a03000 	movne	r3, #0
417004f0:	e3510000 	cmp	r1, #0
417004f4:	03833001 	orreq	r3, r3, #1
417004f8:	e59d2020 	ldr	r2, [sp, #32]
417004fc:	e3530000 	cmp	r3, #0
41700500:	1a00002c 	bne	417005b8 <hw_dec_decompress_ex+0xe8>
41700504:	e5963000 	ldr	r3, [r6]
41700508:	e3520000 	cmp	r2, #0
4170050c:	13530000 	cmpne	r3, #0
41700510:	0a000028 	beq	417005b8 <hw_dec_decompress_ex+0xe8>
41700514:	e3550003 	cmp	r5, #3
41700518:	979ff105 	ldrls	pc, [pc, r5, lsl #2]
4170051c:	ea000025 	b	417005b8 <hw_dec_decompress_ex+0xe8>
41700520:	417005ac 	.word	0x417005ac
41700524:	41700530 	.word	0x41700530
41700528:	41700598 	.word	0x41700598
4170052c:	417005a4 	.word	0x417005a4
41700530:	e3a0c000 	mov	ip, #0
41700534:	e1a0300c 	mov	r3, ip
41700538:	e59fe080 	ldr	lr, [pc, #128]	@ 417005c0 <hw_dec_decompress_ex+0xf0>
4170053c:	e3a04000 	mov	r4, #0
41700540:	e2455002 	sub	r5, r5, #2
41700544:	e58ec000 	str	ip, [lr]
41700548:	e59fc074 	ldr	ip, [pc, #116]	@ 417005c4 <hw_dec_decompress_ex+0xf4>
4170054c:	e58c3000 	str	r3, [ip]
41700550:	e59f3070 	ldr	r3, [pc, #112]	@ 417005c8 <hw_dec_decompress_ex+0xf8>
41700554:	e5834000 	str	r4, [r3]
41700558:	e3a03001 	mov	r3, #1
4170055c:	e58d4008 	str	r4, [sp, #8]
41700560:	e58d4004 	str	r4, [sp, #4]
41700564:	e58d3000 	str	r3, [sp]
41700568:	e5963000 	ldr	r3, [r6]
4170056c:	ebffff6d 	bl	41700328 <hw_dec_start>
41700570:	e3550001 	cmp	r5, #1
41700574:	81a00004 	movhi	r0, r4
41700578:	8a000004 	bhi	41700590 <hw_dec_decompress_ex+0xc0>
4170057c:	ebffff97 	bl	417003e0 <hw_dec_wait_finish>
41700580:	e59f3044 	ldr	r3, [pc, #68]	@ 417005cc <hw_dec_decompress_ex+0xfc>
41700584:	e5933088 	ldr	r3, [r3, #136]	@ 0x88
41700588:	f57ff05f 	dmb	sy
4170058c:	e5863000 	str	r3, [r6]
41700590:	e28dd010 	add	sp, sp, #16
41700594:	e8bd8070 	pop	{r4, r5, r6, pc}
41700598:	e3a0c000 	mov	ip, #0
4170059c:	e3a03001 	mov	r3, #1
417005a0:	eaffffe4 	b	41700538 <hw_dec_decompress_ex+0x68>
417005a4:	e3a0c001 	mov	ip, #1
417005a8:	eafffffb 	b	4170059c <hw_dec_decompress_ex+0xcc>
417005ac:	e1a03005 	mov	r3, r5
417005b0:	e3a0c001 	mov	ip, #1
417005b4:	eaffffdf 	b	41700538 <hw_dec_decompress_ex+0x68>
417005b8:	e3e00000 	mvn	r0, #0
417005bc:	eafffff3 	b	41700590 <hw_dec_decompress_ex+0xc0>
417005c0:	417209a0 	.word	0x417209a0
417005c4:	4172099c 	.word	0x4172099c
417005c8:	417209a4 	.word	0x417209a4
417005cc:	170f2000 	.word	0x170f2000

417005d0 <hw_dec_init>:
417005d0:	e59f3074 	ldr	r3, [pc, #116]	@ 4170064c <hw_dec_init+0x7c>
417005d4:	e5932b80 	ldr	r2, [r3, #2944]	@ 0xb80
417005d8:	f57ff05f 	dmb	sy
417005dc:	e3822010 	orr	r2, r2, #16
417005e0:	f57ff05f 	dmb	sy
417005e4:	e5832b80 	str	r2, [r3, #2944]	@ 0xb80
417005e8:	e5932b80 	ldr	r2, [r3, #2944]	@ 0xb80
417005ec:	f57ff05f 	dmb	sy
417005f0:	e3c22001 	bic	r2, r2, #1
417005f4:	f57ff05f 	dmb	sy
417005f8:	e5832b80 	str	r2, [r3, #2944]	@ 0xb80
417005fc:	f57ff05f 	dmb	sy
41700600:	e59f3048 	ldr	r3, [pc, #72]	@ 41700650 <hw_dec_init+0x80>
41700604:	e3a01000 	mov	r1, #0
41700608:	e5831108 	str	r1, [r3, #264]	@ 0x108
4170060c:	f57ff05f 	dmb	sy
41700610:	e3a02003 	mov	r2, #3
41700614:	e583210c 	str	r2, [r3, #268]	@ 0x10c
41700618:	f57ff05f 	dmb	sy
4170061c:	e5831110 	str	r1, [r3, #272]	@ 0x110
41700620:	f57ff05f 	dmb	sy
41700624:	e5832114 	str	r2, [r3, #276]	@ 0x114
41700628:	e1a02001 	mov	r2, r1
4170062c:	e3a01003 	mov	r1, #3
41700630:	e7c72011 	bfi	r2, r1, #0, #8
41700634:	f57ff05f 	dmb	sy
41700638:	e5832128 	str	r2, [r3, #296]	@ 0x128
4170063c:	f57ff05f 	dmb	sy
41700640:	e3a02001 	mov	r2, #1
41700644:	e5832100 	str	r2, [r3, #256]	@ 0x100
41700648:	e12fff1e 	bx	lr
4170064c:	11012000 	.word	0x11012000
41700650:	170f0000 	.word	0x170f0000

41700654 <hw_dec_uinit>:
41700654:	f57ff05f 	dmb	sy
41700658:	e59f2040 	ldr	r2, [pc, #64]	@ 417006a0 <hw_dec_uinit+0x4c>
4170065c:	e3a03000 	mov	r3, #0
41700660:	e5823100 	str	r3, [r2, #256]	@ 0x100
41700664:	e3c330ff 	bic	r3, r3, #255	@ 0xff
41700668:	f57ff05f 	dmb	sy
4170066c:	e5823128 	str	r3, [r2, #296]	@ 0x128
41700670:	e59f302c 	ldr	r3, [pc, #44]	@ 417006a4 <hw_dec_uinit+0x50>
41700674:	e5932b80 	ldr	r2, [r3, #2944]	@ 0xb80
41700678:	f57ff05f 	dmb	sy
4170067c:	e3822001 	orr	r2, r2, #1
41700680:	f57ff05f 	dmb	sy
41700684:	e5832b80 	str	r2, [r3, #2944]	@ 0xb80
41700688:	e5932b80 	ldr	r2, [r3, #2944]	@ 0xb80
4170068c:	f57ff05f 	dmb	sy
41700690:	e3c22010 	bic	r2, r2, #16
41700694:	f57ff05f 	dmb	sy
41700698:	e5832b80 	str	r2, [r3, #2944]	@ 0xb80
4170069c:	e12fff1e 	bx	lr
417006a0:	170f0000 	.word	0x170f0000
417006a4:	11012000 	.word	0x11012000

417006a8 <start_armboot>:
417006a8:	e92d4030 	push	{r4, r5, lr}
417006ac:	e59f0094 	ldr	r0, [pc, #148]	@ 41700748 <start_armboot+0xa0>
417006b0:	e24dd014 	sub	sp, sp, #20
417006b4:	ebfffeb7 	bl	41700198 <uart_early_puts>
417006b8:	e59f308c 	ldr	r3, [pc, #140]	@ 4170074c <start_armboot+0xa4>
417006bc:	e3a02000 	mov	r2, #0
417006c0:	e5832000 	str	r2, [r3]
417006c4:	ebffffc1 	bl	417005d0 <hw_dec_init>
417006c8:	e59f2080 	ldr	r2, [pc, #128]	@ 41700750 <start_armboot+0xa8>
417006cc:	e28dc00c 	add	ip, sp, #12
417006d0:	e59f307c 	ldr	r3, [pc, #124]	@ 41700754 <start_armboot+0xac>
417006d4:	e1a0100c 	mov	r1, ip
417006d8:	e2420004 	sub	r0, r2, #4
417006dc:	e0423003 	sub	r3, r2, r3
417006e0:	e4d0e001 	ldrb	lr, [r0], #1
417006e4:	e4cce001 	strb	lr, [ip], #1
417006e8:	e1500002 	cmp	r0, r2
417006ec:	1afffffb 	bne	417006e0 <start_armboot+0x38>
417006f0:	e59f5060 	ldr	r5, [pc, #96]	@ 41700758 <start_armboot+0xb0>
417006f4:	e3a02000 	mov	r2, #0
417006f8:	e58d2000 	str	r2, [sp]
417006fc:	e1a00005 	mov	r0, r5
41700700:	e59f204c 	ldr	r2, [pc, #76]	@ 41700754 <start_armboot+0xac>
41700704:	ebffff51 	bl	41700450 <hw_dec_decompress>
41700708:	e2504000 	subs	r4, r0, #0
4170070c:	1a00000a 	bne	4170073c <start_armboot+0x94>
41700710:	e59f0044 	ldr	r0, [pc, #68]	@ 4170075c <start_armboot+0xb4>
41700714:	ebfffe9f 	bl	41700198 <uart_early_puts>
41700718:	ebffffcd 	bl	41700654 <hw_dec_uinit>
4170071c:	ee074f15 	mcr	15, 0, r4, cr7, cr5, {0}
41700720:	ee074fd5 	mcr	15, 0, r4, cr7, cr5, {6}
41700724:	f57ff04f 	dsb	sy
41700728:	f57ff06f 	isb	sy
4170072c:	e1a03005 	mov	r3, r5
41700730:	e28dd014 	add	sp, sp, #20
41700734:	e8bd4030 	pop	{r4, r5, lr}
41700738:	e12fff13 	bx	r3
4170073c:	e59f001c 	ldr	r0, [pc, #28]	@ 41700760 <start_armboot+0xb8>
41700740:	ebfffe94 	bl	41700198 <uart_early_puts>
41700744:	eafffffe 	b	41700744 <start_armboot+0x9c>
41700748:	417208eb 	.word	0x417208eb
4170074c:	417209a4 	.word	0x417209a4
41700750:	417208c8 	.word	0x417208c8
41700754:	417008c0 	.word	0x417008c0
41700758:	41800000 	.word	0x41800000
4170075c:	417208f9 	.word	0x417208f9
41700760:	417208fd 	.word	0x417208fd

41700764 <hang>:
41700764:	e59f000c 	ldr	r0, [pc, #12]	@ 41700778 <hang+0x14>
41700768:	e92d4010 	push	{r4, lr}
4170076c:	ebfffe89 	bl	41700198 <uart_early_puts>
41700770:	eafffffe 	b	41700770 <hang+0xc>
41700774:	e8bd8010 	pop	{r4, pc}
41700778:	41720903 	.word	0x41720903

4170077c <do_bad_sync>:
4170077c:	e92d4010 	push	{r4, lr}
41700780:	e59f0010 	ldr	r0, [pc, #16]	@ 41700798 <do_bad_sync+0x1c>
41700784:	ebfffe83 	bl	41700198 <uart_early_puts>
41700788:	e59f000c 	ldr	r0, [pc, #12]	@ 4170079c <do_bad_sync+0x20>
4170078c:	ebfffe81 	bl	41700198 <uart_early_puts>
41700790:	e8bd4010 	pop	{r4, lr}
41700794:	ea000040 	b	4170089c <reset_cpu>
41700798:	4172092d 	.word	0x4172092d
4170079c:	4172093e 	.word	0x4172093e

417007a0 <do_sync>:
417007a0:	e92d4010 	push	{r4, lr}
417007a4:	e59f0010 	ldr	r0, [pc, #16]	@ 417007bc <do_sync+0x1c>
417007a8:	ebfffe7a 	bl	41700198 <uart_early_puts>
417007ac:	e59f000c 	ldr	r0, [pc, #12]	@ 417007c0 <do_sync+0x20>
417007b0:	ebfffe78 	bl	41700198 <uart_early_puts>
417007b4:	e8bd4010 	pop	{r4, lr}
417007b8:	ea000037 	b	4170089c <reset_cpu>
417007bc:	41720931 	.word	0x41720931
417007c0:	4172093e 	.word	0x4172093e

417007c4 <do_bad_error>:
417007c4:	e92d4010 	push	{r4, lr}
417007c8:	e59f0010 	ldr	r0, [pc, #16]	@ 417007e0 <do_bad_error+0x1c>
417007cc:	ebfffe71 	bl	41700198 <uart_early_puts>
417007d0:	e59f000c 	ldr	r0, [pc, #12]	@ 417007e4 <do_bad_error+0x20>
417007d4:	ebfffe6f 	bl	41700198 <uart_early_puts>
417007d8:	e8bd4010 	pop	{r4, lr}
417007dc:	ea00002e 	b	4170089c <reset_cpu>
417007e0:	41720952 	.word	0x41720952
417007e4:	4172093e 	.word	0x4172093e

417007e8 <do_error>:
417007e8:	e92d4010 	push	{r4, lr}
417007ec:	e59f0010 	ldr	r0, [pc, #16]	@ 41700804 <do_error+0x1c>
417007f0:	ebfffe68 	bl	41700198 <uart_early_puts>
417007f4:	e59f000c 	ldr	r0, [pc, #12]	@ 41700808 <do_error+0x20>
417007f8:	ebfffe66 	bl	41700198 <uart_early_puts>
417007fc:	e8bd4010 	pop	{r4, lr}
41700800:	ea000025 	b	4170089c <reset_cpu>
41700804:	41720956 	.word	0x41720956
41700808:	4172093e 	.word	0x4172093e

4170080c <do_bad_fiq>:
4170080c:	e92d4010 	push	{r4, lr}
41700810:	e59f0010 	ldr	r0, [pc, #16]	@ 41700828 <do_bad_fiq+0x1c>
41700814:	ebfffe5f 	bl	41700198 <uart_early_puts>
41700818:	e59f000c 	ldr	r0, [pc, #12]	@ 4170082c <do_bad_fiq+0x20>
4170081c:	ebfffe5d 	bl	41700198 <uart_early_puts>
41700820:	e8bd4010 	pop	{r4, lr}
41700824:	ea00001c 	b	4170089c <reset_cpu>
41700828:	4172095e 	.word	0x4172095e
4170082c:	4172093e 	.word	0x4172093e

41700830 <do_bad_irq>:
41700830:	e92d4010 	push	{r4, lr}
41700834:	e59f0010 	ldr	r0, [pc, #16]	@ 4170084c <do_bad_irq+0x1c>
41700838:	ebfffe56 	bl	41700198 <uart_early_puts>
4170083c:	e59f000c 	ldr	r0, [pc, #12]	@ 41700850 <do_bad_irq+0x20>
41700840:	ebfffe54 	bl	41700198 <uart_early_puts>
41700844:	e8bd4010 	pop	{r4, lr}
41700848:	ea000013 	b	4170089c <reset_cpu>
4170084c:	4172097b 	.word	0x4172097b
41700850:	4172093e 	.word	0x4172093e

41700854 <do_fiq>:
41700854:	e92d4010 	push	{r4, lr}
41700858:	e59f0010 	ldr	r0, [pc, #16]	@ 41700870 <do_fiq+0x1c>
4170085c:	ebfffe4d 	bl	41700198 <uart_early_puts>
41700860:	e59f000c 	ldr	r0, [pc, #12]	@ 41700874 <do_fiq+0x20>
41700864:	ebfffe4b 	bl	41700198 <uart_early_puts>
41700868:	e8bd4010 	pop	{r4, lr}
4170086c:	ea00000a 	b	4170089c <reset_cpu>
41700870:	41720962 	.word	0x41720962
41700874:	4172093e 	.word	0x4172093e

41700878 <do_irq>:
41700878:	e92d4010 	push	{r4, lr}
4170087c:	e59f0010 	ldr	r0, [pc, #16]	@ 41700894 <do_irq+0x1c>
41700880:	ebfffe44 	bl	41700198 <uart_early_puts>
41700884:	e59f000c 	ldr	r0, [pc, #12]	@ 41700898 <do_irq+0x20>
41700888:	ebfffe42 	bl	41700198 <uart_early_puts>
4170088c:	e8bd4010 	pop	{r4, lr}
41700890:	ea000001 	b	4170089c <reset_cpu>
41700894:	4172097f 	.word	0x4172097f
41700898:	4172093e 	.word	0x4172093e

4170089c <reset_cpu>:
4170089c:	e59f100c 	ldr	r1, [pc, #12]	@ 417008b0 <rstctl>
417008a0:	e3a03002 	mov	r3, #2
417008a4:	e5813000 	str	r3, [r1]
417008a8:	e1a00000 	nop			@ (mov r0, r0)

417008ac <_loop_forever>:
417008ac:	eafffffe 	b	417008ac <_loop_forever>

417008b0 <rstctl>:
417008b0:	11020004 	.word	0x11020004
