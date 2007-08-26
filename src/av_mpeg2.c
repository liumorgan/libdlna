/*
 * libdlna: reference DLNA standards implementation.
 * Copyright (C) 2007 Benjamin Zores <ben@geexbox.org>
 *
 * This file is part of libdlna.
 *
 * libdlna is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * libdlna is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with libdlna; if not, write to the Free Software
 * Foundation, Inc, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "dlna.h"
#include "profiles.h"
#include "containers.h"

#define MPEG2_KNOWN_EXTENSIONS "mpg,mpeg,mpe,m2v,mp2p,mp2t,ts,ps,pes"
#define MPEG2_MIME_TYPE "video/mpeg"
#define MPEG2_TS_DLNA_MIME_TYPE "video/vnd.dlna.mpeg-tts"
#define MPEG2_LABEL_CIF30 "CIF30"
#define MPEG2_LABEL_SD "SD"
#define MPEG2_LABEL_HD "HD"

typedef struct mpeg_ps_es_stream_s {
  int width;
  int height;
} mpeg_ps_es_stream_t;

typedef struct mpeg_ts_stream_s {
  int width;
  int height;
  int fps_num;
  int fps_den;
} mpeg_ts_stream_t;

static mpeg_ps_es_stream_t mpeg_ps_es_valid_streams_ntsc[] = {
  { 720, 480 },
  { 704, 480 },
  { 544, 480 },
  { 480, 480 },
  { 352, 480 },
  { 352, 240 }
};

static mpeg_ps_es_stream_t mpeg_ps_es_valid_streams_pal[] = {
  { 720, 576 },
  { 704, 576 },
  { 544, 576 },
  { 480, 576 },
  { 352, 576 },
  { 352, 288 }
};

static mpeg_ts_stream_t mpeg_ts_valid_streams_eu_sd[] = {
  { 720, 576, 25, 1},
  { 544, 576, 25, 1},
  { 480, 576, 25, 1},
  { 352, 576, 25, 1},
  { 252, 288, 25, 1}
};

static mpeg_ts_stream_t mpeg_ts_valid_streams_na_sd[] = {
  { 720, 480, 30, 1001},
  { 704, 480, 30, 1001},
  { 704, 480, 30, 1},
  { 704, 480, 24, 1001},
  { 704, 480, 24, 1},
  { 640, 480, 30, 1001},
  { 640, 480, 30, 1},
  { 640, 480, 24, 1001},
  { 640, 480, 24, 1},
  { 544, 480, 30, 1001},
  { 480, 480, 30, 1001},
  { 352, 480, 30, 1001}
};

static mpeg_ts_stream_t mpeg_ts_valid_streams_na_hd[] = {
  { 1920, 1080, 30, 1001},
  { 1920, 1080, 30, 1},
  { 1920, 1080, 24, 1001},
  { 1920, 1080, 24, 1},
  { 1280, 720, 30, 1001},
  { 1280, 720, 30, 1},
  { 1280, 720, 24, 1001},
  { 1280, 720, 24, 1},
  { 1440, 1080, 30, 1001},
  { 1440, 1080, 30, 1},
  { 1440, 1080, 24, 1001},
  { 1440, 1080, 24, 1},
  { 1280, 1080, 30, 1001},
  { 1280, 1080, 30, 1},
  { 1280, 1080, 24, 1001},
  { 1280, 1080, 24, 1}
};

/* Profile for NTSC-formatted AV class media */
static dlna_profile_t mpeg_ps_ntsc = {
  .id = "MPEG_PS_NTSC",
  .mime = MPEG2_MIME_TYPE,
  .label = MPEG2_LABEL_SD
};

/* Profile for NTSC-formatted AV class media */
static dlna_profile_t mpeg_ps_ntsc_xac3 = {
  .id = "MPEG_PS_NTSC_XAC3",
  .mime = MPEG2_MIME_TYPE,
  .label = MPEG2_LABEL_SD
};

/* Profile for PAL-formatted AV class media */
static dlna_profile_t mpeg_ps_pal = {
  .id = "MPEG_PS_PAL",
  .mime = MPEG2_MIME_TYPE,
  .label = MPEG2_LABEL_SD
};

