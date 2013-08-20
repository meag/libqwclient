/*
GNU General Public License version 3 notice

Copyright (C) 2012 Mihawk <luiz@netdome.biz>. All rights reserved.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see < http://www.gnu.org/licenses/ >.
*/

#ifndef QUAKEDEF_H
#define QUAKEDEF_H

// for chunked downloads
#define MAXBLOCKS		1024	// Must be power of 2
#define DLBLOCKSIZE 1024
#define DL_QWCHUNKS 2

/* Needed definitions taken from EZQuake Source */

typedef float vec3_t[3];

typedef struct
{
	int			number;			// edict index
	int			flags;			// nolerp, etc
	vec3_t	origin;
	vec3_t	angles;
	int			modelindex;
	int			frame;
	int			colormap;
	int			skinnum;
	int			effects;
} entityState_t;

#define MAX_PACKET_ENTITIES	64

typedef struct
{
	int						numentities;
	entityState_t	entities[MAX_PACKET_ENTITIES];
} packetEntities_t;

#define	PROTOCOL_VERSION	28
#define PROTOCOL_VERSION_FTE	(('F'<<0) + ('T'<<8) + ('E'<<16) + ('X' << 24)) //fte extensions.
#define PROTOCOL_VERSION_FTE2	(('F'<<0) + ('T'<<8) + ('E'<<16) + ('2' << 24))	//fte extensions.

#ifdef PROTOCOL_VERSION_FTE
	#define	FTE_PEXT_TRANS							0x00000008	// .alpha support
	#define FTE_PEXT_HLBSP							0x00000200	//stops fte servers from complaining
	#define FTE_PEXT_MODELDBL						0x00001000
	#define FTE_PEXT_ENTITYDBL					0x00002000	//max of 1024 ents instead of 512
	#define FTE_PEXT_ENTITYDBL2					0x00004000	//max of 1024 ents instead of 512
	#define FTE_PEXT_FLOATCOORDS				0x00008000	//supports floating point origins.
	#define FTE_PEXT_SPAWNSTATIC2				0x00400000	//Sends an entity delta instead of a baseline.
	#define FTE_PEXT_256PACKETENTITIES	0x01000000	//Client can recieve 256 packet entities.
	#define FTE_PEXT_CHUNKEDDOWNLOADS		0x20000000	//alternate file download method. Hopefully it'll give quadroupled download speed, especially on higher pings.
#endif // PROTOCOL_VERSION_FTE

#ifdef PROTOCOL_VERSION_FTE2
	#define FTE_PEXT2_VOICECHAT			0x00000002
#endif // PROTOCOL_VERSION_FTE2

#ifdef PROTOCOL_VERSION_FTE
	#define U_FTE_EVENMORE	(1<<7)		//extension info follows
	//EVENMORE flags
	#ifdef FTE_PEXT_SCALE
		#define U_FTE_SCALE		(1<<0)		//scaler of alias models
	#endif
	#ifdef FTE_PEXT_TRANS
		#define U_FTE_TRANS		(1<<1)		//transparency value
	#endif
	#ifdef FTE_PEXT_TRANS
		#define	PF_TRANS_Z			(1<<17)
	#endif
	#ifdef FTE_PEXT_FATNESS
		#define U_FTE_FATNESS	(1<<2)		//byte describing how fat an alias model should be.
									//moves verticies along normals
									// Useful for vacuum chambers...
	#endif
	#ifdef FTE_PEXT_MODELDBL
		#define U_FTE_MODELDBL	(1<<3)		//extra bit for modelindexes
	#endif
	#define U_FTE_UNUSED1	(1<<4)
	#ifdef FTE_PEXT_ENTITYDBL
		#define U_FTE_ENTITYDBL	(1<<5)		//use an extra byte for origin parts, cos one of them is off
	#endif
	#ifdef FTE_PEXT_ENTITYDBL2
		#define U_FTE_ENTITYDBL2 (1<<6)		//use an extra byte for origin parts, cos one of them is off
	#endif
	#define U_FTE_YETMORE	(1<<7)		//even more extension info stuff.
	#define U_FTE_DRAWFLAGS	(1<<8)		//use an extra qbyte for origin parts, cos one of them is off
	#define U_FTE_ABSLIGHT	(1<<9)		//Force a lightlevel
	#define U_FTE_COLOURMOD	(1<<10)		//rgb
	#define U_FTE_DPFLAGS (1<<11)
	#define U_FTE_TAGINFO (1<<12)
	#define U_FTE_LIGHT (1<<13)
	#define	U_FTE_EFFECTS16	(1<<14)
	#define U_FTE_FARMORE (1<<15)
