/*
 * libdlna: reference DLNA standards implementation.
 * Copyright (C) 2007-2008 Benjamin Zores <ben@geexbox.org>
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

#ifndef DLNA_H
#define DLNA_H

/**
 * @file dlna.h
 * external api header.
 */

#ifdef __cplusplus
extern "C" {
#if 0 /* avoid EMACS indent */
}
#endif /* 0 */
#endif /* __cplusplus */

#include <inttypes.h>
#include <sys/types.h>

#define DLNA_STRINGIFY(s)         DLNA_TOSTRING(s)
#define DLNA_TOSTRING(s) #s

#define LIBDLNA_VERSION_INT  ((0<<16)+(3<<8)+0)
#define LIBDLNA_VERSION      0.3.0
#define LIBDLNA_BUILD        LIBDLNA_VERSION_INT

#define LIBDLNA_IDENT        "DLNA " DLNA_STRINGIFY(LIBDLNA_VERSION)

/***************************************************************************/
/*                                                                         */
/* DLNA Library Common Utilities                                           */
/*  Mandatory: Used to configure the whole instance of the library.        */
/*                                                                         */
/***************************************************************************/

/* Status code for DLNA related functions */
typedef enum {
  DLNA_ST_OK,
  DLNA_ST_ERROR
} dlna_status_code_t;

/* Verbosity level: defines which kind of log can be displayed */
typedef enum {
  DLNA_MSG_NONE,          /* no error messages */
  DLNA_MSG_INFO,          /* working operations */
  DLNA_MSG_WARNING,       /* harmless failures */
  DLNA_MSG_ERROR,         /* may result in hazardous behavior */
  DLNA_MSG_CRITICAL,      /* prevents lib from working */
} dlna_verbosity_level_t;

/* DLNA Capability/Compatibility mode settings */
typedef enum {
  DLNA_CAPABILITY_DLNA,          /* comply with DLNA specifications */
  DLNA_CAPABILITY_UPNP_AV,       /* comply with UPnP A/V specifications */
  DLNA_CAPABILITY_UPNP_AV_XBOX,  /* UPnP A/V with XboX 360 hacks */
} dlna_capability_mode_t;

typedef enum {
  DLNA_PROTOCOL_INFO_TYPE_UNKNOWN,
  DLNA_PROTOCOL_INFO_TYPE_HTTP,
  DLNA_PROTOCOL_INFO_TYPE_RTP,
  DLNA_PROTOCOL_INFO_TYPE_ANY
} dlna_protocol_info_type_t;

/* DLNA.ORG_PS: play speed parameter (integer)
 *     0 invalid play speed
 *     1 normal play speed
 */
typedef enum {
  DLNA_ORG_PLAY_SPEED_INVALID = 0,
  DLNA_ORG_PLAY_SPEED_NORMAL = 1,
} dlna_org_play_speed_t;

/* DLNA.ORG_CI: conversion indicator parameter (integer)
 *     0 not transcoded
 *     1 transcoded
 */
typedef enum {
  DLNA_ORG_CONVERSION_NONE = 0,
  DLNA_ORG_CONVERSION_TRANSCODED = 1,
} dlna_org_conversion_t;

/* DLNA.ORG_OP: operations parameter (string)
 *     "00" (or "0") neither time seek range nor range supported
 *     "01" range supported
 *     "10" time seek range supported
 *     "11" both time seek range and range supported
 */
typedef enum {
  DLNA_ORG_OPERATION_NONE                  = 0x00,
  DLNA_ORG_OPERATION_RANGE                 = 0x01,
  DLNA_ORG_OPERATION_TIMESEEK              = 0x10,
} dlna_org_operation_t;

/* DLNA.ORG_FLAGS, padded with 24 trailing 0s
 *     80000000  31  senderPaced
 *     40000000  30  lsopTimeBasedSeekSupported
 *     20000000  29  lsopByteBasedSeekSupported
 *     10000000  28  playcontainerSupported
 *      8000000  27  s0IncreasingSupported
 *      4000000  26  sNIncreasingSupported
 *      2000000  25  rtspPauseSupported
 *      1000000  24  streamingTransferModeSupported
 *       800000  23  interactiveTransferModeSupported
 *       400000  22  backgroundTransferModeSupported
 *       200000  21  connectionStallingSupported
 *       100000  20  dlnaVersion15Supported
 *
 *     Example: (1 << 24) | (1 << 22) | (1 << 21) | (1 << 20)
 *       DLNA.ORG_FLAGS=01700000[000000000000000000000000] // [] show padding
 */
