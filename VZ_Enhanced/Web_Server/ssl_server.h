/*
	VZ Enhanced is a caller ID notifier that can forward and block phone calls.
	Copyright (C) 2013-2017 Eric Kutcher

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _SSL_WSA_H
#define _SSL_WSA_H

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define SECURITY_WIN32
#include <security.h>
#include <schannel.h>

#include "doublylinkedlist.h"
#include "lite_ws2_32.h"

#define SSL_STATE_SHUTDOWN		0
#define SSL_STATE_RUNNING		1

#define SP_PROT_TLS1_1_SERVER		0x00000100
#define SP_PROT_TLS1_1_CLIENT		0x00000200
#define SP_PROT_TLS1_1				( SP_PROT_TLS1_1_SERVER | SP_PROT_TLS1_1_CLIENT )
#define SP_PROT_TLS1_2_SERVER		0x00000400
#define SP_PROT_TLS1_2_CLIENT		0x00000800
#define SP_PROT_TLS1_2				( SP_PROT_TLS1_2_SERVER | SP_PROT_TLS1_2_CLIENT )

enum IO_OPERATION
{
	ClientIoAccept,
	ClientIoHandshakeReply,
	ClientIoHandshakeResponse,
	ClientIoWrite,
	ClientIoWriteRequestResource,
	ClientIoWriteWebSocketLists,
	ClientIoReadRequest,
	ClientIoReadMoreRequest,
	ClientIoReadWebSocketRequest,
	ClientIoReadMoreWebSocketRequest,
	ClientIoShutdown,
	ClientIoClose
};

struct ACCEPT_DATA
{
	SecBuffer       InBuffers[ 2 ];
	SecBuffer       OutBuffers[ 1 ];

	SECURITY_STATUS scRet;

	bool			fInitContext;
	bool            fDoRead;
};

struct RECV_DATA
{
	SecBuffer       Buffers[ 4 ];

	SECURITY_STATUS scRet;
};

struct SEND_BUFFER
{
	UCHAR			*pbDataBuffer;
	WSABUF			*wsabuf;
	OVERLAPPED		*overlapped;

	IO_OPERATION	*IOOperation;
	IO_OPERATION	*NextIOOperation;

	bool			in_use;
};

struct SEND_DATA
{
	SecPkgContext_StreamSizes	Sizes;

	DoublyLinkedList			*send_buffer_pool;

	bool						got_stream_sizes;
};

struct SHUTDOWN_DATA
{
    SecBuffer       OutBuffers[ 1 ];
};

struct SSL
{
	SEND_DATA		sd;
	ACCEPT_DATA		ad;
	RECV_DATA		rd;
	SHUTDOWN_DATA	sdd;

	CtxtHandle		hContext;

	BYTE			*pbRecDataBuf;
	BYTE			*pbIoBuffer;

	SOCKET			s;

	DWORD			cbRecDataBuf;
	DWORD			sbRecDataBuf;

	DWORD			cbIoBuffer;
	DWORD			sbIoBuffer;
};

int SSL_library_init( void );
int SSL_library_uninit( void );

SSL *SSL_new( DWORD protocol );
void SSL_free( SSL *ssl );

void ResetCredentials();

PCCERT_CONTEXT LoadPublicPrivateKeyPair( wchar_t *cer, wchar_t *key );
PCCERT_CONTEXT LoadPKCS12( wchar_t *p12_file, wchar_t *password );

extern unsigned char ssl_state;

extern PSecurityFunctionTableA g_pSSPI;

extern PCCERT_CONTEXT g_pCertContext;

#endif