#endif

#define QW_CHECK_HASH 0x5157

//=========================================

#define	PORT_CLIENT	27001
#define	PORT_MASTER	27000
#define	PORT_SERVER	27500

// out of band message id bytes
// M = master, S = server, C = client, A = any
// the second character will always be \n if the message isn't a single
// byte long (?? not true anymore?)

#define	S2C_CHALLENGE				'c'
#define	S2C_CONNECTION			'j'
#define	A2A_PING						'k'	// respond with an A2A_ACK
#define	A2A_ACK							'l'	// general acknowledgement without info
#define	A2A_NACK						'm'	// [+ comment] general failure
#define A2A_ECHO						'e' // for echoing
#define	A2C_PRINT						'n'	// print a message on client
#define	S2M_HEARTBEAT				'a'	// + serverinfo + userlist + fraglist
#define	A2C_CLIENT_COMMAND	'B'	// + command line
#define	S2M_SHUTDOWN				'C'

//
// server to client
//
#define	svc_bad									0
#define	svc_nop									1
#define	svc_disconnect					2
#define	svc_updatestat					3	// [byte] [byte]
#define	nq_svc_version					4	// [long] server version
#define	svc_setview							5	// [short] entity number
#define	svc_sound								6	// <see code>
#define	nq_svc_time							7	// [float] server time
#define	svc_print								8	// [byte] id [string] null terminated string
#define	svc_stufftext						9	// [string] stuffed into client's console buffer
#define	svc_setangle						10	// [angle3] set the view angle to this absolute value
#define	svc_serverdata					11	// [long] protocol ...
#define	svc_lightstyle					12	// [byte] [string]
#define	nq_svc_updatename				13	// [byte] [string]
#define	svc_updatefrags					14	// [byte] [short]
#define	nq_svc_clientdata				15	// <shortbits + data>
#define	svc_stopsound						16	// <see code>
#define	nq_svc_updatecolors			17	// [byte] [byte] [byte]
#define	nq_svc_particle					18	// [vec3] <variable>
#define	svc_damage							19
#define	svc_spawnstatic					20
#define	svc_fte_spawnstatic2		21		// @!@!@!
#define	svc_spawnbaseline				22
#define	svc_temp_entity					23	// variable
#define	svc_setpause						24	// [byte] on / off
#define	nq_svc_signonnum				25	// [byte]  used for the signon sequence
#define	svc_centerprint					26	// [string] to put in center of the screen
#define	svc_killedmonster				27
#define	svc_foundsecret					28
#define	svc_spawnstaticsound		29    // [coord3] [byte] samp [byte] vol [byte] aten
#define	svc_intermission				30		// [vec3_t] origin [vec3_t] angle
#define	svc_finale							31		// [string] text
#define	svc_cdtrack							32		// [byte] track
#define svc_sellscreen					33
#define	svc_smallkick						34		// set client punchangle to 2
#define	svc_bigkick							35		// set client punchangle to 4
#define	svc_updateping					36		// [byte] [short]
#define	svc_updateentertime			37		// [byte] [float]
#define	svc_updatestatlong			38		// [byte] [long]
#define	svc_muzzleflash					39		// [short] entity
#define	svc_updateuserinfo			40		// [byte] slot [long] uid
#define	svc_download						41		// [short] size [size bytes]
#define	svc_playerinfo					42		// variable
#define	svc_nails								43		// [byte] num [48 bits] xyzpy 12 12 12 4 8
#define	svc_chokecount					44		// [byte] packets choked
#define	svc_modellist						45		// [strings]
#define	svc_soundlist						46		// [strings]
#define	svc_packetentities			47		// [...]
#define	svc_deltapacketentities	48		// [...]
#define svc_maxspeed						49		// maxspeed change, for prediction
#define svc_entgravity					50		// gravity change, for prediction
#define svc_setinfo							51		// setinfo on a client
#define svc_serverinfo					52		// serverinfo
#define svc_updatepl						53		// [byte] [byte]
#define svc_nails2							54		// [byte] num [52 bits] nxyzpy 8 12 12 12 4 8
#define	svc_fte_modellistshort	60		// [strings]
#define svc_fte_spawnbaseline2	66
#define svc_qizmovoice					83
#define svc_fte_voicechat				84

