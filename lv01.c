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

void lv01_enemy01_tick(ent *e)
{
	int i;

	if(e->base.ctr[0] < 70)
	{
		e->base.x += (F12N(e->enemy.tx) - e->base.x)>>4;
		e->base.y += (F12N(e->enemy.ty) - e->base.y)>>4;
		e->base.ctr[0]++;
	} else {
		e->base.x += e->base.vx;
		e->base.y += e->base.vy;
		if(ent_off_screen(e, 2, NULL, NULL))
			return ent_free(e);

		if(--e->base.ctr[1] < 0)
		{
			e->base.ctr[1] = 20;
			// 
			ent *plr = &entlist[0];
			fed12 vxh = (plr->base.x - e->base.x)>>6;
			fed12 vyh = (plr->base.y - e->base.y)>>6;
			fed12 vxl = 0;
			fed12 vyl = 0;
			fed12 vx = (vxh+vxl)>>1;
			fed12 vy = (vyh+vyl)>>1;

			for(i = 0; i < 10; i++)
			{
				if(vx*vx + vy*vy > (1<<24))
				{
					vxh = vx;
					vyh = vy;
				} else {
					vxl = vx;
					vyl = vy;
				}

				vx = (vxh+vxl)>>1;
				vy = (vyh+vyl)>>1;
			}

			vx *= 2;
			vy *= 2;

			ent_new_bullet_glide(e, ENT_EBULLET, e->base.x, e->base.y, vx, vy, 2, 1, 0, 4);
			f3m_sfx_play(&player, CHN_ESHOT, F3M_PRIO_NORMAL, s_eshot, s_eshot_len, 0, 32768, 64);

		}

	}
}

ent *lv01_new_enemy01(int32_t x, int32_t y, fed12 vx, fed12 vy)
{
	ent *e = ent_new(NULL, F12N(x), F12N(-64), ENT_ENEMY, 0, 6, 1);
	e->enemy.tx = (x);
	e->enemy.ty = (y);
	e->enemy.health = 3;
	e->enemy.radius = 4;
	e->base.vx = vx;
	e->base.vy = vy;
	e->base.f_tick = lv01_enemy01_tick;
	e->base.f_hit = f_enemy_hit;

	return e;
}

void lv01_enemy02_tick(ent *e)
{
	fed12 spd = DIFF(0x800, 0xC00, 0x1000);

	// Set pos
	e->base.x = F12N(e->enemy.tx) + ((e->base.ctr[0]*sintab[((e->base.vx>>12)+0x00)&0xFF])>>(14-12));
	e->base.y = F12N(e->enemy.ty) + ((e->base.ctr[0]*sintab[((e->base.vx>>12)-0x40)&0xFF])>>(14-12));

	// Fire shots
	if(--e->base.ctr[1] <= 0)
	{
		e->base.ctr[1] = DIFF(25, 15, 10);
		fed12 svx = (sintab[((e->base.vx>>12)+0x00)&0xFF])>>(14-12);
		fed12 svy = (sintab[((e->base.vx>>12)-0x40)&0xFF])>>(14-12);
		fed12 spd = DIFF(0x1200, 0x1800, 0x2400);
		svx = (spd*svx)>>12;
		svy = (spd*svy)>>12;
		ent_new_bullet_glide(e, ENT_EBULLET, e->base.x, e->base.y, svx, svy, 0, 1, 0, 4);
		f3m_sfx_play(&player, CHN_ESHOT, F3M_PRIO_NORMAL, s_eshot, s_eshot_len, 0, 32768, 64);
	}

	// Advance
	if(e->base.vx < e->base.vy)
	{
		e->base.vx += spd;
		if(e->base.vx >= e->base.vy)
			ent_free(e);
	} else {

		e->base.vx -= spd;
		if(e->base.vx < e->base.vy)
			ent_free(e);
	}
}

