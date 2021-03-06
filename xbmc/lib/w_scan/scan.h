/*
 * Simple MPEG/DVB parser to achieve network/service information without initial tuning data
 *
 * Copyright (C) 2006, 2007, 2008, 2009, 2010 Winfried Koehler 
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 * Or, point your browser to http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 *
 * The author can be reached at: handygewinnspiel AT gmx DOT de
 *
 * The project's page is http://wirbel.htpc-forum.de/w_scan/index2.html
 */

#ifndef __SCAN_H__
#define __SCAN_H__

#include <stdio.h>
#include <errno.h>
#include <time.h>
#include "extended_frontend.h"
#include <sys/types.h>
#include <stdint.h>
#include "list.h"
#include "descriptors.h"



/******************************************************************************
 * internal definitions.
 *****************************************************************************/

struct w_scan_flags {
	uint32_t	version;
	fe_type_t	fe_type;
	uint8_t		atsc_type;
	uint8_t		need_2g_fe;
	uint32_t	list_id;
	uint8_t		tuning_timeout;
	uint8_t		filter_timeout;
	uint8_t		get_other_nits;
	uint8_t		add_frequencies;
	uint8_t		dump_provider;
	uint8_t		vdr_version;
	uint8_t		qam_no_auto;
	uint8_t		ca_select;
	int		rotor_position;
	uint16_t	api_version;
	uint16_t	sw_pos;
        uint16_t        codepage;
};

#define AUDIO_CHAN_MAX (32)
#define AC3_CHAN_MAX (32)
#define CA_SYSTEM_ID_MAX (16)



struct service {
	struct list_head list;
	int transport_stream_id;
	int service_id;
	char *provider_name;
	char *provider_short_name;
	char *service_name;
	char *service_short_name;
	uint16_t pmt_pid;
	uint16_t pcr_pid;
	uint16_t video_pid;
	uint16_t audio_pid[AUDIO_CHAN_MAX];
	char audio_lang[AUDIO_CHAN_MAX][4];
	int audio_num;
	uint16_t ca_id[CA_SYSTEM_ID_MAX];
	int ca_num;
	uint16_t teletext_pid;
	uint16_t subtitling_pid;
	uint16_t ac3_pid[AC3_CHAN_MAX];
	char ac3_lang[AC3_CHAN_MAX][4];
	int ac3_num;
	unsigned int type	: 8;
	unsigned int scrambled	: 1;
	enum running_mode running;
	void *priv;
	int channel_num;
	int is_last_audio_ac3;
};

struct transponder_ids {
	int network_id;
	int original_network_id;		/* onid patch by Hartmut Birr */
	int transport_stream_id; 
};

struct transponder {
	struct list_head list;
	struct list_head services;
	struct transponder_ids pids;
	enum fe_type type;
	struct extended_dvb_frontend_parameters param;
	unsigned int scan_done:1;
	unsigned int last_tuning_failed:1;
	unsigned int other_frequency_flag:1;	/* DVB-T */
	int n_other_f;
	int updated_by_nit;
	uint32_t *other_f;			/* DVB-T frequency-list descriptor */
	char *network_name;
};



/* some basic logging facility
 * made of macros. ugly, but working fine.
 */

extern int verbosity;

int run_time(); // 
void hexdump(const char * intro, const unsigned char * buf, int len);

#define dprintf(level, fmt...)			\
	do {					\
		if (level <= verbosity)	{	\
			fprintf(stderr, fmt); }	\
	} while (0)

#define dpprintf(level, fmt, args...) \
	dprintf(level, "%s:%d: " fmt, __FUNCTION__, __LINE__ , ##args)

#define fatal(fmt, args...) do { dpprintf(-1, "FATAL: " fmt , ##args); exit(1); } while(0)
#define error(msg...) dprintf(0, "\nERROR: " msg)
#define errorn(msg) dprintf(0, "%s:%d: ERROR: " msg ": %d %s\n", __FUNCTION__, __LINE__, errno, strerror(errno))
#define warning(msg...) dprintf(1, "WARNING: " msg)
#define info(msg...) dprintf(2, msg)
#define verbose(msg...) dprintf(3, msg)
#define moreverbose(msg...) dprintf(4, msg)
#define debug(msg...) dpprintf(5, msg)
#define verbosedebug(msg...) dpprintf(6, msg)



struct transponder *alloc_transponder(uint32_t frequency);

/* write transponder data to dest. no memory allocating,
 * so dest has to be big enough - think about before use!
 */
void print_transponder(char * dest, struct transponder * t);


#endif
