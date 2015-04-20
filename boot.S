	// simple GBA fastmem bootloader
	// GreaseMonkey, 2014-2015 - Public Domain
	.arm
	.section .text
	.global run_exe
	.extern init
	b _start_asm
	.incbin "nintendo.bin"
	.ascii "multibooter "
	.ascii "AAAE00"
	.byte 0x96, 0x00, 0x00
	.byte 0,0,0,0,0,0,0
	.byte 0x00
	.byte 0x13 // compliment check. here's a compliment: NINTENDO SUCKS.
	.hword 0x00
	b run_exe
	// data goes here
	.word s3m_trk01, s3m_trk01_end - s3m_trk01
	.ascii "mus01   "
	.word s3m_trk02, s3m_trk02_end - s3m_trk02
	.ascii "mus02   "
	.word tga_tit01, tga_tit01_end - tga_tit01
	.ascii "tit01   "
	.word tga_spr01, tga_spr01_end - tga_spr01
	.ascii "spr01   "
	.word tga_glv01, tga_glv01_end - tga_glv01
	.ascii "glv01   "
	.word 0

_start_asm:
	ldr r0, =init_beg
	ldr r1, =init_end
run_exe:
	/* Copy to WRAM */
	ldr r2, =0x03000000
	mov r3, r2
	_lp_copy_prog:
		ldmia r0!, {r4-r11}
		stmia r2!, {r4-r11}
		cmp r1, r0
		bhi _lp_copy_prog

	/* Jump */
	bx r3
	.pool

	.align 5
init_beg:
	.incbin "main.bin"
	.align 5
init_end:
s3m_trk01:
	.incbin "ld32-trk1.s3m"
s3m_trk01_end:
	.align 4
s3m_trk02:
	.incbin "ld32-trk2.s3m"
s3m_trk02_end:
	.align 4
tga_tit01:
	.incbin "splash0.tga"
tga_tit01_end:
	.align 4
tga_spr01:
	.incbin "spr0.tga"
tga_spr01_end:
	.align 4
tga_glv01:
	.incbin "lv01.tga"
tga_glv01_end:
	.align 4