/* Profile for PAL-formatted AV class media */
static dlna_profile_t mpeg_ps_pal_xac3 = {
  .id = "MPEG_PS_PAL_XAC3",
  .mime = MPEG2_MIME_TYPE,
  .label = MPEG2_LABEL_SD
};

/* MPEG-2 Main Profile at Low Level AAC LC audio encapsulated in
   MPEG-2 transport stream with zero value timestamp */
static dlna_profile_t mpeg_ts_mp_ll_aac = {
  .id = "MPEG_TS_MP_LL_AAC",
  .mime = MPEG2_TS_DLNA_MIME_TYPE,
  .label = MPEG2_LABEL_CIF30
};

/* MPEG-2 Main Profile at Low Level AAC LC audio encapsulated in
   MPEG-2 transport stream with valid value timestamp */
static dlna_profile_t mpeg_ts_mp_ll_aac_t = {
  .id = "MPEG_TS_MP_LL_AAC_T",
  .mime = MPEG2_TS_DLNA_MIME_TYPE,
  .label = MPEG2_LABEL_CIF30
};

/* MPEG-2 Main Profile at Low Level AAC LC audio encapsulated in
   MPEG-2 transport stream without a Timestamp field */
static dlna_profile_t mpeg_ts_mp_ll_aac_iso = {
  .id = "MPEG_TS_MP_LL_AAC_ISO",
  .mime = MPEG2_MIME_TYPE,
  .label = MPEG2_LABEL_CIF30
};

/* European region profile for Standard Definition AV class utilizing
   a DLNA Transport Packet with zero value timestamp */
static dlna_profile_t mpeg_ts_sd_eu = {
  .id = "MPEG_TS_SD_EU",
  .mime = MPEG2_TS_DLNA_MIME_TYPE,
  .label = MPEG2_LABEL_SD
};

/* European region profile for Standard Definition AV class utilizing
   a DLNA Transport Packet with a valid non-zero value timestamp */
static dlna_profile_t mpeg_ts_sd_eu_t = {
  .id = "MPEG_TS_SD_EU_T",
  .mime = MPEG2_TS_DLNA_MIME_TYPE,
  .label = MPEG2_LABEL_SD
};

/* European region profile for Standard Definition AV class utilizing
   a DLNA Transport Packet without a Timestamp field */
static dlna_profile_t mpeg_ts_sd_eu_iso = {
  .id = "MPEG_TS_SD_EU_ISO",
  .mime = MPEG2_MIME_TYPE,
  .label = MPEG2_LABEL_SD
};

/* North America region profile for Standard Definition AV class utilizing
   a DLNA Transport Packet with zero value timestamp */
static dlna_profile_t mpeg_ts_sd_na = {
  .id = "MPEG_TS_SD_NA",
  .mime = MPEG2_TS_DLNA_MIME_TYPE,
  .label = MPEG2_LABEL_SD
};

/* North America region profile for Standard Definition AV class utilizing
   a DLNA Transport Packet with a valid non-zero value timestamp */
static dlna_profile_t mpeg_ts_sd_na_t = {
  .id = "MPEG_TS_SD_NA_T",
  .mime = MPEG2_TS_DLNA_MIME_TYPE,
  .label = MPEG2_LABEL_SD
};

/* North America region profile for Standard Definition AV class utilizing
   a DLNA Transport Packet without a Timestamp field */
static dlna_profile_t mpeg_ts_sd_na_iso = {
  .id = "MPEG_TS_SD_NA_ISO",
  .mime = MPEG2_MIME_TYPE,
  .label = MPEG2_LABEL_SD
};

/* North America region profile for Standard Definition AV class utilizing
   a DLNA Transport Packet with zero value timestamp */
static dlna_profile_t mpeg_ts_sd_na_xac3 = {
  .id = "MPEG_TS_SD_NA_XAC3",
  .mime = MPEG2_TS_DLNA_MIME_TYPE,
  .label = MPEG2_LABEL_SD
};