typedef enum {
  DLNA_ORG_FLAG_SENDER_PACED               = (1 << 31),
  DLNA_ORG_FLAG_TIME_BASED_SEEK            = (1 << 30),
  DLNA_ORG_FLAG_BYTE_BASED_SEEK            = (1 << 29),
  DLNA_ORG_FLAG_PLAY_CONTAINER             = (1 << 28),
  DLNA_ORG_FLAG_S0_INCREASE                = (1 << 27),
  DLNA_ORG_FLAG_SN_INCREASE                = (1 << 26),
  DLNA_ORG_FLAG_RTSP_PAUSE                 = (1 << 25),
  DLNA_ORG_FLAG_STREAMING_TRANSFER_MODE    = (1 << 24),
  DLNA_ORG_FLAG_INTERACTIVE_TRANSFERT_MODE = (1 << 23),
  DLNA_ORG_FLAG_BACKGROUND_TRANSFERT_MODE  = (1 << 22),
  DLNA_ORG_FLAG_CONNECTION_STALL           = (1 << 21),
  DLNA_ORG_FLAG_DLNA_V15                   = (1 << 20),
} dlna_org_flags_t;

/**
 * DLNA Library's controller.
 * This controls the whole library.
 */
typedef struct dlna_s dlna_t;

/**
 * Initialization of library.
 *
 * @warning This function must be called before any libdlna function.
 * @return DLNA library's controller.
 */
dlna_t *dlna_init (void);

/**
 * Uninitialization of library.
 *
 * @param[in] dlna The DLNA library's controller.
 */
void dlna_uninit (dlna_t *dlna);

/**
 * Set library's verbosity level.
 *
 * @param[in] dlna  The DLNA library's controller.
 * @param[in] level Level of verbosity
 */
void dlna_set_verbosity (dlna_t *dlna, dlna_verbosity_level_t level);

/**
 * Set library's capability/compatibility mode.
 *  N.B. Very few devices actualyl support DLNA specifications.
 *       If you have any problem having your device regonized,
 *       just downgrade to UPnP A/V compatibility mode which is just
 *       a non-restricted mode of DLNA specifications.
 *
 * @param[in] dlna  The DLNA library's controller.
 * @param[in] mode  Capability mode
 */
void dlna_set_capability_mode (dlna_t *dlna, dlna_capability_mode_t mode);

/**
 * Set library's mask of flags.
 *
 * @param[in] dlna  The DLNA library's controller.
 * @param[in] flags Mask of flags to be set
 */
void dlna_set_org_flags (dlna_t *dlna, dlna_org_flags_t flags);

/**
 * Set library's check level on files extension.
 *
 * @param[in] dlna  The DLNA library's controller.
 * @param[in] level Level of check (0 for no check, 1 to enable checks).
 */
void dlna_set_extension_check (dlna_t *dlna, int level);

/**
 * Set library's network interface to use.
 *
 * @param[in] dlna  The DLNA library's controller.
 * @param[in] itf   Name of the interface to be used.
 */
void dlna_set_interface (dlna_t *dlna, char *itf);

/**
 * Set library's network port to use (if available).
 *
 * @param[in] dlna  The DLNA library's controller.
 * @param[in] port  The port number.
 */
void dlna_set_port (dlna_t *dlna, int port);

/***************************************************************************/
/*                                                                         */
/* DLNA Media Profiles Handling                                            */
/*  Mandatory: Used to register one or many DLNA profiles                  */
/*             you want your device to support.                            */
/*                                                                         */
/***************************************************************************/

typedef enum {
  /* Image Class */
  DLNA_PROFILE_IMAGE_JPEG,
  DLNA_PROFILE_IMAGE_PNG,
  /* Audio Class */
  DLNA_PROFILE_AUDIO_AC3,
  DLNA_PROFILE_AUDIO_AMR,
  DLNA_PROFILE_AUDIO_ATRAC3,
  DLNA_PROFILE_AUDIO_LPCM,
  DLNA_PROFILE_AUDIO_MP3,
  DLNA_PROFILE_AUDIO_MPEG4,
  DLNA_PROFILE_AUDIO_WMA,
  /* AV Class */
  DLNA_PROFILE_AV_MPEG1,
  DLNA_PROFILE_AV_MPEG2,
  DLNA_PROFILE_AV_MPEG4_PART2,
  DLNA_PROFILE_AV_MPEG4_PART10, /* a.k.a. MPEG-4 AVC */
  DLNA_PROFILE_AV_WMV9
} dlna_media_profile_t;

/**
 * Register all known/supported DLNA profiles.
 *
 * @param[in] dlna  The DLNA library's controller.
 */
