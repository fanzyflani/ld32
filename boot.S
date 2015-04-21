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
	.word snd_graze, snd_graze_end - snd_graze
	.ascii "s_graze "
	.word snd_pshot, snd_pshot_end - snd_pshot
	.ascii "s_pshot "
	.word snd_eshot, snd_eshot_end - snd_eshot
	.ascii "s_eshot "
	.word snd_edead, snd_edead_end - snd_edead
	.ascii "s_edead "
	.word snd_pdead, snd_pdead_end - snd_pdead
	.ascii "s_pdead "
	.word snd_bdead, snd_bdead_end - snd_bdead
	.ascii "s_bdead "
	.word snd_ehit1, snd_ehit1_end - snd_ehit1
	.ascii "s_ehit1 "
	.word snd_ehit2, snd_ehit2_end - snd_ehit2
	.ascii "s_ehit2 "
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
snd_graze:
	.incbin "graze.raw"
snd_graze_end:
	.align 4
snd_pshot:
	.incbin "pshot.raw"
snd_pshot_end:
	.align 4
snd_eshot:
	.incbin "eshot.raw"
snd_eshot_end:
	.align 4
snd_edead:
	.incbin "edead.raw"
snd_edead_end:
	.align 4
snd_pdead:
	.incbin "pdead.raw"
snd_pdead_end:
	.align 4
snd_bdead:
	.incbin "bdead.raw"
snd_bdead_end:
	.align 4
snd_ehit1:
	.incbin "ehit1.raw"
snd_ehit1_end:
	.align 4
snd_ehit2:
	.incbin "ehit2.raw"
snd_ehit2_end:
	.align 4