/* North America region profile for Standard Definition AV class utilizing
   a DLNA Transport Packet with a valid non-zero value timestamp */
static dlna_profile_t mpeg_ts_sd_na_xac3_t = {
  .id = "MPEG_TS_SD_NA_XAC3_T",
  .mime = MPEG2_TS_DLNA_MIME_TYPE,
  .label = MPEG2_LABEL_SD
};

/* North America region profile for Standard Definition AV class utilizing
   a DLNA Transport Packet without a Timestamp field */
static dlna_profile_t mpeg_ts_sd_na_xac3_iso = {
  .id = "MPEG_TS_SD_NA_XAC3_ISO",
  .mime = MPEG2_MIME_TYPE,
  .label = MPEG2_LABEL_SD
};

/* North America region profile for High Definition AV class utilizing
   a DLNA Transport Packet with zero value timestamp */
static dlna_profile_t mpeg_ts_hd_na = {
  .id = "MPEG_TS_HD_NA",
  .mime = MPEG2_TS_DLNA_MIME_TYPE,
  .label = MPEG2_LABEL_HD
};

/* North America region profile for High Definition AV class utilizing
   a DLNA Transport Packet with a valid non-zero value timestamp */
static dlna_profile_t mpeg_ts_hd_na_t = {
  .id = "MPEG_TS_HD_NA_T",
  .mime = MPEG2_TS_DLNA_MIME_TYPE,
  .label = MPEG2_LABEL_HD
};

/* North America region profile for High Definition AV class utilizing
   a DLNA Transport Packet without a Timestamp field */
static dlna_profile_t mpeg_ts_hd_na_iso = {
  .id = "MPEG_TS_HD_NA_ISO",
  .mime = MPEG2_MIME_TYPE,
  .label = MPEG2_LABEL_HD
};

/* North America region profile for transcoded High Definition AV class
   media with a zero value timestamp */
static dlna_profile_t mpeg_ts_hd_na_xac3 = {
  .id = "MPEG_TS_HD_NA_XAC3",
  .mime = MPEG2_TS_DLNA_MIME_TYPE,
  .label = MPEG2_LABEL_HD
};

/* North America region profile for transcoded High Definition AV class
   media with a valid non-zero value timestamp */
static dlna_profile_t mpeg_ts_hd_na_xac3_t = {
  .id = "MPEG_TS_HD_NA_XAC3_T",
  .mime = MPEG2_TS_DLNA_MIME_TYPE,
  .label = MPEG2_LABEL_HD
};

/* North America region profile for transcoded High Definition AV class
   media without a Timestamp field */
static dlna_profile_t mpeg_ts_hd_na_xac3_iso = {
  .id = "MPEG_TS_HD_NA_XAC3_ISO",
  .mime = MPEG2_MIME_TYPE,
  .label = MPEG2_LABEL_HD
};

/* Profile defining ES encapsulation for transport of MPEG_PS_PAL over RTP */
static dlna_profile_t mpeg_es_pal = {
  .id = "MPEG_ES_PAL",
  .mime = MPEG2_MIME_TYPE,
  .label = MPEG2_LABEL_SD
};

/* Profile defining ES encapsulation for transport of MPEG_PS_NTSC over RTP */
static dlna_profile_t mpeg_es_ntsc = {
  .id = "MPEG_ES_NTSC",
  .mime = MPEG2_MIME_TYPE,
  .label = MPEG2_LABEL_SD
};

/* Profile defining ES encapsulation for transport of
   MPEG_PS_PAL_XAC3 over RTP */
static dlna_profile_t mpeg_es_pal_xac3 = {
  .id = "MPEG_ES_PAL_XAC3",
  .mime = MPEG2_MIME_TYPE,
  .label = MPEG2_LABEL_SD
};

/* Profile defining ES encapsulation for transport of
   MPEG_PS_NTSC_XAC3 over RTP */
static dlna_profile_t mpeg_es_ntsc_xac3 = {
  .id = "MPEG_ES_NTSC_XAC3",
  .mime = MPEG2_MIME_TYPE,
  .label = MPEG2_LABEL_SD
};

