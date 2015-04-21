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

#define F3M_FREQ 32768
#define F3M_BUFLEN 546
//define F3M_FREQ 16384
//define F3M_BUFLEN 273
#define F3M_CHNS 1
#define F3M_VCHNS 24

#ifdef SLOWMEM_SOURCE_FILE
#define slowmem_extern __attribute__((section(".slowmem")))
#else
#define slowmem_extern extern
#endif

#include <stdint.h>
#include "f3m.h"

typedef int32_t fed12;
#define F12N(x) ((x)<<12)
#define INTF12R(x) (((x)+0x800)>>12)

enum
{
	ENT_NONE = 0,
	ENT_GFX,

	ENT_PLAYER,
	ENT_ENEMY,
	ENT_PBULLET,
	ENT_EBULLET,

	ENT_ITPOWER,
	ENT_ITPOINT,
};

typedef struct entbase entbase_s;
typedef union ent ent;
struct entbase
{
	uint8_t typ;
	uint16_t oamidx; // &0x8000 == no sprite
	uint16_t oamdata[3];
	fed12 x, y, vx, vy;
	int32_t ctr[3];
	ent *parent;
	// TODO: find other things to shove in this window
	void (*f_tick)(ent *self);
	void (*f_hit)(ent *self, ent *attacker, int damage);
};

union ent
{
	entbase_s base;

	struct {
		entbase_s base;
		uint8_t power;
	} player;

	struct {
		entbase_s base;
		int16_t health;
		int16_t tx, ty;
		int16_t radius;
		int16_t maxhealth;
		int16_t goto_death;
	} enemy;

	struct {
		entbase_s base;
		uint8_t grazed;
		int16_t radius;
		fed12 ax, ay;
	} bullet;
};

#define ENT_MAX 256
slowmem_extern ent entlist[ENT_MAX];
slowmem_extern int32_t entlist_free_start, entlist_used_end;
slowmem_extern ent *e_player;

#define SCORE_DIGITS 10
slowmem_extern uint8_t score[SCORE_DIGITS];
slowmem_extern int16_t lives, bombs;