void dlna_register_all_media_profiles (dlna_t *dlna);

/**
 * Register one specific known/supported DLNA profiles.
 *
 * @param[in] dlna     The DLNA library's controller.
 * @param[in] profile  The profile ID to be registered.
 */
void dlna_register_media_profile (dlna_t *dlna, dlna_media_profile_t profile);

/***************************************************************************/
/*                                                                         */
/* DLNA Item Profile Handling                                              */
/*  Optional: Used to figure out which DLNA profile a file complies with.  */
/*                                                                         */
/***************************************************************************/

typedef enum {
  DLNA_CLASS_UNKNOWN,
  DLNA_CLASS_IMAGE,
  DLNA_CLASS_AUDIO,
  DLNA_CLASS_AV,
  DLNA_CLASS_COLLECTION
} dlna_media_class_t;

/**
 * DLNA profile.
 * This specifies the DLNA profile one file/stream is compatible with.
 */
typedef struct dlna_profile_s {
  /* Profile ID, part of DLNA.ORG_PN= string */
  const char *id;
  /* Profile MIME type */
  const char *mime;
  /* Profile Label */
  const char *label;
  /* Profile type: IMAGE / AUDIO / AV */
  dlna_media_class_t media_class;
} dlna_profile_t;

/**
 * Guess which DLNA profile one input file/stream is compatible with.
 *
 * @warning This function returns a pointer, do _NOT_ free it.
 * @param[in] dlna     The DLNA library's controller.
 * @param[in] filename The file to be checked for compliance.
 * @return A pointer on file's DLNA profile if compatible, NULL otherwise.
 */
dlna_profile_t *dlna_guess_media_profile (dlna_t *dlna, const char *filename);

/**
 * Provides UPnP A/V ContentDirectory Object Item associated to profile.
 *
 * @warning This function returns a pointer, do _NOT_ free it.
 * @param[in] profile The DLNA profile that was targeted.
 * @return A pointer on CDS Object Item string.
 */
char *dlna_profile_upnp_object_item (dlna_profile_t *profile);

/**
 * Output the protocol information string that must be send by a DMS to a DMP
 * for the file to be played/recognized.
 *
 * @param[in] type    Streaming method.
 * @param[in] speed   DLNA.ORG_PS parameter.
 * @param[in] ci      DLNA.ORG_CI parameter.
 * @param[in] op      DLNA.ORG_OP parameter.
 * @param[in] flags   DLNA.ORG_FLAGS parameter.
 * @param[in] profile The DLNA's file profile that has been guessed.
 * @return            The protocol information string.
 */
char * dlna_write_protocol_info (dlna_protocol_info_type_t type,
                                 dlna_org_play_speed_t speed,
                                 dlna_org_conversion_t ci,
                                 dlna_org_operation_t op,
                                 dlna_org_flags_t flags,
                                 dlna_profile_t *p);

/***************************************************************************/
/*                                                                         */
/* DLNA Item Handling                                                      */
/*  Optional: Used to create a DLNA Media Item instance from a given file. */
/*                                                                         */
/***************************************************************************/

/**
 * DLNA Media Object item metadata
 */
typedef struct dlna_metadata_s {
  char     *title;                /* <dc:title> */
  char     *author;               /* <dc:artist> */
  char     *comment;              /* <upnp:longDescription> */
  char     *album;                /* <upnp:album> */
  uint32_t track;                 /* <upnp:originalTrackNumber> */
  char     *genre;                /* <upnp:genre> */
} dlna_metadata_t;

/**
 * DLNA Media Object item properties
 */
typedef struct dlna_properties_s {
  int64_t  size;                  /* res@size */
  char     duration[64];          /* res@duration */
  uint32_t bitrate;               /* res@bitrate */
  uint32_t sample_frequency;      /* res@sampleFrequency */
  uint32_t bps;                   /* res@bitsPerSample */
  uint32_t channels;              /* res@nrAudioChannels */
  char     resolution[64];        /* res@resolution */
} dlna_properties_t;

/**
 * DLNA Media Object item
 */
typedef struct dlna_item_s {
  char *filename;
  dlna_media_class_t media_class;
  dlna_properties_t *properties;
  dlna_metadata_t *metadata;
  dlna_profile_t *profile;
} dlna_item_t;

/**
 * Create a new DLNA media object item.
 *
 * @param[in] dlna     The DLNA library's controller.
 * @param[in] filename The input file to be added.
 * @return A new DLNA object item if compatible, NULL otherwise.
 */