static int
is_mpeg_ps_es_audio_stream_lpcm (AVFormatContext *ctx, av_codecs_t *codecs)
{
  /* check for 16-bit signed network-endian PCM codec  */
  if (codecs->ac->codec_id != CODEC_ID_PCM_S16BE &&
      codecs->ac->codec_id != CODEC_ID_PCM_S16LE)
    return 0;

  /* sampling rate is 48 KHz */
  if (codecs->ac->sample_rate != 48000)
    return 0;

  /* channels mode: 1/0, 2/0, 1/0 + 1/0 */
  if (codecs->ac->channels > 2)
    return 0;
  
  /* audio bit rate: 1.536 Mbps for stereo, 768 Kbps for mono */
  if (codecs->ac->channels == 2 && codecs->ac->bit_rate > 1536000)
    return 0;
  if (codecs->ac->channels == 1 && codecs->ac->bit_rate > 768000)
    return 0;
  
  return 1;
}

static int
common_ac3_check (AVFormatContext *ctx, av_codecs_t *codecs)
{
  /* check for AC3 */
  if (codecs->ac->codec_id != CODEC_ID_AC3)
    return 0;

  /* sampling rate is 48 KHz */
  if (codecs->ac->sample_rate != 48000)
    return 0;

  /* supported channels: 1/0, 1/0 + 1/0, 2/0, 3/0, 2/1, 3/1, 2/2, 3/2 */
  if (codecs->ac->channels > 5)
    return 0;

  return 1;
}

static int
is_mpeg_ps_es_audio_stream_extended_ac3 (AVFormatContext *ctx,
                                      av_codecs_t *codecs)
{
  int res;
  
  res = common_ac3_check (ctx, codecs);
  if (!res)
    return 0;

  /* supported bitrate: 64 Kbps - 640 Kbps */
  if (codecs->ac->bit_rate < 64000 || codecs->ac->bit_rate > 640000)
    return 0;

  return 1;
}

static int
is_mpeg_ps_es_audio_stream_ac3 (AVFormatContext *ctx, av_codecs_t *codecs)
{
  int res;
  
  res = common_ac3_check (ctx, codecs);
  if (!res)
    return 0;

  /* supported bitrate: 64 Kbps - 448 Kbps */
  if (codecs->ac->bit_rate < 64000 || codecs->ac->bit_rate > 448000)
    return 0;
  
  return 1;
}

static int
is_mpeg_ps_es_audio_stream_mp2 (AVFormatContext *ctx, av_codecs_t *codecs)
{
  /* check for MPEG-1 Layer-2 audio codec */
  if (codecs->ac->codec_id != CODEC_ID_MP2 &&
      codecs->ac->codec_id != CODEC_ID_MP3)
    return 0;

  /* sampling rate is 44.1 or 48 KHz */
  if (codecs->ac->sample_rate != 44100 && codecs->ac->sample_rate != 48000)
    return 0;

  /* supported channels: 1/0, 1/0 + 1/0, 2/0 */
  if (codecs->ac->channels > 2)
    return 0;

  /* audio bit rate: 64-192 Kbps for mono, 64-384 Kbps for stereo */
  if (codecs->ac->channels == 1 &&
      (codecs->ac->bit_rate < 64000 || codecs->ac->bit_rate > 192000))
    return 0;
  if (codecs->ac->channels == 2 &&
      (codecs->ac->bit_rate < 64000 || codecs->ac->bit_rate > 384000))
    return 0;
  
  return 1;
}

static int
is_mpeg_ts_audio_stream_mp2 (AVFormatContext *ctx, av_codecs_t *codecs)
{
  /* check for MPEG-1 Layer-2 audio codec */
  if (codecs->ac->codec_id != CODEC_ID_MP2 &&
      codecs->ac->codec_id != CODEC_ID_MP3)
    return 0;

  /* sampling rate is 32, 44.1 or 48 KHz */
  if (codecs->ac->sample_rate != 32000 &&
      codecs->ac->sample_rate != 44100 &&
      codecs->ac->sample_rate != 48000)
    return 0;

  if (codecs->ac->channels > 5)
    return 0;

  /* from 32 to 448 Kbps */
  if (codecs->ac->bit_rate < 32000 || codecs->ac->bit_rate > 448000)
    return 0;
  
  return 1;
}

