void lv01_enemy01_tick(ent *e)
{
	int i;

	if(e->base.ctr[0] < 90)
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
			//e->base.ctr[1] = 4; // SKIP
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
			VPAL0[0] = 15<<10;
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
			}

			e->base.ctr[2]++;

			// abuse music for direction >:D
			e->base.vy += ((player.cord & 1) ? 1 : -1);
			break;

		case 8:
			// TODOland
			VPAL0[0] = 15;
			e->base.ctr[0] = 60;
			e->base.ctr[2] = 0;
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
	int i;
	int incr;
	mod_s *mod;

	// ctr[0]: countdown for next event
	// ctr[1]: current event
	// ctr[2]: repetition count for current event

	if(--e->base.ctr[0] > 0)
		return;

	switch(e->base.ctr[1]++)
	{
		case 0:
			// Prep for warming background up
			e->base.ctr[2] = 0;

			// *** FALL THROUGH ***
		case 1:
			// Warm background up
			e->base.ctr[2]++;
			VPAL0[0] = ((e->base.ctr[2]*((31+3/2)/3))>>6)*0x421;
			//e->base.ctr[1] = 11; // SKIP
			if(e->base.ctr[2] < 64)
				e->base.ctr[1] = 1;
			break;

		case 2:
			// Window before title appearance
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
			e->base.ctr[0] = 125+95;
			for(i = 0; i < 0x40; i += DIFF(0x40/5, 0x40/7, 0x40/10))
				lv01_new_enemy02(120, -180, -F12N(0x60 - i), -F12N(0xC0), 220);
			break;

		case 11:
			// Boss fight
			lv01_new_mainboss();
			VPAL0[0] = 15;
			fs_get_must("mus02   ", (void **)&mod, NULL);
			f3m_player_init(&player, mod);
			e->base.ctr[0] = 600;
			break;

		case 12:
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

	return e;
}

