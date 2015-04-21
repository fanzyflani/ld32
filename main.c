/*
Exotic Butterfly Net
Copyright (c) 2015 fanzyflani

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <stdint.h>

#include "gba.h"
#include "slowmem.h"

#define TARGET_GBA
#include "f3m.c"

const uint8_t sztab[3][4][2] = {
	{{8,8}, {16,16}, {32,32}, {64,64}},
	{{16,8}, {32,8}, {32,16}, {64,32}},
	{{8,16}, {8,32}, {16,32}, {32,64}},
};

// 2^14 amplitude
const int16_t sintab[256] = {
0,402,804,1205,1606,2006,2404,2801,3196,3590,3981,4370,4756,5139,5520,5897,
6270,6639,7005,7366,7723,8076,8423,8765,9102,9434,9760,10080,10394,10702,11003,11297,
11585,11866,12140,12406,12665,12916,13160,13395,13623,13842,14053,14256,14449,14635,14811,14978,
15137,15286,15426,15557,15679,15791,15893,15986,16069,16143,16207,16261,16305,16340,16364,16379,
16384,16379,16364,16340,16305,16261,16207,16143,16069,15986,15893,15791,15679,15557,15426,15286,
15137,14978,14811,14635,14449,14256,14053,13842,13623,13395,13160,12916,12665,12406,12140,11866,
11585,11297,11003,10702,10394,10080,9760,9434,9102,8765,8423,8076,7723,7366,7005,6639,
6270,5897,5520,5139,4756,4370,3981,3590,3196,2801,2404,2006,1606,1205,804,402,
0,-401,-803,-1204,-1605,-2005,-2403,-2800,-3195,-3589,-3980,-4369,-4755,-5138,-5519,-5896,
-6269,-6638,-7004,-7365,-7722,-8075,-8422,-8764,-9101,-9433,-9759,-10079,-10393,-10701,-11002,-11296,
-11584,-11865,-12139,-12405,-12664,-12915,-13159,-13394,-13622,-13841,-14052,-14255,-14448,-14634,-14810,-14977,
-15136,-15285,-15425,-15556,-15678,-15790,-15892,-15985,-16068,-16142,-16206,-16260,-16304,-16339,-16363,-16378,
-16383,-16378,-16363,-16339,-16304,-16260,-16206,-16142,-16068,-15985,-15892,-15790,-15678,-15556,-15425,-15285,
-15136,-14977,-14810,-14634,-14448,-14255,-14052,-13841,-13622,-13394,-13159,-12915,-12664,-12405,-12139,-11865,
-11584,-11296,-11002,-10701,-10393,-10079,-9759,-9433,-9101,-8764,-8422,-8075,-7722,-7365,-7004,-6638,
-6269,-5896,-5519,-5138,-4755,-4369,-3980,-3589,-3195,-2800,-2403,-2005,-1605,-1204,-803,-401,
};

volatile int mod_bump = 0;
volatile int keypad = 0;
volatile int okeypad = 0;

uint16_t bgpal[4];

int32_t mbuf[F3M_BUFLEN*F3M_CHNS];
uint8_t obuf[3][F3M_BUFLEN*F3M_CHNS];
player_s player;

int oam_soffs = 0;
int oam_doffs = 0;

uint8_t *s_graze; uint32_t s_graze_len;
uint8_t *s_pshot; uint32_t s_pshot_len;
uint8_t *s_eshot; uint32_t s_eshot_len;
uint8_t *s_edead; uint32_t s_edead_len;
uint8_t *s_pdead; uint32_t s_pdead_len;
uint8_t *s_bdead; uint32_t s_bdead_len;
uint8_t *s_ehit1; uint32_t s_ehit1_len;
uint8_t *s_ehit2; uint32_t s_ehit2_len;
#define CHN_GRAZE 16
#define CHN_PSHOT 17
#define CHN_ESHOT 18
#define CHN_EDEAD 19
#define CHN_PDEAD 20
#define CHN_EHITX 21

// 0 = easy
// 1 = normal
// 2 = hard
int difficulty = 1;

#define DIFF(a,b,c) \
	(difficulty == 0 ? (a) : difficulty == 1 ? (b) : (c))

// takes a BCD score
// we skip the first digit
void score_inc(uint32_t sinc)
{
	int i;
	for(i = SCORE_DIGITS-2; i >= 0 && sinc != 0; i--)
	{
		int dig = sinc & 0xF;
		sinc >>= 4;
		dig += score[i];
		if(dig > 9)
		{
			sinc += 0x1;
			dig -= 10;
		}

		score[i] = dig;
	}

}

void blend_bg_pal(int r, int g, int b)
{
	int i;

	for(i = 0; i < 4; i++)
	{
		int sc = bgpal[i];
		int sr = (sc>>0)&31;
		int sg = (sc>>5)&31;
		int sb = (sc>>10)&31;

		sr *= r;
		sg *= g;
		sb *= b;

		sr >>= 8;
		sg >>= 8;
		sb >>= 8;

		VPAL0[32+i] = sr|(sg<<5)|(sb<<10);
	}
}

#include "ent.c"
#include "lv01.c"

static void tga_load(uint8_t *tga, uint16_t *dest, uint16_t *pal, int w, int h)
{
	int x, y, i;

	// skip to palette
	tga += 0x12;

	// load palette
	for(i = 0; i < 256; i++)
	{
		int b = tga[0];
		int g = tga[1];
		int r = tga[2];

		tga += 3;

		r >>= 3;
		g >>= 3;
		b >>= 3;

		pal[i] = (b<<10) | (g<<5) | (r<<0);
	}

	// load image
	for(y = 0; y < h; y++)
	for(x = 0; x < w; x += 2)
	{
		dest[((y>>3)*(w<<3) + (x&7) + ((y&7)<<3) + ((x>>3)<<6))>>1] = *(uint16_t *)tga;
		tga += 2;
	}
}

static void isr_handler(void)
{
	//asm("mov pc, lr;\n");
	uint16_t isr_state = IF;

	if(isr_state & 0x0001)
	{
		if(mod_bump != 0)
		{
			// Start DMA1 (sound/music)
			DMA1CNT_H = 0x0000;
			DMA1SAD = obuf[0];
			DMA1DAD = &FIFO_A;
			DMA1CNT_H = 0xB600;

			mod_bump = 0;
		}

		// Acknowledge vblank
		IF = 0x0001;
		ISR_flags |= 0x0001;
	}

	//asm("mov r0, #0x08000000 ; bx r0 ;\n");
}

static void wait_timer()
{
	while(!(ISR_flags & 0x0001)) {}
	ISR_flags &= ~0x0001;
}

void game_update_disp(void)
{
	int i, j;

	// Update entities
	j = (oam_soffs == -1 ? 0 : oam_soffs);
	oam_soffs = -1;
	oam_doffs = 0;
	for(i = 0; i < entlist_used_end; i++, j++)
	{
		if(j > entlist_used_end)
			j = 0;

		ent *e = &entlist[j];

		if(e->base.typ == ENT_NONE)
			continue;

		int sx, sy;
		if(e->base.oamidx < 0x8000 && !ent_off_screen(e, 0, &sx, &sy))
		{
			if(oam_doffs < 128)
			{
				VOAM16[4*oam_doffs + 0] = (e->base.oamdata[0] & ~255) | (sy & 255);
				VOAM16[4*oam_doffs + 1] = (e->base.oamdata[1] & ~511) | (sx & 511);
				VOAM16[4*oam_doffs + 2] = e->base.oamdata[2];
				oam_doffs++;
			} else if(oam_soffs == -1) {
				oam_soffs = j;
			}
		}
	}

	for(i = oam_doffs; i < 128; i++)
	{
		VOAM16[4*i + 0] = 0x02F0;
		VOAM16[4*i + 1] = 0x0000;
		VOAM16[4*i + 2] = 0x0000;

	}
}

void game_update_tick(void)
{
	int i;

	// Update keypad
	okeypad = keypad;
	keypad = ~KEYINPUT;

	// Update entities
	for(i = 0; i < entlist_used_end; i++)
	{
		ent *e = &entlist[i];

		if(e->base.typ == ENT_NONE)
			continue;

		if(e->base.f_tick != NULL)
			e->base.f_tick(e);
	}

	if((okeypad & KEY_START) && !(keypad & KEY_START))
		((void (*)(void))0x08000000)();

}

int fs_get(const char *name, void **loc, uint32_t *len)
{
	const uint64_t *match = (const uint64_t *)name;

	void *fol = (void *)0x080000C4;

	while(*((uint32_t *)(fol+0x0)) != 0)
	{
		void *floc = *((void **)(fol+0x0));
		uint32_t flen = *((uint32_t *)(fol+0x4));
		uint64_t fname = *((uint64_t *)(fol+0x8));
		if(fname == *match)
		{
			if(loc != NULL) *loc = floc;
			if(len != NULL) *len = flen;
			return 1;
		}

		fol += 0x10;
	}

	return 0;
}

int fs_get_must(const char *name, void **loc, uint32_t *len)
{
	if(!fs_get(name, loc, len))
	{
		// lock up
		VPAL0[0] = 0xFFF;
		for(;;) {}
	}
}

void _start(void)
{
	int i;

	// Set up interrupts
	IME = 0;
	ISR_funct = isr_handler;
	IE = 0x0001;
	IME = 1;

	// Kill sound 
	SOUNDCNT_X = 0;
	SOUNDCNT_L = 0;
	SOUNDCNT_H = 0;
	TM0CNT_H = 0;
	DMA1CNT_H = 0;

	// Clear EWRAM/slowmem
	for(i = 0; i < (256*1024)/4; i++)
		((volatile uint32_t *)(0x02000000))[i] = 0;

	DISPCNT = 0x0000;

	// Clear VRAM and OAM
	for(i = 0; i < (0x18000>>2); i++)
		VRAM0D32[i] = 0;
	for(i = 0; i < 256*2; i++)
		VPAL0[i] = 0;
	for(i = 0; i < 128; i++)
	{
		VOAM16[i*4+0] = 0x02F0;
		VOAM16[i*4+1] = 0x0000;
		VOAM16[i*4+2] = 0x0000;
		VOAM16[i*4+3] = 0x0000;
	}

	// Load splash
	DISPCNT = 0x0404;

	uint16_t *ptr_tit01;
	fs_get_must("tit01   ", (void **)&ptr_tit01, NULL);

	VPAL0[0] = 0x0000;
	VPAL0[1] = 0x7FFF;
	for(i = 0; i < 120*160; i++)
		VRAM0D[i] = ptr_tit01[i + (0x18>>1)];

	difficulty = -1;
	for(;;)
	{
		//wait_timer();
		okeypad = keypad;
		keypad = ~KEYINPUT;
		VPAL0[0]++;

		if((!(keypad & KEY_START)) && (okeypad & KEY_START))
		{
			if(okeypad & KEY_A) difficulty = 0;
			else if(okeypad & KEY_B) difficulty = 1;
			else difficulty = 2;
			break;
		}
	}

	//VPAL0[1] = 0;

	DISPCNT = 0x1300;
	BG0CNT = 1 | (1<<7) | (31<<8); // BG0 == HUD  - tmap @ 0xF800 (0x7C00 word address)
	BG1CNT = 3 | (1<<7) | (30<<8); // BG1 == land - tmap @ 0xF000 (0x7800 word address)

	// Load images
	void *ptr_spr01;
	void *ptr_glv01;
	fs_get_must("spr01   ", (void **)&ptr_spr01, NULL);
	fs_get_must("glv01   ", (void **)&ptr_glv01, NULL);
	tga_load(ptr_spr01, VRAM0D + ((0x10000)>>1), VPAL1, 128, 256);
	tga_load(ptr_glv01, VRAM0D + ((0x00000)>>1), VPAL0, 128, 256);
	for(i = 0; i < 4; i++)
		bgpal[i] = VPAL0[32+i];
	//VPAL0[0] = 0x421*((31+3/2)/3);

	// Reset all entities
	entlist_used_end = 0;
	entlist_free_start = 0;
	for(i = 0; i < ENT_MAX; i++)
		ent_reset(&entlist[i], NULL, 0, 0, ENT_NONE, -1, -1, 0);

	// Set OAM indices for each entity
	for(i = 0; i < ENT_MAX; i++)
		entlist[i].base.oamidx = i|0x8000;

	// Create player
	lives = 2;
	bombs = 0;
	e_player = ent_new_player();
	lv01_new_controller();

	// Set up audio
	fs_get_must("s_graze ", (void **)&s_graze, &s_graze_len);
	fs_get_must("s_pshot ", (void **)&s_pshot, &s_pshot_len);
	fs_get_must("s_eshot ", (void **)&s_eshot, &s_eshot_len);
	fs_get_must("s_edead ", (void **)&s_edead, &s_edead_len);
	fs_get_must("s_pdead ", (void **)&s_pdead, &s_pdead_len);
	fs_get_must("s_bdead ", (void **)&s_bdead, &s_bdead_len);
	fs_get_must("s_ehit1 ", (void **)&s_ehit1, &s_ehit1_len);
	fs_get_must("s_ehit2 ", (void **)&s_ehit2, &s_ehit2_len);
	mod_s *mod;
	fs_get_must("mus01   ", (void **)&mod, NULL);

	f3m_player_init(&player, mod);
	f3m_player_play(&player, mbuf, obuf[0]);
	f3m_player_play(&player, mbuf, obuf[1]);

	// Init sound properly
	SOUNDCNT_X = (1<<7);
	SOUNDCNT_L = (7<<0) | (7<<4);
	SOUNDCNT_H = (2<<0) | (1<<2) | (1<<11) | (3<<8);

	TM0CNT_H = 0;

	DMA1SAD = obuf[0];
	DMA1DAD = &FIFO_A;
	DMA1CNT_H = 0xB600;

	TM0CNT_L = 0x10000-((1<<24)/F3M_FREQ);
	TM0CNT_H = 0x0080;

	// Enable interrupts and run
	DISPSTAT |= 0x0008;
	for(;;)
	{
		wait_timer();
		game_update_disp();
		f3m_player_play(&player, mbuf, obuf[0]);
		// copy a bit to the end
		((uint32_t *)(obuf[2]))[0] = ((uint32_t *)(obuf[0]))[0];
		((uint32_t *)(obuf[2]))[1] = ((uint32_t *)(obuf[0]))[1];
		((uint32_t *)(obuf[2]))[2] = ((uint32_t *)(obuf[0]))[2];
		((uint32_t *)(obuf[2]))[3] = ((uint32_t *)(obuf[0]))[3];
		mod_bump = 1;
		game_update_tick();
		wait_timer();
		game_update_disp();
		f3m_player_play(&player, mbuf, obuf[1]);
		game_update_tick();
	}

	for(;;) {}
}