static int
is_mpeg_ts_audio_stream_ac3 (AVFormatContext *ctx, av_codecs_t *codecs)
{
  /* check for AC3 */
  if (codecs->ac->codec_id != CODEC_ID_AC3)
    return 0;

  /* sampling rate is 32, 44.1 or 48 KHz */
  if (codecs->ac->sample_rate != 32000 &&
      codecs->ac->sample_rate != 44100 &&
      codecs->ac->sample_rate != 48000)
    return 0;

  /* supported channels: 1/0, 1/0 + 1/0, 2/0, 3/0, 2/1, 3/1, 2/2, 3/2 */
  if (codecs->ac->channels > 5)
    return 0;

  /* supported bitrate: 32 Kbps - 640 Kbps */
  if (codecs->ac->bit_rate < 32000 || codecs->ac->bit_rate > 640000)
    return 0;
  
  return 1;
}

static dlna_profile_t *
probe_mpeg_ps_es (AVFormatContext *ctx, av_codecs_t *codecs,
                  dlna_profile_t *pal, dlna_profile_t *pal_xac3,
                  dlna_profile_t *ntsc, dlna_profile_t *ntsc_xac3)
{
  int i;

  /* determine region through frame rate */
  if ((codecs->vs->r_frame_rate.num == 30000 &&
       codecs->vs->r_frame_rate.den == 1001)) /* NTSC */
  {
    for (i = 0; i < sizeof (mpeg_ps_es_valid_streams_ntsc)
           / sizeof (mpeg_ps_es_stream_t); i++)
    {
      if (mpeg_ps_es_valid_streams_ntsc[i].width == codecs->vc->width &&
          mpeg_ps_es_valid_streams_ntsc[i].height == codecs->vc->height)
      {
        if (is_mpeg_ps_es_audio_stream_extended_ac3 (ctx, codecs))
          return set_profile (ntsc_xac3);
        else if (is_mpeg_ps_es_audio_stream_lpcm (ctx, codecs) ||
                 is_mpeg_ps_es_audio_stream_ac3 (ctx, codecs) ||
                 is_mpeg_ps_es_audio_stream_mp2 (ctx, codecs))
          return set_profile (ntsc);

        return NULL;
      }
    }

    /* invalid resolution */
    return NULL;
  }
  else if (codecs->vs->r_frame_rate.num == 25 &&
           codecs->vs->r_frame_rate.den == 1) /* PAL */
  {
    for (i = 0; i <  sizeof (mpeg_ps_es_valid_streams_pal)
           / sizeof (mpeg_ps_es_stream_t); i++)
    {
      if (mpeg_ps_es_valid_streams_pal[i].width == codecs->vc->width &&
          mpeg_ps_es_valid_streams_pal[i].height == codecs->vc->height)
      {
        if (is_mpeg_ps_es_audio_stream_extended_ac3 (ctx, codecs))
          return set_profile (pal_xac3);
        else if (is_mpeg_ps_es_audio_stream_lpcm (ctx, codecs) ||
                 is_mpeg_ps_es_audio_stream_ac3 (ctx, codecs) ||
                 is_mpeg_ps_es_audio_stream_mp2 (ctx, codecs))
          return set_profile (pal);

        return NULL;
      }
    }

    /* invalid resolution */
    return NULL;
  }

  return NULL;
}

static dlna_profile_t *
probe_mpeg_ps (AVFormatContext *ctx, av_codecs_t *codecs)
{
  return probe_mpeg_ps_es (ctx, codecs,
                           &mpeg_ps_pal, &mpeg_ps_pal_xac3,
                           &mpeg_ps_ntsc, &mpeg_ps_ntsc_xac3);
}

