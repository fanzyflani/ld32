ent *ent_new_player(void);

void normalise(fed12 *x, fed12 *y, fed12 amp)
{
	int i;

	fed12 vx = *x;
	fed12 vy = *y;
	fed12 vxl = 0;
	fed12 vyl = 0;
	fed12 vxh = *x;
	fed12 vyh = *y;
	fed12 vamp = amp*amp;
	for(i = 0; i < 10; i++)
	{
		if(vx*vx + vy*vy > vamp)
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

	*x = vx;
	*y = vy;
}

void ent_reset(ent *e, ent *parent, fed12 spawn_x, fed12 spawn_y, int typ, int tilex, int tiley, int sz)
{
	e->base.typ = typ;
	e->base.x = spawn_x;
	e->base.y = spawn_y;
	e->base.vx = F12N(0);
	e->base.vy = F12N(0);
	if(tilex == -1)
		e->base.oamidx |= 0x8000;
	else
		e->base.oamidx &= ~0x8000;
	e->base.oamdata[0] = (1<<13) | (((sz>>2)&3)<<14);
	e->base.oamdata[1] = ((sz&3)<<14);
	e->base.oamdata[2] = ((tilex&15)<<1) | ((tiley&31)<<5);
	e->base.ctr[0] = 0;
	e->base.ctr[1] = 0;
	e->base.ctr[2] = 0;
	e->base.parent = parent;
}

ent *ent_new(ent *parent, fed12 spawn_x, fed12 spawn_y, int typ, int tilex, int tiley, int sz)
{
	if(entlist_free_start >= ENT_MAX || entlist[entlist_free_start].base.typ != ENT_NONE)
	{
		VPAL0[0] = 0xFFF;
		for(;;) {}
	}

	ent *e = &entlist[entlist_free_start];
	ent_reset(e, parent, spawn_x, spawn_y, typ, tilex, tiley, sz);

	while(entlist_free_start < ENT_MAX && entlist[entlist_free_start].base.typ != ENT_NONE)
		entlist_free_start++;

	while(entlist_used_end < ENT_MAX && entlist[entlist_used_end].base.typ != ENT_NONE)
		entlist_used_end++;

	return e;
}

void ent_free(ent *e)
{
	if(e->base.typ == ENT_NONE)
	{
		VPAL0[0] = 0xFFF;
		for(;;) {}
	}

	e->base.typ = ENT_NONE;
	e->base.oamidx |= 0x8000;
	e->base.f_tick = NULL;
	e->base.f_hit = NULL;
	int eidx = e->base.oamidx & 0x7FFF;

	if(eidx < entlist_free_start)
		entlist_free_start = eidx;
	while(entlist_used_end >= 1 && entlist[entlist_used_end-1].base.typ == ENT_NONE)
		entlist_used_end--;
}

int ent_off_screen(ent *e, int lenience, int *rsx, int *rsy)
{
	// Convert position
	int32_t sx = INTF12R(e->base.x);
	int32_t sy = INTF12R(e->base.y);

	// Get size
	int sz0 = ((e->base.oamdata[0])>>14)&3;
	int sz1 = ((e->base.oamdata[1])>>14)&3;
	int szx = sztab[sz0][sz1][0]>>1;
	int szy = sztab[sz0][sz1][1]>>1;

	// Centre sprite
	sx -= szx;
	sy -= szy;

	// Return sx, sy
	if(rsx != NULL) *rsx = sx;
	if(rsy != NULL) *rsy = sy;

	// Check if off-screen
	return (sx + szx < 0-lenience || sy + szy < 0-lenience
		|| sx - szx >= 240+lenience || sy - szy >= 160+lenience);
}

int ent_try_hit_one(ent *e, ent *target, int radius, int dmg)
{
	// Skip if none
	if(target->base.typ == ENT_NONE)
		return 0;

	// Get distance
	int dx = (INTF12R(target->base.x) - INTF12R(e->base.x));
	int dy = (INTF12R(target->base.y) - INTF12R(e->base.y));
	int d2 = dx*dx + dy*dy;

	if(d2 <= radius*radius)
	{
		if(target->base.f_hit)
			target->base.f_hit(target, e, dmg);

		return 1;
	}

	return 0;
}

int ent_try_hit_all(ent *e, int radius, int dmg)
{
	int i;
	ent *target;

	int acc = 0;

	for(i = 0; i < entlist_used_end; i++)
	{
		target = &entlist[i];

		if(target->base.typ == ENT_NONE)
			continue;

		acc += ent_try_hit_one(e, target, radius, dmg);
	}

	return acc;
}

int ent_try_hit_enemy(ent *e, int radius, int dmg)
{
	int i;
	ent *target;

	int acc = 0;

	for(i = 0; i < entlist_used_end; i++)
	{
		target = &entlist[i];

		if(target->base.typ != ENT_ENEMY)
			continue;

		acc += ent_try_hit_one(e, target, radius + target->enemy.radius, dmg);
	}

	return acc;
}


//
// BULLET: glide
//
void f_tick_bullet_glide(ent *e)
{
	e->base.x += e->base.vx;
	e->base.y += e->base.vy;

	if(e->base.typ == ENT_EBULLET)
	{
		if(ent_try_hit_one(e, e_player, e->bullet.radius, 1))
		{
			ent_free(e);
			return;
		}
	} else if(e->base.typ == ENT_PBULLET) {
		if(ent_try_hit_enemy(e, e->bullet.radius, 1))
		{
			ent_free(e);
			return;
		}
	}

	if(ent_off_screen(e, e->bullet.radius, NULL, NULL))
	{
		ent_free(e);
		return;
	}
}

ent *ent_new_bullet_glide(ent *parent, int typ, fed12 x, fed12 y, fed12 vx, fed12 vy, int tilex, int tiley, int sz, int radius)
{
	ent *e = ent_new(parent, x, y, typ, tilex, tiley, sz);
	e->base.vx = vx;
	e->base.vy = vy;
	e->base.f_tick = f_tick_bullet_glide;
	e->bullet.radius = radius;

	return e;
}

//
// BULLET: accel
//

// TODO!

//
// ENEMY
//

void f_enemy_hit(ent *e, ent *other, int dmg)
{
	if(other->base.typ == ENT_PBULLET)
	{
		e->enemy.health -= dmg;
		if(e->enemy.health <= 0)
		{
			// TODO: kill properly
			ent_free(e);
		}
	}
}

//
// PLAYER
//

void f_hit_player(ent *e, ent *other, int dmg)
{
	switch(other->base.typ)
	{
		case ENT_ITPOWER:
			// Raise power level
			// TODO!
			return;

		case ENT_EBULLET:
		case ENT_ENEMY:
			// Return if we have immunity
			if(e->base.ctr[1] > 0)
				return;

			// Kill player
			// TODO: do this properly
			// (right now we're just doing a soft reset)
			ent_free(e);
			lives--;
			if(lives < 0)
			{
				// RESET
				IME = 0;
				((void (*)(void))0x08000000)();
				return;
			}

			e_player = ent_new_player();
			break;
	}
}

void f_tick_player(ent *e)
{
	int i;

	// ctr[0]: shot fire counter
	// ctr[1]: respawn immunity counter

	// show HUD
	VRAM0D[0x7C00 + 32*1 + 0] = 0x01;
	VRAM0D[0x7C00 + 32*1 + 1] = 0x02;
	VRAM0D[0x7C00 + 32*1 + 2] = 0x03;
	for(i = 0; i < SCORE_DIGITS; i++)
		VRAM0D[0x7C00 + 32*1 + 3 + i] = 0x30 + score[i];

	VRAM0D[0x7C00 + 32*3 + 0] = 0x11;
	VRAM0D[0x7C00 + 32*3 + 1] = 0x12;
	VRAM0D[0x7C00 + 32*3 + 2] = 0x13;
	for(i = 0; i < bombs; i++)
		VRAM0D[0x7C00 + 32*3 + 3 + i] = 0x10;
	VRAM0D[0x7C00 + 32*2 + 3 + bombs] = 0x00;

	VRAM0D[0x7C00 + 32*2 + 0] = 0x21;
	VRAM0D[0x7C00 + 32*2 + 1] = 0x22;
	VRAM0D[0x7C00 + 32*2 + 2] = 0x23;
	for(i = 0; i < lives; i++)
		VRAM0D[0x7C00 + 32*2 + 3 + i] = 0x20;
	VRAM0D[0x7C00 + 32*2 + 3 + lives] = 0x00;

	if(e->base.ctr[1] > 0)
	{
		e->base.ctr[1]--;
		if(e->base.ctr[1]&1) e->base.oamidx |= 0x8000;
		else e->base.oamidx &= ~0x8000;
	}

	if(e->base.y > F12N(160-8))
	{
		e->base.y -= F12N(1);
		return;
	}

	int is_focused = (keypad & KEY_B);

	fed12 mvspeed = (is_focused
		? F12N(1)
		: F12N(2));
	if(keypad & KEY_UP) e->base.y -= mvspeed;
	if(keypad & KEY_DOWN) e->base.y += mvspeed;
	if(keypad & KEY_LEFT) e->base.x -= mvspeed;
	if(keypad & KEY_RIGHT) e->base.x += mvspeed;

	if(e->base.x < F12N(8)) e->base.x = F12N(8);
	if(e->base.y < F12N(16)) e->base.y = F12N(16);
	if(e->base.x > F12N(240-8)) e->base.x = F12N(240-8);
	if(e->base.y > F12N(160-8)) e->base.y = F12N(160-8);

	if(e->base.ctr[0] > 0)
		e->base.ctr[0] -= 1;

	if((keypad & KEY_A) && e->base.ctr[0] <= 0)
	{
		e->base.ctr[0] = 60/10;

		int spread = (is_focused ? 32/16 : 32/8);
		for(i = -1; i <= 1; i += 2)
		{
			int vx = sintab[(i*spread + 0x00)&255] + e->base.vx;
			int vy = sintab[(i*spread - 0x40)&255] + e->base.vy;
			ent_new_bullet_glide(e, ENT_PBULLET, e->base.x, e->base.y, vx, vy, 1, 1, 0, 4);
		}
	}
}

ent *ent_new_player(void)
{
	ent *e = ent_new(NULL, F12N(240/2), F12N(160+32/2), ENT_PLAYER, 0, 2, 10);
	e->base.f_tick = f_tick_player;
	e->base.f_hit = f_hit_player;
	e->base.ctr[1] = 60*2;

	if(bombs < 2) bombs = 2;

	return e;
}