//
// client to server
//
#define	clc_bad				0
#define	clc_nop				1
#define	clc_move			3		// [[usercmd_t]
#define	clc_stringcmd	4		// [string] message
#define	clc_delta			5		// [byte] sequence number, requests delta compression of message
#define clc_tmove			6		// teleport request, spectator only
#define clc_upload		7		// teleport request, spectator only

typedef struct userCmd_s
{
	unsigned char	msec;
	float			angles[3];
	short			forwardmove, sidemove, upmove;
	unsigned char	buttons;
	unsigned char	impulse;
} userCmd_t;

#define	PF_MSEC					(1<<0)
#define	PF_COMMAND			(1<<1)
#define	PF_VELOCITY1		(1<<2)
#define	PF_VELOCITY2		(1<<3)
#define	PF_VELOCITY3		(1<<4)
#define	PF_MODEL				(1<<5)
#define	PF_SKINNUM			(1<<6)
#define	PF_EFFECTS			(1<<7)
#define	PF_WEAPONFRAME	(1<<8)		// only sent for view player
#define	PF_DEAD					(1<<9)		// don't block movement any more
#define	PF_GIB					(1<<10)		// offset the view height differently
#define	PF_NOGRAV				(1<<11)		// don't apply gravity for prediction
#define	CM_ANGLE1				(1<<0)
#define	CM_ANGLE3				(1<<1)
#define	CM_FORWARD			(1<<2)
#define	CM_SIDE					(1<<3)
#define	CM_UP						(1<<4)
#define	CM_BUTTONS			(1<<5)
#define	CM_IMPULSE			(1<<6)
#define	CM_ANGLE2				(1<<7)
#define	U_ORIGIN1				(1<<9)
#define	U_ORIGIN2				(1<<10)
#define	U_ORIGIN3				(1<<11)
#define	U_ANGLE2				(1<<12)
#define	U_FRAME					(1<<13)
#define	U_REMOVE				(1<<14)		// REMOVE this entity, don't add it
#define	U_MOREBITS			(1<<15)
#define	U_ANGLE1				(1<<0)
#define	U_ANGLE3				(1<<1)
#define	U_MODEL					(1<<2)
#define	U_COLORMAP			(1<<3)
#define	U_SKIN					(1<<4)
#define	U_EFFECTS				(1<<5)
#define	U_SOLID					(1<<6)		// the entity should be solid for prediction
#define	SND_VOLUME			(1<<15)		// a byte
#define	SND_ATTENUATION	(1<<14)		// a byte
#define	TE_SPIKE					0
#define	TE_SUPERSPIKE			1
#define	TE_GUNSHOT				2
#define	TE_EXPLOSION			3
#define	TE_TAREXPLOSION		4
#define	TE_LIGHTNING1			5
#define	TE_LIGHTNING2			6
#define	TE_WIZSPIKE				7
#define	TE_KNIGHTSPIKE		8
#define	TE_LIGHTNING3			9
#define	TE_LAVASPLASH			10
#define	TE_TELEPORT				11
#define	TE_BLOOD					12
#define	TE_LIGHTNINGBLOOD	13
#define BSPVERSION				29
#define MAX_MAP_HULLS			4

typedef struct
{
	int		fileofs, filelen;
} lump_t;

#define	LUMP_ENTITIES			0
#define	LUMP_PLANES				1
#define	LUMP_TEXTURES			2
#define	LUMP_VERTEXES			3
#define	LUMP_VISIBILITY		4
#define	LUMP_NODES				5
#define	LUMP_TEXINFO			6
#define	LUMP_FACES				7
#define	LUMP_LIGHTING			8
#define	LUMP_CLIPNODES		9
#define	LUMP_LEAFS				10
#define	LUMP_MARKSURFACES 11
#define	LUMP_EDGES				12
#define	LUMP_SURFEDGES		13
#define	LUMP_MODELS				14
#define	HEADER_LUMPS			15

typedef struct
{
	float		mins[3], maxs[3];
	float		origin[3];
	int			headnode[MAX_MAP_HULLS];
	int			visleafs;		// not including the solid leaf 0
	int			firstface, numfaces;
} dmodel_t;

typedef struct
{
	int				version;
	lump_t		lumps[HEADER_LUMPS];
} dheader_t;

#define MAX_MSGLEN 2048

#endif // QUAKEDEF_H