static dlna_profile_t *
probe_mpeg_es (AVFormatContext *ctx, av_codecs_t *codecs)
{
  return probe_mpeg_ps_es (ctx, codecs,
                           &mpeg_es_pal, &mpeg_es_pal_xac3,
                           &mpeg_es_ntsc, &mpeg_es_ntsc_xac3);
}

static dlna_profile_t *
probe_mpeg_ts (AVFormatContext *ctx,
               av_codecs_t *codecs, dlna_container_type_t st)
{
  int xac3 = 0; /* extended AC3 audio */
  int i;
  
  /* check for MPEG-2 MP@LL profile */
  if (codecs->ac->codec_id == CODEC_ID_AAC)
  {
    /* 352x288 only */
    if (codecs->vc->width != 352 && codecs->vc->height != 288)
      return NULL;

    /* 30 fps */
    if (codecs->vs->r_frame_rate.num != 30 &&
        codecs->vs->r_frame_rate.den != 1)
      return NULL;

    /* video bitrate is less or equal to 4 Mbps */
    if (codecs->vc->bit_rate > 4000000)
      return NULL;

    /* audio bitrate is less or equal to 256 Kbps */
    if (codecs->ac->bit_rate > 256000)
      return NULL;

    switch (st)
    {
    case CT_MPEG_TRANSPORT_STREAM:
      return set_profile (&mpeg_ts_mp_ll_aac_iso);
    case CT_MPEG_TRANSPORT_STREAM_DLNA:
      return set_profile (&mpeg_ts_mp_ll_aac_t);
    case CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS:
      return set_profile (&mpeg_ts_mp_ll_aac);
    default:
      return NULL;
    }
  }

  /* check for Region: only Europe supports 25 fps (50i) */
  if (codecs->vs->r_frame_rate.num == 25 &&
      codecs->vs->r_frame_rate.den == 1)
  {
    for (i = 0; i <  sizeof (mpeg_ts_valid_streams_eu_sd)
           / sizeof (mpeg_ts_stream_t); i++)
    {
      if (mpeg_ts_valid_streams_eu_sd[i].width == codecs->vc->width &&
          mpeg_ts_valid_streams_eu_sd[i].height == codecs->vc->height &&
          mpeg_ts_valid_streams_eu_sd[i].fps_num == 25 &&
          mpeg_ts_valid_streams_eu_sd[i].fps_den == 1)
      {
        if (is_mpeg_ts_audio_stream_ac3 (ctx, codecs) ||
            is_mpeg_ts_audio_stream_mp2 (ctx, codecs))
        {
          switch (st)
          {
          case CT_MPEG_TRANSPORT_STREAM:
            return set_profile (&mpeg_ts_sd_eu_iso);
          case CT_MPEG_TRANSPORT_STREAM_DLNA:
            return set_profile (&mpeg_ts_sd_eu_t);
          case CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS:
            return set_profile (&mpeg_ts_sd_eu);
          default:
            return NULL;
          }
        }

        /* invalid audio stream */
        return NULL;
      }
    }

    /* invalid resolution */
    return NULL;
  }

  /* now comes the stupid part: there's no way to differentiate region
     codes between North America (NA) and Korea (KO) as both have exactly
     the same requirements !! NA however supports additional stream formats
     so all streams will be declared as NA ones (which shouldn't bother
     the real KO ones). */

  /* NA and KO streams can be either SD (Standard Definition)
     or HD (High-Definition) and only support AC3 as audio stream codec */

  /* maximum system bit rate is 19.3927 Mb/s */
  if (ctx->bit_rate > 19392700)
    return NULL;

  if (codecs->ac->codec_id != CODEC_ID_AC3)
    return NULL;

  /* 48 KHz only */
  if (codecs->ac->sample_rate != 48000)
    return NULL;

  /* up to 5 audio channels */
  if (codecs->ac->channels > 5)
    return NULL;

  /* audio bitrate up to 448 Kbps (or 640 for extended AC3) */
  if (codecs->ac->bit_rate > 448000)
    xac3 = 1;
  if (codecs->ac->bit_rate > 640000)
    return NULL;

  /* look for compatible SD video stream */
  for (i = 0; i <  sizeof (mpeg_ts_valid_streams_na_sd)
         / sizeof (mpeg_ts_stream_t); i++)
  {
    if (mpeg_ts_valid_streams_na_sd[i].width == codecs->vc->width &&
        mpeg_ts_valid_streams_na_sd[i].height == codecs->vc->height &&
        mpeg_ts_valid_streams_na_sd[i].fps_num
        == codecs->vs->r_frame_rate.num &&
        mpeg_ts_valid_streams_na_sd[i].fps_den
        == codecs->vs->r_frame_rate.num)
    {
      switch (st)
      {
      case CT_MPEG_TRANSPORT_STREAM:
        return xac3 ? set_profile (&mpeg_ts_sd_na_xac3_iso)
          : set_profile (&mpeg_ts_sd_na_iso);
      case CT_MPEG_TRANSPORT_STREAM_DLNA:
        return xac3 ? set_profile (&mpeg_ts_sd_na_xac3_t)
          : set_profile (&mpeg_ts_sd_na_t);
      case CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS:
        return xac3 ? set_profile (&mpeg_ts_sd_na_xac3)
          : set_profile (&mpeg_ts_sd_na);
      default:
        return NULL;
      }
    }
  }

  /* look for compatible HD video stream */
  for (i = 0; i <  sizeof (mpeg_ts_valid_streams_na_hd)
         / sizeof (mpeg_ts_stream_t); i++)
  {
    if (mpeg_ts_valid_streams_na_hd[i].width == codecs->vc->width &&
        mpeg_ts_valid_streams_na_hd[i].height == codecs->vc->height &&
        mpeg_ts_valid_streams_na_hd[i].fps_num
        == codecs->vs->r_frame_rate.num &&
        mpeg_ts_valid_streams_na_hd[i].fps_den
        == codecs->vs->r_frame_rate.num)
    {
      switch (st)
      {
      case CT_MPEG_TRANSPORT_STREAM:
        return xac3 ? set_profile (&mpeg_ts_hd_na_xac3_iso)
          : set_profile (&mpeg_ts_hd_na_iso);
      case CT_MPEG_TRANSPORT_STREAM_DLNA:
        return xac3 ? set_profile (&mpeg_ts_hd_na_xac3_t)
          : set_profile (&mpeg_ts_hd_na_t);
      case CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS:
        return xac3 ? set_profile (&mpeg_ts_hd_na_xac3)
          : set_profile (&mpeg_ts_hd_na);
      default:
        return NULL;
      }
    }
  }

  /* no compliant resolution found */
  return NULL;
}