dlna_item_t *dlna_item_new (dlna_t *dlna, const char *filename);

/**
 * Free an existing DLNA media object item.
 *
 * @param[in] item     The DLNA object item to be freed.
 */
void dlna_item_free (dlna_item_t *item);

/***************************************************************************/
/*                                                                         */
/* DLNA UPnP Digital Media Server (DMS) Management                         */
/*  Mandatory: Configure the device to act as a Media Server.              */
/*                                                                         */
/***************************************************************************/

typedef enum {
  DLNA_DMS_STORAGE_MEMORY,
  DLNA_DMS_STORAGE_SQL_DB,
} dlna_dms_storage_type_t;

/**
 * Initialize a DLNA Digital Media Server compliant device.
 *
 * @param[in] dlna  The DLNA library's controller.
 *
  * @return   DLNA_ST_OK in case of success, DLNA_ST_ERROR otherwise.
 */
int dlna_dms_init (dlna_t *dlna);

/**
 * Uninitialize a DLNA Digital Media Server compliant device.
 *
 * @param[in] dlna  The DLNA library's controller.
 *
  * @return   DLNA_ST_OK in case of success, DLNA_ST_ERROR otherwise.
 */
int dlna_dms_uninit (dlna_t *dlna);

/**
 * Create a valid UPnP device description for Digital Media Server (DMS).
 *
 * @param[in] dlna  The DLNA library's controller.
 * 
 * @return                       The DMS device description string.
 */
char *dlna_dms_description_get (dlna_t *dlna);

/**
 * Defines DMS storage type for VFS Metadata.
 *
 * @param[in] dlna  The DLNA library's controller.
 * @param[in] type  The VFS storage type.
 * @param[in] data  Optional cookie depending on storage type:
 *                   - May be NULL for memory storage.
 *                   - Path to databased file for SQL_DB storage.
 * 
 */
void dlna_dms_set_vfs_storage_type (dlna_t *dlna,
                                    dlna_dms_storage_type_t type, char *data);

/***************************************************************************/
/*                                                                         */
/* DLNA UPnP Digital Media Player (DMP) Management                         */
/*  Mandatory: Configure the device to act as a Media Player.              */
/*                                                                         */
/***************************************************************************/


/***************************************************************************/
/*                                                                         */
/* DLNA Services Management                                                */
/*  Optional: Used to register common services or add new ones.            */
/*                                                                         */
/***************************************************************************/

typedef enum {
  DLNA_SERVICE_CONNECTION_MANAGER,
  DLNA_SERVICE_CONTENT_DIRECTORY,
  DLNA_SERVICE_AV_TRANSPORT,
  DLNA_SERVICE_MS_REGISTAR,
} dlna_service_type_t;

/**
 * Register a known DLNA/UPnP Service to be used by the device.
 *   This is automatically done for all mandatory services at device init.
 *
 * @param[in] dlna  The DLNA library's controller.
 * @param[in] srv   The service type to be registered.
 */
void dlna_service_register (dlna_t *dlna, dlna_service_type_t srv);

/***************************************************************************/
/*                                                                         */
/* DLNA UPnP Device Management                                             */
/*  Optional: Used to overload default device parameters.                  */
/*                                                                         */
/***************************************************************************/

/**
 * Set device UPnP friendly name.
 *
 * @param[in] dlna  The DLNA library's controller.
 * @param[in] str   Value to be set.
 */
void dlna_device_set_friendly_name (dlna_t *dlna, char *str);

/**
 * Set device UPnP manufacturer name.
 *
 * @param[in] dlna  The DLNA library's controller.
 * @param[in] str   Value to be set.
 */
void dlna_device_set_manufacturer (dlna_t *dlna, char *str);

/**
 * Set device UPnP manufacturer URL.
 *
 * @param[in] dlna  The DLNA library's controller.
 * @param[in] str   Value to be set.
 */
void dlna_device_set_manufacturer_url (dlna_t *dlna, char *str);

/**
 * Set device UPnP model description.
 *
 * @param[in] dlna  The DLNA library's controller.
 * @param[in] str   Value to be set.
 */
void dlna_device_set_model_description (dlna_t *dlna, char *str);

/**
 * Set device UPnP model name.
 *
 * @param[in] dlna  The DLNA library's controller.
 * @param[in] str   Value to be set.
 */
void dlna_device_set_model_name (dlna_t *dlna, char *str);

/**
 * Set device UPnP model number.
 *
 * @param[in] dlna  The DLNA library's controller.
 * @param[in] str   Value to be set.
 */
void dlna_device_set_model_number (dlna_t *dlna, char *str);