ent *lv01_new_enemy02(fed12 tx, fed12 ty, fed12 start_ang, fed12 end_ang, int radius)
{
	ent *e = ent_new(NULL, F12N(0), F12N(-160), ENT_ENEMY, 0, 6, 1);
	e->enemy.health = 2;
	e->enemy.radius = 4;
	e->enemy.tx = tx;
	e->enemy.ty = ty;
	e->base.vx = start_ang;
	e->base.vy = end_ang;
	e->base.f_tick = lv01_enemy02_tick;
	e->base.f_hit = f_enemy_hit;
	e->base.ctr[0] = radius;
	e->base.ctr[1] += 30;

	return e;
}

void lv01_mainboss_tick(ent *e)
{
	int i, j;

	// ctr[0]: countdown for next event
	// ctr[1]: current event
	// ctr[2]: health increment; repetition count for current event

	switch(e->base.ctr[1])
	{
		case 1:
			// dropin; inc health
			e->base.x += (F12N(e->enemy.tx) - e->base.x)>>4;
			e->base.y += (F12N(e->enemy.ty) - e->base.y)>>4;
			e->base.ctr[2] += 2;
			break;

		case 2: 
			// finish dropin
			e->base.x += (F12N(e->enemy.tx) - e->base.x)>>4;
			e->base.y += (F12N(e->enemy.ty) - e->base.y)>>4;
			break;

		case 3:
			// attack 0.0
			e->base.x += (F12N(e->enemy.tx) - e->base.x)>>4;
			e->base.y += (F12N(e->enemy.ty) - e->base.y)>>4;
			break;

		case 5:
			// prep attack 1.0; inc health
			e->base.x += (F12N(e->enemy.tx) - e->base.x)>>4;
			e->base.y += (F12N(e->enemy.ty) - e->base.y)>>4;
			e->base.ctr[2] += 2;
			break;

		case 6:
			// finish prep attack 1.0
			e->base.x += (F12N(e->enemy.tx) - e->base.x)>>4;
			e->base.y += (F12N(e->enemy.ty) - e->base.y)>>4;
			break;

		case 7:
			// attack 1.0
			e->base.x = F12N(120) + ((sintab[(e->base.vx>>4)&0xFF]*15)>>(14-12));
			e->base.vx += 0x1000/180;
			break;

		case 9:
			// prep attack 2.0; inc health
			e->base.x += (F12N(e->enemy.tx) - e->base.x)>>4;
			e->base.y += (F12N(e->enemy.ty) - e->base.y)>>4;
			e->base.ctr[2] += 2;
			break;

		case 10:
			// finish prep attack 2.0
			e->base.x += (F12N(e->enemy.tx) - e->base.x)>>4;
			e->base.y += (F12N(e->enemy.ty) - e->base.y)>>4;
			break;


		case 11:
			// attack 2.0
			e->base.x += (F12N(e->enemy.tx) - e->base.x)>>4;
			e->base.y += (F12N(e->enemy.ty) - e->base.y)>>4;
			if(e->base.ctr[0] % DIFF(30, 20, 10) == 0)
			{
				for(i = 0; i < 256; i += DIFF((256+5)/6, (256+7)/8, (256+11)/12))
				{
					int spd = DIFF(0x100, 0x140, 0x140);
					int ang = i + e->base.ctr[2]*10 + e->base.ctr[0];
					int vx = (sintab[(ang+0x00)&255]*spd)>>(14-12+8);
					int vy = (sintab[(ang-0x40)&255]*spd)>>(14-12+8);
					ent_new_bullet_glide(e, ENT_EBULLET, e->base.x, e->base.y, vx, vy, e->base.vy, 1, 0, 4);
				}

				e->base.vy++;
				if(e->base.vy == 1) e->base.vy = 2;
				if(e->base.vy == 4) e->base.vy = 0;

				f3m_sfx_play(&player, CHN_ESHOT, F3M_PRIO_NORMAL, s_eshot, s_eshot_len, 0, 32768, 64);

			}
			break;
	}

	// Set BG0 HUD
	int healthend = 240*(e->enemy.health > 0 ? e->enemy.health : e->base.ctr[2])/e->enemy.maxhealth;

	for(i = 0; i < 30; i++)
	{
		if(i < (healthend>>3))
			VRAM0D[0x7C00 + i] = 0x17 + 8;
		else if(i > (healthend>>3))
			VRAM0D[0x7C00 + i] = 0x17 + 0;
		else
			VRAM0D[0x7C00 + i] = 0x17 + (healthend&7);

	}
	

	if(--e->base.ctr[0] > 0)
		return;

	switch(e->base.ctr[1]++)
	{
		case 0:
			// dropin prep
			e->enemy.tx = 120;
			e->enemy.ty = 40;
			e->base.ctr[0] = 50;
			//e->base.ctr[1] = 8; // SKIP
			break;

		case 1:
			// dropin end
			e->base.ctr[0] = 120;
			break;

		case 2:
			// prep for attack 0.0
			e->enemy.health = e->enemy.maxhealth;
			e->enemy.goto_death = 4;
			e->base.ctr[2] = 0;
			e->base.ctr[1]++;

			// * FALL THROUGH *
			// attack 0.0
		case 3:
			e->base.ctr[1]--;
			e->base.ctr[0] = DIFF(30, 20, 15);

			for(i = 0; i < 8; i++)
			for(j = 0; j < DIFF(1, 2, 3); j++)
			{
				int spd = (j+3)*DIFF(0x100/3, 0x180/3, 0x140/3);
				int ang = i*(256/8) + j*DIFF(0, 256/8/2, 256/8/2)
					+ e->base.ctr[2]*DIFF(20, 20, 256/8/4);
				int vx = (sintab[(ang+0x00)&255]*spd)>>(14-12+8);
				int vy = (sintab[(ang-0x40)&255]*spd)>>(14-12+8);
				ent_new_bullet_glide(e, ENT_EBULLET, e->base.x, e->base.y, vx, vy, j, 1, 0, 4);
			}
			f3m_sfx_play(&player, CHN_ESHOT, F3M_PRIO_NORMAL, s_eshot, s_eshot_len, 0, 32768, 128);

			e->base.ctr[2]++;
			if(e->base.ctr[2] % DIFF(4, 6, 8) == 0)
			{
				e->enemy.tx = (e->enemy.tx == 90
					? 150
					: e->enemy.tx == 150
					? 120
					: 90);
				e->enemy.ty = (e->enemy.tx == 120
					? 30
					: 50);

			}
			break;

		case 4:
			// death after attack 0.0
			// health change
			e->enemy.tx = 120;
			e->enemy.ty = 40;
			e->base.ctr[0] = 50;
			e->enemy.health = -1;
			e->enemy.maxhealth = 100;
			e->base.ctr[2] = 0;
			break;

		case 5:
			// end regen for 1.0 health
			e->base.ctr[0] = 120;
			blend_bg_pal(32, 32, 256);
			break;

		case 6:
			// prep for attack 1.0
			e->enemy.health = e->enemy.maxhealth;
			e->enemy.goto_death = 8;
			e->base.ctr[2] = 0;
			e->base.ctr[1]++;
			e->base.vx = 0;
			e->base.vy = 0;

			// * FALL THROUGH *
		case 7:
			// attack 1.0
			e->base.ctr[0] = DIFF(20, 14, 10);
			e->base.ctr[1]--;
			if((e->base.ctr[2]%DIFF(3, 4, 6)) != 0)
			for(i = 0; i < 8; i++)
			{
				int spd = (j+3)*DIFF(0x80/3, 0x100/3, 0x280/3);
				int ang = i*(256/8) + j*DIFF(0, 256/8/2, 256/8/2)
					+ e->base.vy*DIFF(5, 5, 5);
				int ang0 = ang - 2;
				int ang1 = ang + 2;
				fed12 vx0 = (sintab[(ang0+0x00)&255]*spd)>>(14-12+8);
				fed12 vy0 = (sintab[(ang0-0x40)&255]*spd)>>(14-12+8);
				fed12 vx1 = (sintab[(ang1+0x00)&255]*spd)>>(14-12+8);
				fed12 vy1 = (sintab[(ang1-0x40)&255]*spd)>>(14-12+8);
				ent *ve0 = ent_new_bullet_glide(e, ENT_EBULLET, e->base.x, e->base.y, vx0, vy0, 3, 1, 0, 4);
				ent *ve1 = ent_new_bullet_glide(e, ENT_EBULLET, e->base.x, e->base.y, vx1, vy1, 3, 1, 0, 4);
				ve0->base.oamdata[2] |= (1<<10);
				ve1->base.oamdata[2] |= (1<<10);
			}

			if((e->base.ctr[2]%DIFF(3,4,6)) == 2)
			{
				fed12 avx = (e_player->base.x - e->base.x)>>6;
				fed12 avy = (e_player->base.y - e->base.y)>>6;
				normalise(&avx, &avy, DIFF(0x1500, 0x1A00, 0x1E00));
				for(i = -DIFF(0, 1, 2); i <= DIFF(0, 1, 2); i++)
				{
					fed12 vs = sintab[0x00 + DIFF(10, 6, 4)*i]>>(14-12);
					fed12 vc = sintab[0x40 + DIFF(10, 6, 4)*i]>>(14-12);
					fed12 nvx = (avx*vc - avy*vs)>>12;
					fed12 nvy = (avx*vs + avy*vc)>>12;
					ent_new_bullet_glide(e, ENT_EBULLET, e->base.x, e->base.y, nvx, nvy, 1, 1, 0, 4);
				}
				f3m_sfx_play(&player, CHN_ESHOT, F3M_PRIO_NORMAL, s_eshot, s_eshot_len, 0, 32768, 192);
			} else {
				f3m_sfx_play(&player, CHN_ESHOT, F3M_PRIO_NORMAL, s_eshot, s_eshot_len, 0, 32768, 96);

			}

			e->base.ctr[2]++;

			// abuse music for direction >:D
			e->base.vy += ((player.cord & 1) ? 1 : -1);
			break;

		case 8:
			// death after attack 1.0
			// health change
			blend_bg_pal(256, 32, 32);
			e->enemy.tx = 120;
			e->enemy.ty = 40;
			e->base.ctr[0] = 50;
			e->enemy.health = -1;
			e->enemy.maxhealth = 100;
			e->base.ctr[2] = 0;
			break;

		case 9:
			// end regen for 2.0 health
			e->base.ctr[0] = 120;
			break;

		case 10:
			// prep for attack 2.0
			e->enemy.health = e->enemy.maxhealth;
			e->enemy.goto_death = 200;
			e->base.ctr[2] = 0;
			e->base.ctr[1]++;
			e->base.vx = 0;
			e->base.vy = 0;

			// * FALL THROUGH *
		case 11:
			// attack 2.0
			e->base.ctr[0] = 60;
			e->base.ctr[2]++;

			e->enemy.tx = 120 + ((40*sintab[(e->base.vx*(256*2/5) + 0x00)&255])>>14);
			e->enemy.ty =  55 + ((15*sintab[(e->base.vx*(256*2/5) - 0x40)&255])>>14);

			e->base.vx++;
			if(e->base.vx >= 5)
				e->base.vx = 0;
			e->base.ctr[1]--;
			break;

		case 200:
			// boss dead
			f3m_sfx_play(&player, CHN_EDEAD, F3M_PRIO_NORMAL, s_bdead, s_bdead_len, 0, 32768, 127);
			f3m_sfx_play(&player, CHN_ESHOT, F3M_PRIO_NORMAL, s_bdead, s_bdead_len, 0, 32768, 127);
			f3m_sfx_play(&player, CHN_EHITX, F3M_PRIO_NORMAL, s_bdead, s_bdead_len, 0, 32768, 127);
			ent_kill(e);
			blend_bg_pal(256, 256, 256);
			break;

		case 201:
			e->base.ctr[0] = 240;
			e->base.ctr[1]--;
			break;
	}
}