static dlna_profile_t *
probe_mpeg2 (AVFormatContext *ctx)
{
  av_codecs_t *codecs;
  dlna_profile_t *profile = NULL;
  dlna_container_type_t st;
  
  /* check for valid file extension */
  if (!match_file_extension (ctx->filename, MPEG2_KNOWN_EXTENSIONS))
    return NULL;

  codecs = av_profile_get_codecs (ctx);
  if (!codecs)
    goto probe_mpeg2_end;

  /* check for MPEG-2 video codec */
  if (codecs->vc->codec_id != CODEC_ID_MPEG2VIDEO)
    goto probe_mpeg2_end;

  st = stream_get_container (ctx);
  switch (st)
  {
  case CT_MPEG_ELEMENTARY_STREAM:
    profile = probe_mpeg_es (ctx, codecs);
    break;
  case CT_MPEG_PROGRAM_STREAM:
    profile = probe_mpeg_ps (ctx, codecs);
    break;
  case CT_MPEG_TRANSPORT_STREAM:
  case CT_MPEG_TRANSPORT_STREAM_DLNA:
  case CT_MPEG_TRANSPORT_STREAM_DLNA_NO_TS:
    profile = probe_mpeg_ts (ctx, codecs, st);
    break;
  default:
    break;
  }

 probe_mpeg2_end:
  if (codecs)
    free (codecs);
  
  return profile;
}

dlna_registered_profile_t dlna_profile_av_mpeg2 = {
  .id = DLNA_PROFILE_AV_MPEG2,
  .probe = probe_mpeg2,
  .next = NULL
};