/**
 * Set device UPnP model URL.
 *
 * @param[in] dlna  The DLNA library's controller.
 * @param[in] str   Value to be set.
 */
void dlna_device_set_model_url (dlna_t *dlna, char *str);

/**
 * Set device UPnP serial number.
 *
 * @param[in] dlna  The DLNA library's controller.
 * @param[in] str   Value to be set.
 */
void dlna_device_set_serial_number (dlna_t *dlna, char *str);

/**
 * Set device UPnP UUID.
 *
 * @param[in] dlna  The DLNA library's controller.
 * @param[in] str   Value to be set ("uuid:" is automatically preprended).
 */
void dlna_device_set_uuid (dlna_t *dlna, char *str);

/**
 * Set device UPnP presentation URL.
 *
 * @param[in] dlna  The DLNA library's controller.
 * @param[in] str   Value to be set.
 */
void dlna_device_set_presentation_url (dlna_t *dlna, char *str);

/***************************************************************************/
/*                                                                         */
/* DLNA UPnP Virtual File System (VFS) Management                          */
/*  Optional: Routines to add/remove element from VFS.                     */
/*                                                                         */
/***************************************************************************/

/**
 * Add a new container to the VFS layer.
 *
 * @param[in] dlna         The DLNA library's controller.
 * @param[in] name         Displayed name of the container.
 * @param[in] object_id    Expected UPnP object ID.
 * @param[in] container_id UPnP object ID of its parent.
 * @return The attrbiuted UPnP object ID if successfull, 0 otherwise.
 */
uint32_t dlna_vfs_add_container (dlna_t *dlna, char *name,
                                 uint32_t object_id, uint32_t container_id);

/**
 * Add a new resource to the VFS layer.
 *
 * @param[in] dlna         The DLNA library's controller.
 * @param[in] name         Displayed name of the resource.
 * @param[in] fullname     Full path to the specified resource.
 * @param[in] size         Resource file size (in bytes).
 * @param[in] container_id UPnP object ID of its parent.
 * @return The attrbiuted UPnP object ID if successfull, 0 otherwise.
 */
uint32_t dlna_vfs_add_resource (dlna_t *dlna, char *name,
                                char *fullpath, off_t size,
                                uint32_t container_id);

/**
 * Remove an existing item (and all its children) from VFS layer by ID.
 *
 * @param[in] dlna         The DLNA library's controller.
 * @param[in] id           Unique ID of the item to be removed.
 */
void dlna_vfs_remove_item_by_id (dlna_t *dlna, uint32_t id);

/**
 * Remove an existing item (and all its children) from VFS layer by name.
 *
 * @param[in] dlna         The DLNA library's controller.
 * @param[in] name         Name of the item to be removed.
 */
void dlna_vfs_remove_item_by_name (dlna_t *dlna, char *name);

/***************************************************************************/
/*                                                                         */
/* DLNA WebServer Callbacks & Handlers                                     */
/*  Optional: Used to overload the internal HTTP server behavior.          */
/*                                                                         */
/***************************************************************************/

/**
 * DLNA Internal WebServer File Handler
 */
typedef struct dlna_http_file_handler_s {
  int external;                   /* determines whether the file has to be
                                     handled internally by libdlna or by
                                     external application */
  void *priv;                     /* private file handler */
} dlna_http_file_handler_t;

/**
 * DLNA Internal WebServer File Information
 */
typedef struct dlna_http_file_info_s {
  off_t file_length;
  char *content_type;
} dlna_http_file_info_t;

/**
 * DLNA Internal WebServer Operation Callbacks
 *  Return 0 for success, 1 otherwise.
 */
typedef struct dlna_http_callback_s {
  int (*get_info) (const char *filename, dlna_http_file_info_t *info);
  dlna_http_file_handler_t * (*open) (const char *filename);
  int (*read) (void *hdl, char *buf, size_t len);
  int (*write) (void *hdl, char *buf, size_t len);
  int (*seek) (void *hdl, off_t offset, int origin);
  int (*close) (void *hdl);
} dlna_http_callback_t;

/**
 * Set library's WebServer Callback routines.
 *   This is used by application to overload default's HTTP routines.
 *
 * @param[in] dlna  The DLNA library's controller.
 * @param[in] cb    Structure with HTTP callbacks.
 */
void dlna_set_http_callback (dlna_t *dlna, dlna_http_callback_t *cb);

#ifdef __cplusplus
#if 0 /* avoid EMACS indent */
{
#endif /* 0 */
}
#endif /* __cplusplus */

#endif /* DLNA_H */