void lv01_mainboss_hit(ent *e, ent *other, int dmg)
{
	if(e->enemy.health <= 0)
		return;

	e->enemy.health -= dmg;
	if(e->enemy.health <= 0)
	{
		int i;

		// kill all my bullets
		for(i = 0; i < ENT_MAX; i++)
		{
			ent *oe = &entlist[i];

			if(oe->base.typ == ENT_NONE)
				continue;
			if(oe->base.parent != e)
				continue;

			ent_free(oe);
		}

		e->base.ctr[2] = 0;
		e->base.ctr[1] = e->enemy.goto_death;
		e->base.ctr[0] = 0;
		f3m_sfx_play(&player, CHN_EDEAD, F3M_PRIO_NORMAL, s_edead, s_edead_len, 0, 32768/2, 127);
		f3m_sfx_play(&player, CHN_ESHOT, F3M_PRIO_NORMAL, s_edead, s_edead_len, 0, 32768/2, 127);
		f3m_sfx_play(&player, CHN_EHITX, F3M_PRIO_NORMAL, s_edead, s_edead_len, 0, 32768/2, 127);
		score_inc(0x1000);
	} else if(e->enemy.health < 25) {
		score_inc(0x1);
		f3m_sfx_play(&player, CHN_EHITX, F3M_PRIO_NORMAL, s_ehit2, s_ehit2_len, 0, 32768, 128);
	} else {
		score_inc(0x1);
		f3m_sfx_play(&player, CHN_EHITX, F3M_PRIO_NORMAL, s_ehit1, s_ehit1_len, 0, 32768, 128);
	}
}

