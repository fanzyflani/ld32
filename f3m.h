#ifndef _HDR_F3M_H
#define _HDR_F3M_H
#include <stdint.h>

#ifndef F3M_FREQ
#define F3M_FREQ 32768
//define F3M_FREQ 16384
#endif
#ifndef F3M_BUFLEN
#define F3M_BUFLEN 546
//define F3M_BUFLEN 273
#endif
#ifndef F3M_CHNS
#define F3M_CHNS 1
#endif
#ifndef F3M_VCHNS
#define F3M_VCHNS 20
#endif
#define F3M_PRIO_NORMAL 50
#define F3M_PRIO_MUSIC_OFF 100
#define F3M_PRIO_MUSIC 0x7FFF

typedef struct ins
{
	uint8_t typ;
	uint8_t fname[12];
	uint8_t dat_para_h;
	uint16_t dat_para;
	uint32_t len, lpbeg, lpend;
	uint8_t vol, rsv1, pack, flags;
	uint32_t c4freq;
	uint8_t rsv2[12];
	uint8_t name[28];
	uint8_t magic[4];
} __attribute__((__packed__)) ins_s;

typedef struct mod
{
	uint8_t name[28];
	uint8_t magic[4];
	uint16_t ord_num, ins_num, pat_num;
	uint16_t flags, ver, smptyp;
	uint8_t magic_scrm[4];
	uint8_t gvol, ispeed, itempo, mvol;
	uint8_t uclick, defpanFC;
	uint8_t unused1[8];
	uint16_t special;
	uint8_t cset[32];
	uint8_t extra[];
}__attribute__((__packed__)) mod_s;

typedef struct vchn
{
	const uint8_t *data;
	int32_t len;
	int32_t len_loop;

	int32_t period;
	int32_t gxx_period;

	int32_t freq;
	int32_t offs;
	uint16_t suboffs;
	int16_t priority;

	int8_t vol;

	uint8_t vib_offs;
	uint8_t rtg_count;

	uint8_t eft, efp, lefp, last_note;
	uint8_t lins;
	uint8_t mem_gxx, mem_hxx, mem_oxx;
} vchn_s;

typedef struct player
{
	const mod_s *mod;
	const void *modbase;
	const uint16_t *ins_para;
	const uint16_t *pat_para;
	const uint8_t *ord_list;

	int32_t speed, tempo;
	int32_t ctick, tempo_samples, tempo_wait;
	int32_t cord, cpat, crow;
	const uint8_t *patptr;

	int sfxoffs;
	int ccount;

	vchn_s vchn[F3M_VCHNS];
} player_s;
#endif