ent *lv01_new_mainboss(void)
{
	ent *e = ent_new(NULL, F12N(0), F12N(-160), ENT_ENEMY, 0, 8, 10);
	e->enemy.health = 0;
	e->enemy.maxhealth = 100;
	e->enemy.radius = 7;
	e->base.f_tick = lv01_mainboss_tick;
	e->base.f_hit = lv01_mainboss_hit;

	return e;
}

void lv01_controller_tick(ent *e)
{
	int i, j;
	int incr;
	mod_s *mod;

	// ctr[0]: countdown for next event
	// ctr[1]: current event
	// ctr[2]: repetition count for current event

	e->base.x += e->base.vx;
	e->base.y += e->base.vy;
	BG1HOFS = ((-e->base.x)>>12) & 0x0FF;
	BG1VOFS = ((-e->base.y)>>12) & 0x0FF;

	if(--e->base.ctr[0] > 0)
		return;

	switch(e->base.ctr[1]++)
	{
		case 0:
			// Prep for warming background up
			e->base.ctr[2] = 0;

			// Also spam BG1 with crap
			i = 0xBEEF;
			for(j = 0; j < 32*32; j++)
			{
				VRAM0D[0x7800 + j] = (i % 6) + 0x40;
				if(i&1)
					i ^= 0x9000<<1;
				i >>= 1;
			}

			// *** FALL THROUGH ***
		case 1:
			// Warm background up
			e->base.ctr[2]++;
			i = ((e->base.ctr[2]*((32+3/2)/3))>>2);
			blend_bg_pal(i, i, i);

			//e->base.ctr[1] = 200; // SKIP
			if(e->base.ctr[2] < 64)
				e->base.ctr[1] = 1;
			break;

		case 2:
			// Window before title appearance
			blend_bg_pal(256, 256, 256);
			e->base.ctr[0] = 30;
			break;

		case 3:
			// Spawn title card generator
			// TODO!
			//VPAL0[0] = 31<<10;
			e->base.ctr[0] = 285;
			break;

		case 4:
			// Wave 0.0 prep
			e->base.ctr[2] = -4;

			e->base.ctr[1]++;

			// *** FALL THROUGH ***
		case 5:
			// Wave 0.0
			//VPAL0[0] = 31;
			incr = DIFF(4,2,1);
			
			i = e->base.ctr[2];
			if(!(i&(incr-1)))
			{
				lv01_new_enemy01(
					240/2 + ((100*(int)sintab[(i*16+0x00)&0xFF])>>15),
					160/2 - ((30*(int)sintab[(i*16+0x40)&0xFF])>>15),
					F12N((i&incr)?-1:1), F12N(-1)>>1);
			}

			e->base.ctr[0] = 10;
			e->base.ctr[2]++;
			if(e->base.ctr[2] <= 4)
				e->base.ctr[1]--;
			break;

		case 6:
			// Gap after wave 0.0
			e->base.ctr[0] = 300;
			break;

		case 7:
			// Wave 1.0
			e->base.ctr[0] = 95;
			for(i = 0; i < 0x40; i += DIFF(0x40/5, 0x40/7, 0x40/10))
				lv01_new_enemy02(0, -30, F12N(0x40 - i), F12N(0x90), 160);
			break;

		case 8:
			// Wave 1.1
			e->base.ctr[0] = 95;
			for(i = 0; i < 0x40; i += DIFF(0x40/5, 0x40/7, 0x40/10))
				lv01_new_enemy02(240, -30, -F12N(0x40 - i), -F12N(0x90), 160);
			break;

		case 9:
			// Wave 1.2
			e->base.ctr[0] = 65;
			for(i = 0; i < 0x40; i += DIFF(0x40/5, 0x40/7, 0x40/10))
				lv01_new_enemy02(120, -180, F12N(0x60 - i), F12N(0xC0), 220);
			break;

		case 10:
			// Wave 1.3
			e->base.ctr[0] = 126+96+96;
			for(i = 0; i < 0x40; i += DIFF(0x40/5, 0x40/7, 0x40/10))
				lv01_new_enemy02(120, -180, -F12N(0x60 - i), -F12N(0xC0), 220);
			break;

		case 11:
			// Wave 2.0 prep

			e->base.ctr[2] = 0;
			e->base.ctr[1]++;

			// *** FALL THROUGH ***

		case 12:
			e->base.ctr[0] = 24;

			//if((e->base.ctr[2] & DIFF(3, 1, 0)) == 0)
			{
				lv01_new_enemy01(
					240/2 + (e->base.ctr[2]-4)*(80/4),
					70, 
					F12N(((e->base.ctr[2]&7) >= 4)?-1:1), F12N(1)>>1);
			}

			e->base.ctr[2]++;
			e->base.vy >>= 1;
			if(e->base.ctr[2] < 8+1)
			{
				e->base.ctr[1]--;
				break;
			} else {
				e->base.ctr[1]++;

				// *** FALL THROUGH ***
			}

		case 13:
			e->base.ctr[0] = 96*4;
			e->base.vx = 0x800;
			e->base.vy = 0x1000;
			break;

		case 14:
			e->base.ctr[1] = 200+1;

			// *** FALL THROUGH ***
		case 200:
			// Boss fight
			lv01_new_mainboss();
			blend_bg_pal(256, 32, 32);
			fs_get_must("mus02   ", (void **)&mod, NULL);
			f3m_player_init(&player, mod);
			e->base.ctr[0] = 600;
			break;

		case 201:
			// TODOland
			e->base.ctr[0] = 600;
			e->base.ctr[1]--;
			break;

		default:
			VPAL0[0] = 0xFFF;
			for(;;) {}
			break;
	}
}

ent *lv01_new_controller(void)
{
	ent *e = ent_new(NULL, F12N(240/2), F12N(160+32/2), ENT_GFX, -1, -1, 0);
	e->base.f_tick = lv01_controller_tick;
	e->base.vx = 0;
	e->base.vy = 0x1000;

	return e;
}

