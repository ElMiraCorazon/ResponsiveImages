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

#include "globals.h"

#include "lite_ntdll.h"
#include "lite_shell32.h"
#include "lite_gdi32.h"
#include "lite_comdlg32.h"
#include "lite_ole32.h"
#include "lite_user32.h"
#include "lite_normaliz.h"

#include "file_operations.h"
#include "utilities.h"
#include "menus.h"

#include "string_tables.h"

#define BTN_WEB_SERVER					1000
#define EDIT_HOST						1001
#define EDIT_IP_ADDRESS					1002
#define EDIT_PORT						1003

#define BTN_TYPE_HOST					1004
#define BTN_TYPE_IP_ADDRESS				1005

#define EDIT_CERTIFICATE_PKCS			1006
#define BTN_CERTIFICATE_PKCS			1007
#define EDIT_CERTIFICATE_PKCS_PASSWORD	1008

#define CB_SSL_VERSION					1009

#define BTN_AUTO_START					1010

#define BTN_TYPE_PKCS					1011
#define BTN_TYPE_PAIR					1012

#define EDIT_CERTIFICATE_CER			1013
#define BTN_CERTIFICATE_CER				1014
#define EDIT_CERTIFICATE_KEY			1015
#define BTN_CERTIFICATE_KEY				1016

#define BTN_ENABLE_SSL					1017

#define EDIT_SERVER_DIRECTORY			1018
#define BTN_SERVER_DIRECTORY			1019

#define BTN_USE_AUTHENTICATION			1020
#define EDIT_AUTHENTICATION_USERNAME	1021
#define EDIT_AUTHENTICATION_PASSWORD	1022

#define EDIT_THREAD_COUNT				1023
#define EDIT_CACHE_SIZE					1024

#define BTN_VERIFY_ORIGIN				1025
#define BTN_KEEP_ALIVE					1026

HWND g_hWnd_chk_web_server = NULL;
HWND g_hWnd_ip_address = NULL;
HWND g_hWnd_hostname = NULL;
HWND g_hWnd_port = NULL;

HWND g_hWnd_static_port = NULL;
HWND g_hWnd_static_colon = NULL;

HWND g_hWnd_chk_enable_ssl = NULL;

HWND g_hWnd_static_certificate_pkcs_location = NULL;
HWND g_hWnd_certificate_pkcs_location = NULL;
HWND g_hWnd_btn_certificate_pkcs_location = NULL;

HWND g_hWnd_static_certificate_pkcs_password = NULL;
HWND g_hWnd_certificate_pkcs_password = NULL;

HWND g_hWnd_static_thread_count = NULL;
HWND g_hWnd_thread_count = NULL;

HWND g_hWnd_static_resource_cache_size = NULL;
HWND g_hWnd_resource_cache_size = NULL;

HWND g_hWnd_chk_verify_origin = NULL;
HWND g_hWnd_chk_keep_alive = NULL;

HWND g_hWnd_chk_type_hostname = NULL;
HWND g_hWnd_chk_type_ip_address = NULL;

HWND g_hWnd_static_ssl_version = NULL;
HWND g_hWnd_ssl_version = NULL;

HWND g_hWnd_chk_auto_start = NULL;


HWND g_hWnd_chk_type_pkcs = NULL;
HWND g_hWnd_chk_type_pair = NULL;

HWND g_hWnd_static_certificate_cer_location = NULL;
HWND g_hWnd_certificate_cer_location = NULL;
HWND g_hWnd_btn_certificate_cer_location = NULL;

HWND g_hWnd_static_certificate_key_location = NULL;
HWND g_hWnd_certificate_key_location = NULL;
HWND g_hWnd_btn_certificate_key_location = NULL;

HWND g_hWnd_static_web_server_directory = NULL;
HWND g_hWnd_web_server_directory = NULL;
HWND g_hWnd_btn_web_server_directory = NULL;


HWND g_hWnd_chk_use_authentication = NULL;
HWND g_hWnd_static_authentication_username = NULL;
HWND g_hWnd_authentication_username = NULL;
HWND g_hWnd_static_authentication_password = NULL;
HWND g_hWnd_authentication_password = NULL;

HWND g_hWnd_static_hoz1 = NULL;
HWND g_hWnd_static_hoz2 = NULL;

HFONT hFont = NULL;
HFONT hFont_copy = NULL;

// Free these when done.
wchar_t *certificate_pkcs_file_name = NULL;
wchar_t *certificate_cer_file_name = NULL;
wchar_t *certificate_key_file_name = NULL;
wchar_t *t_document_root_directory = NULL;

int web_server_tab_height = 0;
int web_server_tab_scroll_pos = 0;

extern "C" __declspec ( dllexport )
void SaveWebServerSettings()
{
	bool enable_changed = false;
	bool enable_web_server = ( _SendMessageW( g_hWnd_chk_web_server, BM_GETCHECK, 0, 0 ) == BST_CHECKED ? true : false );
	
	enable_changed = ( cfg_enable_web_server != enable_web_server );
	
	cfg_enable_web_server = enable_web_server;

	cfg_address_type = ( _SendMessageW( g_hWnd_chk_type_ip_address, BM_GETCHECK, 0, 0 ) == BST_CHECKED ? 1 : 0 );

	unsigned int hostname_length = _SendMessageW( g_hWnd_hostname, WM_GETTEXTLENGTH, 0, 0 ) + 1;	// Include the NULL terminator.
	if ( cfg_hostname != NULL )
	{
		GlobalFree( cfg_hostname );
	}
	cfg_hostname = ( wchar_t * )GlobalAlloc( GMEM_FIXED, sizeof( wchar_t ) * hostname_length );
	_SendMessageW( g_hWnd_hostname, WM_GETTEXT, hostname_length, ( LPARAM )cfg_hostname );

	if ( normaliz_state == NORMALIZ_STATE_RUNNING )
	{
		if ( cfg_address_type == 0 )
		{
			if ( g_punycode_hostname != NULL )
			{
				GlobalFree( g_punycode_hostname );
				g_punycode_hostname = NULL;
			}

			int punycode_length = _IdnToAscii( 0, cfg_hostname, hostname_length, NULL, 0 );

			if ( punycode_length > ( int )hostname_length )
			{
				g_punycode_hostname = ( wchar_t * )GlobalAlloc( GMEM_FIXED, sizeof( wchar_t ) * punycode_length );
				_IdnToAscii( 0, cfg_hostname, hostname_length, g_punycode_hostname, punycode_length );
			}
		}
	}

	_SendMessageW( g_hWnd_ip_address, IPM_GETADDRESS, 0, ( LPARAM )&cfg_ip_address );

	cfg_auto_start = ( _SendMessageW( g_hWnd_chk_auto_start, BM_GETCHECK, 0, 0 ) == BST_CHECKED ? true : false );

	cfg_verify_origin = ( _SendMessageW( g_hWnd_chk_verify_origin, BM_GETCHECK, 0, 0 ) == BST_CHECKED ? true : false );

	cfg_allow_keep_alive_requests = ( _SendMessageW( g_hWnd_chk_keep_alive, BM_GETCHECK, 0, 0 ) == BST_CHECKED ? true : false );

	cfg_enable_ssl = ( _SendMessageW( g_hWnd_chk_enable_ssl, BM_GETCHECK, 0, 0 ) == BST_CHECKED ? true : false );

	cfg_certificate_type = ( _SendMessageW( g_hWnd_chk_type_pair, BM_GETCHECK, 0, 0 ) == BST_CHECKED ? 1 : 0 );

	char value[ 21 ];
	_SendMessageA( g_hWnd_port, WM_GETTEXT, 6, ( LPARAM )value );
	cfg_port = ( unsigned short )_strtoul( value, NULL, 10 );

	cfg_use_authentication = ( _SendMessageW( g_hWnd_chk_use_authentication, BM_GETCHECK, 0, 0 ) == BST_CHECKED ? true : false );

	unsigned int authentication_username_length = _SendMessageW( g_hWnd_authentication_username, WM_GETTEXTLENGTH, 0, 0 ) + 1;	// Include the NULL terminator.
	if ( cfg_authentication_username != NULL )
	{
		GlobalFree( cfg_authentication_username );
	}
	cfg_authentication_username = ( wchar_t * )GlobalAlloc( GMEM_FIXED, sizeof( wchar_t ) * authentication_username_length );
	_SendMessageW( g_hWnd_authentication_username, WM_GETTEXT, authentication_username_length, ( LPARAM )cfg_authentication_username );


	unsigned int authentication_password_length = _SendMessageW( g_hWnd_authentication_password, WM_GETTEXTLENGTH, 0, 0 ) + 1;	// Include the NULL terminator.
	if ( cfg_authentication_password != NULL )
	{
		GlobalFree( cfg_authentication_password );
	}
	cfg_authentication_password = ( wchar_t * )GlobalAlloc( GMEM_FIXED, sizeof( wchar_t ) * authentication_password_length );
	_SendMessageW( g_hWnd_authentication_password, WM_GETTEXT, authentication_password_length, ( LPARAM )cfg_authentication_password );

	_SendMessageA( g_hWnd_thread_count, WM_GETTEXT, 11, ( LPARAM )value );
	cfg_thread_count = _strtoul( value, NULL, 10 );

	_SendMessageA( g_hWnd_resource_cache_size, WM_GETTEXT, 21, ( LPARAM )value );
	cfg_resource_cache_size = strtoull( value );

	if ( cfg_certificate_pkcs_file_name != NULL )
	{
		GlobalFree( cfg_certificate_pkcs_file_name );
	}

	/*int certificate_pkcs_file_name_length = lstrlenW( certificate_pkcs_file_name );
	cfg_certificate_pkcs_file_name = ( wchar_t * )GlobalAlloc( GMEM_FIXED, sizeof( wchar_t ) * ( certificate_pkcs_file_name_length + 1 ) );
	_wmemcpy_s( cfg_certificate_pkcs_file_name, certificate_pkcs_file_name_length + 1, certificate_pkcs_file_name, certificate_pkcs_file_name_length );
	*( cfg_certificate_pkcs_file_name + certificate_pkcs_file_name_length ) = 0;	// Sanity.*/

	cfg_certificate_pkcs_file_name = GlobalStrDupW( certificate_pkcs_file_name );

	unsigned int certificate_pkcs_password_length = _SendMessageW( g_hWnd_certificate_pkcs_password, WM_GETTEXTLENGTH, 0, 0 ) + 1;	// Include the NULL terminator.
	if ( cfg_certificate_pkcs_password != NULL )
	{
		GlobalFree( cfg_certificate_pkcs_password );
	}
	cfg_certificate_pkcs_password = ( wchar_t * )GlobalAlloc( GMEM_FIXED, sizeof( wchar_t ) * certificate_pkcs_password_length );
	_SendMessageW( g_hWnd_certificate_pkcs_password, WM_GETTEXT, certificate_pkcs_password_length, ( LPARAM )cfg_certificate_pkcs_password );

	if ( cfg_certificate_cer_file_name != NULL )
	{
		GlobalFree( cfg_certificate_cer_file_name );
	}

	/*int certificate_cer_file_name_length = lstrlenW( certificate_cer_file_name );
	cfg_certificate_cer_file_name = ( wchar_t * )GlobalAlloc( GMEM_FIXED, sizeof( wchar_t ) * ( certificate_cer_file_name_length + 1 ) );
	_wmemcpy_s( cfg_certificate_cer_file_name, certificate_cer_file_name_length + 1, certificate_cer_file_name, certificate_cer_file_name_length );
	*( cfg_certificate_cer_file_name + certificate_cer_file_name_length ) = 0;	// Sanity.*/

	cfg_certificate_cer_file_name = GlobalStrDupW( certificate_cer_file_name );

	if ( cfg_certificate_key_file_name != NULL )
	{
		GlobalFree( cfg_certificate_key_file_name );
	}

	/*int certificate_key_file_name_length = lstrlenW( certificate_key_file_name );
	cfg_certificate_key_file_name = ( wchar_t * )GlobalAlloc( GMEM_FIXED, sizeof( wchar_t ) * ( certificate_key_file_name_length + 1 ) );
	_wmemcpy_s( cfg_certificate_key_file_name, certificate_key_file_name_length + 1, certificate_key_file_name, certificate_key_file_name_length );
	*( cfg_certificate_key_file_name + certificate_key_file_name_length ) = 0;	// Sanity.*/

	cfg_certificate_key_file_name = GlobalStrDupW( certificate_key_file_name );

	if ( cfg_document_root_directory != NULL )
	{
		GlobalFree( cfg_document_root_directory );
	}

	g_document_root_directory_length = lstrlenW( t_document_root_directory );
	cfg_document_root_directory = ( wchar_t * )GlobalAlloc( GMEM_FIXED, sizeof( wchar_t ) * ( g_document_root_directory_length + 1 ) );
	_wmemcpy_s( cfg_document_root_directory, g_document_root_directory_length + 1, t_document_root_directory, g_document_root_directory_length );
	*( cfg_document_root_directory + g_document_root_directory_length ) = 0;	// Sanity.

	cfg_ssl_version = ( unsigned char )_SendMessageW( g_hWnd_ssl_version, CB_GETCURSEL, 0, 0 );

	save_config();

	if ( !cfg_enable_web_server )
	{
		StopWebServer();
	}

	if ( enable_changed )
	{
		EnableDisableMenus( false );
	}
}

void Enable_Disable_Windows( BOOL enable )
{
	_EnableWindow( g_hWnd_static_hoz1, enable );
	_EnableWindow( g_hWnd_static_hoz2, enable );

	_EnableWindow( g_hWnd_chk_type_hostname, enable );
	_EnableWindow( g_hWnd_chk_type_ip_address, enable );

	_EnableWindow( g_hWnd_hostname, enable );
	_EnableWindow( g_hWnd_ip_address, enable );

	_EnableWindow( g_hWnd_static_colon, enable );

	_EnableWindow( g_hWnd_static_port, enable );
	_EnableWindow( g_hWnd_port, enable );

	_EnableWindow( g_hWnd_chk_auto_start, enable );
	_EnableWindow( g_hWnd_chk_verify_origin, enable );
	_EnableWindow( g_hWnd_chk_keep_alive, enable );

	_EnableWindow( g_hWnd_static_web_server_directory, enable );
	_EnableWindow( g_hWnd_web_server_directory, enable );
	_EnableWindow( g_hWnd_btn_web_server_directory, enable );

	_EnableWindow( g_hWnd_static_thread_count, enable );
	_EnableWindow( g_hWnd_thread_count, enable );

	_EnableWindow( g_hWnd_static_resource_cache_size, enable );
	_EnableWindow( g_hWnd_resource_cache_size, enable );

	_EnableWindow( g_hWnd_chk_use_authentication, enable );

	_EnableWindow( g_hWnd_chk_enable_ssl, enable );
}

void Enable_Disable_SSL_Windows( BOOL enable )
{
	_EnableWindow( g_hWnd_chk_type_pkcs, enable );
	_EnableWindow( g_hWnd_chk_type_pair, enable );

	_EnableWindow( g_hWnd_static_certificate_pkcs_location, enable );
	_EnableWindow( g_hWnd_certificate_pkcs_location, enable );
	_EnableWindow( g_hWnd_btn_certificate_pkcs_location, enable );

	_EnableWindow( g_hWnd_static_certificate_pkcs_password, enable );
	_EnableWindow( g_hWnd_certificate_pkcs_password, enable );

	_EnableWindow( g_hWnd_static_certificate_cer_location, enable );
	_EnableWindow( g_hWnd_certificate_cer_location, enable );
	_EnableWindow( g_hWnd_btn_certificate_cer_location, enable );

	_EnableWindow( g_hWnd_static_certificate_key_location, enable );
	_EnableWindow( g_hWnd_certificate_key_location, enable );
	_EnableWindow( g_hWnd_btn_certificate_key_location, enable );

	_EnableWindow( g_hWnd_static_ssl_version, enable );
	_EnableWindow( g_hWnd_ssl_version, enable );
}

void Enable_Disable_Authentication_Windows( BOOL enable )
{
	_EnableWindow( g_hWnd_static_authentication_username, enable );
	_EnableWindow( g_hWnd_authentication_username, enable );
	_EnableWindow( g_hWnd_static_authentication_password, enable );
	_EnableWindow( g_hWnd_authentication_password, enable );
}

void Set_Window_Settings()
{
	if ( cfg_address_type == 1 )
	{
		_SendMessageW( g_hWnd_chk_type_ip_address, BM_SETCHECK, BST_CHECKED, 0 );
		_SendMessageW( g_hWnd_chk_type_hostname, BM_SETCHECK, BST_UNCHECKED, 0 );

		_ShowWindow( g_hWnd_hostname, SW_HIDE );
		_ShowWindow( g_hWnd_ip_address, SW_SHOW );
	}
	else
	{
		_SendMessageW( g_hWnd_chk_type_hostname, BM_SETCHECK, BST_CHECKED, 0 );
		_SendMessageW( g_hWnd_chk_type_ip_address, BM_SETCHECK, BST_UNCHECKED, 0 );

		_ShowWindow( g_hWnd_ip_address, SW_HIDE );
		_ShowWindow( g_hWnd_hostname, SW_SHOW );	
	}

	if ( cfg_hostname == NULL )
	{
		_SendMessageW( g_hWnd_hostname, WM_SETTEXT, 0, ( LPARAM )L"localhost" );
	}
	else
	{
		_SendMessageW( g_hWnd_hostname, WM_SETTEXT, 0, ( LPARAM )cfg_hostname );
	}

	_SendMessageW( g_hWnd_ip_address, IPM_SETADDRESS, 0, cfg_ip_address );

	char value[ 21 ];
	_memzero( value, sizeof( char ) * 21 );
	__snprintf( value, 21, "%hu", cfg_port );
	_SendMessageA( g_hWnd_port, WM_SETTEXT, 0, ( LPARAM )value );

	_SendMessageW( g_hWnd_chk_auto_start, BM_SETCHECK, ( cfg_auto_start ? BST_CHECKED : BST_UNCHECKED ), 0 );
	_SendMessageW( g_hWnd_chk_verify_origin, BM_SETCHECK, ( cfg_verify_origin ? BST_CHECKED : BST_UNCHECKED ), 0 );
	_SendMessageW( g_hWnd_chk_keep_alive, BM_SETCHECK, ( cfg_allow_keep_alive_requests ? BST_CHECKED : BST_UNCHECKED ), 0 );

	_SendMessageW( g_hWnd_chk_enable_ssl, BM_SETCHECK, ( cfg_enable_ssl ? BST_CHECKED : BST_UNCHECKED ), 0 );

	_SendMessageW( g_hWnd_chk_use_authentication, BM_SETCHECK, ( cfg_use_authentication ? BST_CHECKED : BST_UNCHECKED ), 0 );

	_SendMessageW( g_hWnd_web_server_directory, WM_SETTEXT, 0, ( LPARAM )cfg_document_root_directory );

	_SendMessageW( g_hWnd_authentication_username, WM_SETTEXT, 0, ( LPARAM )cfg_authentication_username );
	_SendMessageW( g_hWnd_authentication_password, WM_SETTEXT, 0, ( LPARAM )cfg_authentication_password );

	__snprintf( value, 21, "%lu", cfg_thread_count );
	_SendMessageA( g_hWnd_thread_count, WM_SETTEXT, 0, ( LPARAM )value );

	__snprintf( value, 21, "%llu", cfg_resource_cache_size );
	_SendMessageA( g_hWnd_resource_cache_size, WM_SETTEXT, 0, ( LPARAM )value );

	if ( cfg_certificate_type == 1 )
	{
		_SendMessageW( g_hWnd_chk_type_pair, BM_SETCHECK, BST_CHECKED, 0 );
		_SendMessageW( g_hWnd_chk_type_pkcs, BM_SETCHECK, BST_UNCHECKED, 0 );

		_ShowWindow( g_hWnd_static_certificate_pkcs_location, SW_HIDE );
		_ShowWindow( g_hWnd_certificate_pkcs_location, SW_HIDE );
		_ShowWindow( g_hWnd_btn_certificate_pkcs_location, SW_HIDE );

		_ShowWindow( g_hWnd_static_certificate_pkcs_password, SW_HIDE );
		_ShowWindow( g_hWnd_certificate_pkcs_password, SW_HIDE );

		_ShowWindow( g_hWnd_static_certificate_cer_location, SW_SHOW );
		_ShowWindow( g_hWnd_certificate_cer_location, SW_SHOW );
		_ShowWindow( g_hWnd_btn_certificate_cer_location, SW_SHOW );

		_ShowWindow( g_hWnd_static_certificate_key_location, SW_SHOW );
		_ShowWindow( g_hWnd_certificate_key_location, SW_SHOW );
		_ShowWindow( g_hWnd_btn_certificate_key_location, SW_SHOW );
	}
	else
	{
		_SendMessageW( g_hWnd_chk_type_pkcs, BM_SETCHECK, BST_CHECKED, 0 );
		_SendMessageW( g_hWnd_chk_type_pair, BM_SETCHECK, BST_UNCHECKED, 0 );

		_ShowWindow( g_hWnd_static_certificate_cer_location, SW_HIDE );
		_ShowWindow( g_hWnd_certificate_cer_location, SW_HIDE );
		_ShowWindow( g_hWnd_btn_certificate_cer_location, SW_HIDE );

		_ShowWindow( g_hWnd_static_certificate_key_location, SW_HIDE );
		_ShowWindow( g_hWnd_certificate_key_location, SW_HIDE );
		_ShowWindow( g_hWnd_btn_certificate_key_location, SW_HIDE );

		_ShowWindow( g_hWnd_static_certificate_pkcs_location, SW_SHOW );
		_ShowWindow( g_hWnd_certificate_pkcs_location, SW_SHOW );
		_ShowWindow( g_hWnd_btn_certificate_pkcs_location, SW_SHOW );

		_ShowWindow( g_hWnd_static_certificate_pkcs_password, SW_SHOW );
		_ShowWindow( g_hWnd_certificate_pkcs_password, SW_SHOW );
	}

	_SendMessageW( g_hWnd_certificate_pkcs_location, WM_SETTEXT, 0, ( LPARAM )cfg_certificate_pkcs_file_name );
	_SendMessageW( g_hWnd_certificate_pkcs_password, WM_SETTEXT, 0, ( LPARAM )cfg_certificate_pkcs_password );

	_SendMessageW( g_hWnd_certificate_cer_location, WM_SETTEXT, 0, ( LPARAM )cfg_certificate_cer_file_name );
	_SendMessageW( g_hWnd_certificate_key_location, WM_SETTEXT, 0, ( LPARAM )cfg_certificate_key_file_name );

	_SendMessageW( g_hWnd_ssl_version, CB_SETCURSEL, cfg_ssl_version, 0 );

	if ( cfg_enable_web_server )
	{
		_SendMessageW( g_hWnd_chk_web_server, BM_SETCHECK, BST_CHECKED, 0 );
		Enable_Disable_Windows( TRUE );

		Enable_Disable_SSL_Windows( ( ( cfg_enable_ssl ) ? TRUE : FALSE ) );
		Enable_Disable_Authentication_Windows( ( ( cfg_use_authentication ) ? TRUE : FALSE ) );
	}
	else
	{
		_SendMessageW( g_hWnd_chk_web_server, BM_SETCHECK, BST_UNCHECKED, 0 );
		Enable_Disable_Windows( FALSE );

		Enable_Disable_SSL_Windows( FALSE );
		Enable_Disable_Authentication_Windows( FALSE );
	}
}

LRESULT CALLBACK WebServerTabWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch ( msg )
    {
		case WM_CREATE:
		{
			RECT rc;
			_GetClientRect( hWnd, &rc );

			g_hWnd_chk_web_server = _CreateWindowW( WC_BUTTON, ST_Enable_web_server_, BS_AUTOCHECKBOX | WS_CHILD | WS_TABSTOP | WS_VISIBLE, 0, 0, 200, 20, hWnd, ( HMENU )BTN_WEB_SERVER, NULL, NULL );

			g_hWnd_static_hoz1 = _CreateWindowW( WC_STATIC, NULL, SS_ETCHEDHORZ | WS_CHILD | WS_VISIBLE, 0, 25, rc.right - 10, 5, hWnd, NULL, NULL, NULL );

			g_hWnd_chk_type_hostname = _CreateWindowW( WC_BUTTON, ST_Hostname___IPv6_address_, BS_AUTORADIOBUTTON | WS_CHILD | WS_GROUP | WS_TABSTOP | WS_VISIBLE, 0, 30, 150, 20, hWnd, ( HMENU )BTN_TYPE_HOST, NULL, NULL );
			g_hWnd_chk_type_ip_address = _CreateWindowW( WC_BUTTON, ST_IPv4_address_, BS_AUTORADIOBUTTON | WS_CHILD | WS_TABSTOP | WS_VISIBLE, 150, 30, 95, 20, hWnd, ( HMENU )BTN_TYPE_IP_ADDRESS, NULL, NULL );

			g_hWnd_hostname = _CreateWindowExW( WS_EX_CLIENTEDGE, WC_EDIT, NULL, ES_AUTOHSCROLL | ES_CENTER | WS_CHILD | WS_TABSTOP | WS_VISIBLE, 0, 50, 235, 20, hWnd, ( HMENU )EDIT_HOST, NULL, NULL );
			g_hWnd_ip_address = _CreateWindowExW( WS_EX_CLIENTEDGE, WC_IPADDRESS, NULL, WS_CHILD | WS_TABSTOP, 0, 50, 235, 20, hWnd, ( HMENU )EDIT_IP_ADDRESS, NULL, NULL );


			g_hWnd_static_colon = _CreateWindowW( WC_STATIC, ST__, WS_CHILD | WS_VISIBLE, 239, 52, 75, 15, hWnd, NULL, NULL, NULL );


			g_hWnd_static_port = _CreateWindowW( WC_STATIC, ST_Port_, WS_CHILD | WS_VISIBLE, 245, 35, 75, 15, hWnd, NULL, NULL, NULL );
			g_hWnd_port = _CreateWindowExW( WS_EX_CLIENTEDGE, WC_EDIT, NULL, ES_AUTOHSCROLL | ES_CENTER | ES_NUMBER | WS_CHILD | WS_TABSTOP | WS_VISIBLE, 245, 50, 60, 20, hWnd, ( HMENU )EDIT_PORT, NULL, NULL );



			g_hWnd_static_web_server_directory = _CreateWindowW( WC_STATIC, ST_Document_root_directory_, WS_CHILD | WS_VISIBLE, 0, 80, 150, 15, hWnd, NULL, NULL, NULL );
			g_hWnd_web_server_directory = _CreateWindowExW( WS_EX_CLIENTEDGE, WC_EDIT, NULL, ES_AUTOHSCROLL | ES_READONLY | WS_CHILD | WS_TABSTOP | WS_VISIBLE, 0, 95, 345, 20, hWnd, ( HMENU )EDIT_SERVER_DIRECTORY, NULL, NULL );
			g_hWnd_btn_web_server_directory = _CreateWindowW( WC_BUTTON, ST_BTN___, WS_CHILD | WS_TABSTOP | WS_VISIBLE, 350, 95, 35, 20, hWnd, ( HMENU )BTN_SERVER_DIRECTORY, NULL, NULL );


			g_hWnd_chk_use_authentication = _CreateWindowW( WC_BUTTON, ST_Require_authentication_, BS_AUTOCHECKBOX | WS_CHILD | WS_TABSTOP | WS_VISIBLE, 0, 125, 200, 20, hWnd, ( HMENU )BTN_USE_AUTHENTICATION, NULL, NULL );
			g_hWnd_static_authentication_username = _CreateWindowW( WC_STATIC, ST_Username_, WS_CHILD | WS_VISIBLE, 15, 145, 90, 15, hWnd, NULL, NULL, NULL );
			g_hWnd_authentication_username = _CreateWindowExW( WS_EX_CLIENTEDGE, WC_EDIT, NULL, ES_AUTOHSCROLL | WS_CHILD | WS_TABSTOP | WS_VISIBLE, 15, 160, 150, 20, hWnd, ( HMENU )EDIT_AUTHENTICATION_USERNAME, NULL, NULL );
			g_hWnd_static_authentication_password = _CreateWindowW( WC_STATIC, ST_Password_, WS_CHILD | WS_VISIBLE, 170, 145, 90, 15, hWnd, NULL, NULL, NULL );
			g_hWnd_authentication_password = _CreateWindowExW( WS_EX_CLIENTEDGE, WC_EDIT, NULL, ES_AUTOHSCROLL | ES_PASSWORD | WS_CHILD | WS_TABSTOP | WS_VISIBLE, 170, 160, 150, 20, hWnd, ( HMENU )EDIT_AUTHENTICATION_PASSWORD, NULL, NULL );

			g_hWnd_chk_verify_origin = _CreateWindowW( WC_BUTTON, ST_Verify_WebSocket_origin, BS_AUTOCHECKBOX | WS_CHILD | WS_TABSTOP | WS_VISIBLE, 0, 190, 200, 20, hWnd, ( HMENU )BTN_VERIFY_ORIGIN, NULL, NULL );
			g_hWnd_chk_keep_alive = _CreateWindowW( WC_BUTTON, ST_Allow_Keep_Alive_requests, BS_AUTOCHECKBOX | WS_CHILD | WS_TABSTOP | WS_VISIBLE, 0, 210, 200, 20, hWnd, ( HMENU )BTN_KEEP_ALIVE, NULL, NULL );
			g_hWnd_chk_auto_start = _CreateWindowW( WC_BUTTON, ST_Start_web_server_upon_startup, BS_AUTOCHECKBOX | WS_CHILD | WS_TABSTOP | WS_VISIBLE, 0, 230, 200, 20, hWnd, ( HMENU )BTN_AUTO_START, NULL, NULL );


			g_hWnd_static_resource_cache_size = _CreateWindowW( WC_STATIC, ST_Resource_cache_size__bytes__, WS_CHILD | WS_VISIBLE, 0, 260, 175, 15, hWnd, NULL, NULL, NULL );
			g_hWnd_resource_cache_size = _CreateWindowExW( WS_EX_CLIENTEDGE, WC_EDIT, NULL, ES_AUTOHSCROLL | ES_CENTER | ES_NUMBER | WS_CHILD | WS_TABSTOP | WS_VISIBLE, 0, 275, 160, 20, hWnd, ( HMENU )EDIT_CACHE_SIZE, NULL, NULL );

			g_hWnd_static_thread_count = _CreateWindowW( WC_STATIC, ST_Thread_pool_count_, WS_CHILD | WS_VISIBLE, 0, 305, 150, 15, hWnd, NULL, NULL, NULL );
			g_hWnd_thread_count = _CreateWindowExW( WS_EX_CLIENTEDGE, WC_EDIT, NULL, ES_AUTOHSCROLL | ES_CENTER | ES_NUMBER | WS_CHILD | WS_TABSTOP | WS_VISIBLE, 0, 320, 85, 20, hWnd, ( HMENU )EDIT_THREAD_COUNT, NULL, NULL );

			// Keep this unattached. Looks ugly inside the text box.
			HWND hWnd_ud_thread_count = _CreateWindowW( UPDOWN_CLASS, NULL, /*UDS_ALIGNRIGHT |*/ UDS_ARROWKEYS | UDS_NOTHOUSANDS | UDS_SETBUDDYINT | WS_CHILD | WS_VISIBLE, 85, 319, _GetSystemMetrics( SM_CXVSCROLL ), 22, hWnd, NULL, NULL, NULL );

			_SendMessageW( hWnd_ud_thread_count, UDM_SETBUDDY, ( WPARAM )g_hWnd_thread_count, 0 );
            _SendMessageW( hWnd_ud_thread_count, UDM_SETBASE, 10, 0 );
			_SendMessageW( hWnd_ud_thread_count, UDM_SETRANGE32, 1, max_threads );
			_SendMessageW( hWnd_ud_thread_count, UDM_SETPOS, 0, cfg_thread_count );

			g_hWnd_chk_enable_ssl = _CreateWindowW( WC_BUTTON, ST_Enable_SSL___TLS_, BS_AUTOCHECKBOX | WS_CHILD | WS_TABSTOP | WS_VISIBLE, 0, 350, 150, 20, hWnd, ( HMENU )BTN_ENABLE_SSL, NULL, NULL );

			g_hWnd_static_hoz2 = _CreateWindowW( WC_STATIC, NULL, SS_ETCHEDHORZ | WS_CHILD | WS_VISIBLE, 0, 375, rc.right - 10, 5, hWnd, NULL, NULL, NULL );
			

			g_hWnd_chk_type_pkcs = _CreateWindowW( WC_BUTTON, ST_PKCS_NUM12_, BS_AUTORADIOBUTTON | WS_CHILD | WS_GROUP | WS_TABSTOP | WS_VISIBLE, 0, 380, 90, 20, hWnd, ( HMENU )BTN_TYPE_PKCS, NULL, NULL );
			g_hWnd_chk_type_pair = _CreateWindowW( WC_BUTTON, ST_Public___Private_key_pair_, BS_AUTORADIOBUTTON | WS_CHILD | WS_TABSTOP | WS_VISIBLE, 90, 380, 150, 20, hWnd, ( HMENU )BTN_TYPE_PAIR, NULL, NULL );


			g_hWnd_static_certificate_pkcs_location = _CreateWindowW( WC_STATIC, ST_PKCS_NUM12_file_, WS_CHILD | WS_VISIBLE, 15, 405, 150, 15, hWnd, NULL, NULL, NULL );
			g_hWnd_certificate_pkcs_location = _CreateWindowExW( WS_EX_CLIENTEDGE, WC_EDIT, NULL, ES_AUTOHSCROLL | ES_READONLY | WS_CHILD | WS_TABSTOP | WS_VISIBLE, 15, 420, 330, 20, hWnd, ( HMENU )EDIT_CERTIFICATE_PKCS, NULL, NULL );
			g_hWnd_btn_certificate_pkcs_location = _CreateWindowW( WC_BUTTON, ST_BTN___, WS_CHILD | WS_TABSTOP | WS_VISIBLE, 350, 420, 35, 20, hWnd, ( HMENU )BTN_CERTIFICATE_PKCS, NULL, NULL );

			g_hWnd_static_certificate_pkcs_password = _CreateWindowW( WC_STATIC, ST_PKCS_NUM12_password_, WS_CHILD | WS_VISIBLE, 15, 445, 150, 15, hWnd, NULL, NULL, NULL );
			g_hWnd_certificate_pkcs_password = _CreateWindowExW( WS_EX_CLIENTEDGE, WC_EDIT, NULL, ES_AUTOHSCROLL | ES_PASSWORD | WS_CHILD | WS_TABSTOP | WS_VISIBLE, 15, 460, 150, 20, hWnd, ( HMENU )EDIT_CERTIFICATE_PKCS_PASSWORD, NULL, NULL );


			

			g_hWnd_static_certificate_cer_location = _CreateWindowW( WC_STATIC, ST_Certificate_file_, WS_CHILD | WS_VISIBLE, 15, 405, 150, 15, hWnd, NULL, NULL, NULL );
			g_hWnd_certificate_cer_location = _CreateWindowExW( WS_EX_CLIENTEDGE, WC_EDIT, NULL, ES_AUTOHSCROLL | ES_READONLY | WS_CHILD | WS_TABSTOP | WS_VISIBLE, 15, 420, 330, 20, hWnd, ( HMENU )EDIT_CERTIFICATE_CER, NULL, NULL );
			g_hWnd_btn_certificate_cer_location = _CreateWindowW( WC_BUTTON, ST_BTN___, WS_CHILD | WS_TABSTOP | WS_VISIBLE, 350, 420, 35, 20, hWnd, ( HMENU )BTN_CERTIFICATE_CER, NULL, NULL );

			g_hWnd_static_certificate_key_location = _CreateWindowW( WC_STATIC, ST_Key_file_, WS_CHILD | WS_VISIBLE, 15, 445, 150, 15, hWnd, NULL, NULL, NULL );
			g_hWnd_certificate_key_location = _CreateWindowExW( WS_EX_CLIENTEDGE, WC_EDIT, NULL, ES_AUTOHSCROLL | ES_READONLY | WS_CHILD | WS_TABSTOP | WS_VISIBLE, 15, 460, 330, 20, hWnd, ( HMENU )EDIT_CERTIFICATE_KEY, NULL, NULL );
			g_hWnd_btn_certificate_key_location = _CreateWindowW( WC_BUTTON, ST_BTN___, WS_CHILD | WS_TABSTOP | WS_VISIBLE, 350, 460, 35, 20, hWnd, ( HMENU )BTN_CERTIFICATE_KEY, NULL, NULL );



			g_hWnd_static_ssl_version = _CreateWindowW( WC_STATIC, ST_SSL___TLS_version_, WS_CHILD | WS_VISIBLE, 0, 490, 150, 15, hWnd, NULL, NULL, NULL );
			g_hWnd_ssl_version = _CreateWindowExW( WS_EX_CLIENTEDGE, WC_COMBOBOX, NULL, CBS_AUTOHSCROLL | CBS_DROPDOWNLIST | WS_CHILD | WS_TABSTOP | WS_VSCROLL | WS_VISIBLE, 0, 505, 100, 20, hWnd, ( HMENU )CB_SSL_VERSION, NULL, NULL );
			_SendMessageW( g_hWnd_ssl_version, CB_ADDSTRING, 0, ( LPARAM )ST_SSL_2_0 );
			_SendMessageW( g_hWnd_ssl_version, CB_ADDSTRING, 0, ( LPARAM )ST_SSL_3_0 );
			_SendMessageW( g_hWnd_ssl_version, CB_ADDSTRING, 0, ( LPARAM )ST_TLS_1_0 );
			_SendMessageW( g_hWnd_ssl_version, CB_ADDSTRING, 0, ( LPARAM )ST_TLS_1_1 );
			_SendMessageW( g_hWnd_ssl_version, CB_ADDSTRING, 0, ( LPARAM )ST_TLS_1_2 );


			_SendMessageW( g_hWnd_hostname, EM_LIMITTEXT, 254, 0 );
			_SendMessageW( g_hWnd_port, EM_LIMITTEXT, 5, 0 );
			_SendMessageW( g_hWnd_thread_count, EM_LIMITTEXT, 10, 0 );
			_SendMessageW( g_hWnd_resource_cache_size, EM_LIMITTEXT, 20, 0 );
			_SendMessageW( g_hWnd_certificate_pkcs_password, EM_LIMITTEXT, 1024, 0 );	// 1024 characters + 1 NULL


			web_server_tab_scroll_pos = 0;
			web_server_tab_height = ( rc.bottom - rc.top ) + 255;

			SCROLLINFO si;
			si.cbSize = sizeof( SCROLLINFO );
			si.fMask = SIF_ALL;
			si.nMin = 0;
			si.nMax = web_server_tab_height;
			si.nPage = ( ( web_server_tab_height / 2 ) + ( ( web_server_tab_height % 2 ) != 0 ) );
			si.nPos = 0;
			_SetScrollInfo( hWnd, SB_VERT, &si, TRUE );

			
			_SendMessageW( g_hWnd_chk_web_server, WM_SETFONT, ( WPARAM )hFont, 0 );

			_SendMessageW( g_hWnd_chk_type_hostname, WM_SETFONT, ( WPARAM )hFont, 0 );
			_SendMessageW( g_hWnd_chk_type_ip_address, WM_SETFONT, ( WPARAM )hFont, 0 );

			_SendMessageW( g_hWnd_hostname, WM_SETFONT, ( WPARAM )hFont, 0 );

			_SendMessageW( g_hWnd_static_colon, WM_SETFONT, ( WPARAM )hFont, 0 );

			_SendMessageW( g_hWnd_static_port, WM_SETFONT, ( WPARAM )hFont, 0 );
			_SendMessageW( g_hWnd_port, WM_SETFONT, ( WPARAM )hFont, 0 );

			_SendMessageW( g_hWnd_chk_enable_ssl, WM_SETFONT, ( WPARAM )hFont, 0 );

			_SendMessageW( g_hWnd_static_web_server_directory, WM_SETFONT, ( WPARAM )hFont, 0 );
			_SendMessageW( g_hWnd_web_server_directory, WM_SETFONT, ( WPARAM )hFont, 0 );
			_SendMessageW( g_hWnd_btn_web_server_directory, WM_SETFONT, ( WPARAM )hFont, 0 );


			_SendMessageW( g_hWnd_chk_use_authentication, WM_SETFONT, ( WPARAM )hFont, 0 );
			_SendMessageW( g_hWnd_static_authentication_username, WM_SETFONT, ( WPARAM )hFont, 0 );
			_SendMessageW( g_hWnd_authentication_username, WM_SETFONT, ( WPARAM )hFont, 0 );
			_SendMessageW( g_hWnd_static_authentication_password, WM_SETFONT, ( WPARAM )hFont, 0 );
			_SendMessageW( g_hWnd_authentication_password, WM_SETFONT, ( WPARAM )hFont, 0 );

			_SendMessageW( g_hWnd_static_thread_count, WM_SETFONT, ( WPARAM )hFont, 0 );
			_SendMessageW( g_hWnd_thread_count, WM_SETFONT, ( WPARAM )hFont, 0 );

			_SendMessageW( g_hWnd_static_resource_cache_size, WM_SETFONT, ( WPARAM )hFont, 0 );
			_SendMessageW( g_hWnd_resource_cache_size, WM_SETFONT, ( WPARAM )hFont, 0 );

			_SendMessageW( g_hWnd_chk_type_pkcs, WM_SETFONT, ( WPARAM )hFont, 0 );
			_SendMessageW( g_hWnd_chk_type_pair, WM_SETFONT, ( WPARAM )hFont, 0 );

			_SendMessageW( g_hWnd_static_certificate_pkcs_location, WM_SETFONT, ( WPARAM )hFont, 0 );
			_SendMessageW( g_hWnd_certificate_pkcs_location, WM_SETFONT, ( WPARAM )hFont, 0 );
			_SendMessageW( g_hWnd_btn_certificate_pkcs_location, WM_SETFONT, ( WPARAM )hFont, 0 );

			_SendMessageW( g_hWnd_static_certificate_pkcs_password, WM_SETFONT, ( WPARAM )hFont, 0 );
			_SendMessageW( g_hWnd_certificate_pkcs_password, WM_SETFONT, ( WPARAM )hFont, 0 );

			_SendMessageW( g_hWnd_static_certificate_cer_location, WM_SETFONT, ( WPARAM )hFont, 0 );
			_SendMessageW( g_hWnd_certificate_cer_location, WM_SETFONT, ( WPARAM )hFont, 0 );
			_SendMessageW( g_hWnd_btn_certificate_cer_location, WM_SETFONT, ( WPARAM )hFont, 0 );

			_SendMessageW( g_hWnd_static_certificate_key_location, WM_SETFONT, ( WPARAM )hFont, 0 );
			_SendMessageW( g_hWnd_certificate_key_location, WM_SETFONT, ( WPARAM )hFont, 0 );
			_SendMessageW( g_hWnd_btn_certificate_key_location, WM_SETFONT, ( WPARAM )hFont, 0 );

			_SendMessageW( g_hWnd_static_ssl_version, WM_SETFONT, ( WPARAM )hFont, 0 );
			_SendMessageW( g_hWnd_ssl_version, WM_SETFONT, ( WPARAM )hFont, 0 );

			_SendMessageW( g_hWnd_chk_auto_start, WM_SETFONT, ( WPARAM )hFont, 0 );
			_SendMessageW( g_hWnd_chk_verify_origin, WM_SETFONT, ( WPARAM )hFont, 0 );
			_SendMessageW( g_hWnd_chk_keep_alive, WM_SETFONT, ( WPARAM )hFont, 0 );
			

			// Stupid control likes to delete the font object. :-/
			// We'll make a copy.
			LOGFONT lf;
			_memzero( &lf, sizeof( LOGFONT ) );
			_GetObjectW( hFont, sizeof( LOGFONT ), &lf );
			hFont_copy = _CreateFontIndirectW( &lf );
			_SendMessageW( g_hWnd_ip_address, WM_SETFONT, ( WPARAM )hFont_copy, 0 );


			if ( cfg_certificate_pkcs_file_name != NULL )
			{
				/*int certificate_pkcs_file_name_length = lstrlenW( cfg_certificate_pkcs_file_name );
				certificate_pkcs_file_name = ( wchar_t * )GlobalAlloc( GMEM_FIXED, sizeof( wchar_t ) * ( certificate_pkcs_file_name_length + 1 ) );
				_wmemcpy_s( certificate_pkcs_file_name, certificate_pkcs_file_name_length + 1, cfg_certificate_pkcs_file_name, certificate_pkcs_file_name_length );
				*( certificate_pkcs_file_name + certificate_pkcs_file_name_length ) = 0;	// Sanity.*/

				certificate_pkcs_file_name = GlobalStrDupW( cfg_certificate_pkcs_file_name );
			}

			if ( cfg_certificate_cer_file_name != NULL )
			{
				/*int certificate_cer_file_name_length = lstrlenW( cfg_certificate_cer_file_name );
				certificate_cer_file_name = ( wchar_t * )GlobalAlloc( GMEM_FIXED, sizeof( wchar_t ) * ( certificate_cer_file_name_length + 1 ) );
				_wmemcpy_s( certificate_cer_file_name, certificate_cer_file_name_length + 1, cfg_certificate_cer_file_name, certificate_cer_file_name_length );
				*( certificate_cer_file_name + certificate_cer_file_name_length ) = 0;	// Sanity.*/

				certificate_cer_file_name = GlobalStrDupW( cfg_certificate_cer_file_name );
			}

			if ( cfg_certificate_key_file_name != NULL )
			{
				/*int certificate_key_file_name_length = lstrlenW( cfg_certificate_key_file_name );
				certificate_key_file_name = ( wchar_t * )GlobalAlloc( GMEM_FIXED, sizeof( wchar_t ) * ( certificate_key_file_name_length + 1 ) );
				_wmemcpy_s( certificate_key_file_name, certificate_key_file_name_length + 1, cfg_certificate_key_file_name, certificate_key_file_name_length );
				*( certificate_key_file_name + certificate_key_file_name_length ) = 0;	// Sanity.*/

				certificate_key_file_name = GlobalStrDupW( cfg_certificate_key_file_name );
			}

			if ( cfg_document_root_directory != NULL )
			{
				/*int document_root_directory_length = lstrlenW( cfg_document_root_directory );
				t_document_root_directory = ( wchar_t * )GlobalAlloc( GMEM_FIXED, sizeof( wchar_t ) * ( document_root_directory_length + 1 ) );
				_wmemcpy_s( t_document_root_directory, document_root_directory_length + 1, cfg_document_root_directory, document_root_directory_length );
				*( t_document_root_directory + document_root_directory_length ) = 0;	// Sanity.*/

				t_document_root_directory = GlobalStrDupW( cfg_document_root_directory );
			}

			Set_Window_Settings();

			return 0;
		}
		break;

		case WM_CTLCOLORSTATIC:
		{
			return ( LRESULT )( _GetSysColorBrush( COLOR_WINDOW ) );
		}
		break;

		case WM_MBUTTONUP:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		{
			_SetFocus( hWnd );

			return 0;
		}
		break;

		case WM_MOUSEWHEEL:
		case WM_VSCROLL:
		{
			int delta = 0;

			if ( msg == WM_VSCROLL )
			{
				// Only process the standard scroll bar.
				if ( lParam != NULL )
				{
					return _DefWindowProcW( hWnd, msg, wParam, lParam );
				}

				switch ( LOWORD( wParam ) )
				{
					case SB_LINEUP: { delta = -10; } break;
					case SB_LINEDOWN: { delta = 10; } break;
					case SB_PAGEUP: { delta = -50; } break;
					case SB_PAGEDOWN: { delta = 50; } break;
					//case SB_THUMBPOSITION:
					case SB_THUMBTRACK: { delta = ( int )HIWORD( wParam ) - web_server_tab_scroll_pos; } break;
					default: { return 0; } break;
				}
			}
			else if ( msg == WM_MOUSEWHEEL )
			{
				delta = -( GET_WHEEL_DELTA_WPARAM( wParam ) / WHEEL_DELTA ) * 20;
			}

			web_server_tab_scroll_pos += delta;

			if ( web_server_tab_scroll_pos < 0 )
			{
				delta -= web_server_tab_scroll_pos;
				web_server_tab_scroll_pos = 0;
			}
			else if ( web_server_tab_scroll_pos > ( ( web_server_tab_height / 2 ) + ( ( web_server_tab_height % 2 ) != 0 ) ) )
			{
				delta -= ( web_server_tab_scroll_pos - ( ( web_server_tab_height / 2 ) + ( ( web_server_tab_height % 2 ) != 0 ) ) );
				web_server_tab_scroll_pos = ( ( web_server_tab_height / 2 ) + ( ( web_server_tab_height % 2 ) != 0 ) );
			}

			if ( delta != 0 )
			{
				_SetScrollPos( hWnd, SB_VERT, web_server_tab_scroll_pos, TRUE );
				_ScrollWindow( hWnd, 0, -delta, NULL, NULL );
			}

			return 0;
		}
		break;

		case WM_COMMAND:
		{
			switch( LOWORD( wParam ) )
			{
				case BTN_WEB_SERVER:
				{
					if ( _SendMessageW( g_hWnd_chk_web_server, BM_GETCHECK, 0, 0 ) == BST_CHECKED )
					{
						Enable_Disable_Windows( TRUE );
						Enable_Disable_SSL_Windows( ( ( _SendMessageW( g_hWnd_chk_enable_ssl, BM_GETCHECK, 0, 0 ) == BST_CHECKED ) ? TRUE : FALSE ) );
						Enable_Disable_Authentication_Windows( ( ( _SendMessageW( g_hWnd_chk_use_authentication, BM_GETCHECK, 0, 0 ) == BST_CHECKED ) ? TRUE : FALSE ) );
					}
					else
					{
						Enable_Disable_Windows( FALSE );
						Enable_Disable_SSL_Windows( FALSE );
						Enable_Disable_Authentication_Windows( FALSE );
					}

					if ( state_changed != NULL )
					{
						*state_changed = true;
						_EnableWindow( *g_hWnd_apply, TRUE );
					}
				}
				break;

				case EDIT_PORT:
				{
					if ( HIWORD( wParam ) == EN_UPDATE )
					{
						DWORD sel_start = 0;

						char value[ 11 ];
						_SendMessageA( ( HWND )lParam, WM_GETTEXT, 6, ( LPARAM )value );
						unsigned long num = _strtoul( value, NULL, 10 );

						if ( num > 65535 )
						{
							_SendMessageA( ( HWND )lParam, EM_GETSEL, ( WPARAM )&sel_start, NULL );

							_SendMessageA( ( HWND )lParam, WM_SETTEXT, 0, ( LPARAM )"65535" );

							_SendMessageA( ( HWND )lParam, EM_SETSEL, sel_start, sel_start );
						}
						else if ( num == 0 )
						{
							_SendMessageA( ( HWND )lParam, EM_GETSEL, ( WPARAM )&sel_start, NULL );

							_SendMessageA( ( HWND )lParam, WM_SETTEXT, 0, ( LPARAM )"1" );

							_SendMessageA( ( HWND )lParam, EM_SETSEL, sel_start, sel_start );
						}

						if ( state_changed != NULL && num != cfg_port )
						{
							*state_changed = true;
							_EnableWindow( *g_hWnd_apply, TRUE );
						}
					}
				}
				break;

				case EDIT_THREAD_COUNT:
				{
					if ( HIWORD( wParam ) == EN_UPDATE )
					{
						DWORD sel_start = 0;

						char value[ 11 ];
						_SendMessageA( ( HWND )lParam, WM_GETTEXT, 11, ( LPARAM )value );
						unsigned long num = _strtoul( value, NULL, 10 );

						if ( num > max_threads )
						{
							_SendMessageA( ( HWND )lParam, EM_GETSEL, ( WPARAM )&sel_start, NULL );

							__snprintf( value, 11, "%lu", max_threads );
							_SendMessageA( ( HWND )lParam, WM_SETTEXT, 0, ( LPARAM )value );

							_SendMessageA( ( HWND )lParam, EM_SETSEL, sel_start, sel_start );
						}
						else if ( num == 0 )
						{
							_SendMessageA( ( HWND )lParam, EM_GETSEL, ( WPARAM )&sel_start, NULL );

							_SendMessageA( ( HWND )lParam, WM_SETTEXT, 0, ( LPARAM )"1" );

							_SendMessageA( ( HWND )lParam, EM_SETSEL, sel_start, sel_start );
						}

						if ( state_changed != NULL && num != cfg_thread_count )
						{
							*state_changed = true;
							_EnableWindow( *g_hWnd_apply, TRUE );
						}
					}
				}
				break;

				case EDIT_CACHE_SIZE:
				{
					if ( HIWORD( wParam ) == EN_UPDATE )
					{
						DWORD sel_start = 0;

						char value[ 21 ];
						_SendMessageA( ( HWND )lParam, WM_GETTEXT, 21, ( LPARAM )value );
						unsigned long long num = strtoull( value );

						if ( num == ULLONG_MAX && lstrcmpA( value, "18446744073709551615" ) != 0 )
						{
							_SendMessageA( ( HWND )lParam, EM_GETSEL, ( WPARAM )&sel_start, NULL );

							// Reset the text. If our string is greater than ULLONG_MAX.
							_SendMessageA( ( HWND )lParam, WM_SETTEXT, 0, ( LPARAM )"18446744073709551615" );

							_SendMessageA( ( HWND )lParam, EM_SETSEL, sel_start, sel_start );
						}

						if ( state_changed != NULL && num != cfg_resource_cache_size )
						{
							*state_changed = true;
							_EnableWindow( *g_hWnd_apply, TRUE );
						}
					}
				}
				break;

				case EDIT_AUTHENTICATION_USERNAME:
				case EDIT_AUTHENTICATION_PASSWORD:
				case EDIT_CERTIFICATE_PKCS_PASSWORD:
				case EDIT_HOST:
				{
					if ( HIWORD( wParam ) == EN_UPDATE )
					{
						if ( state_changed != NULL )
						{
							*state_changed = true;
							_EnableWindow( *g_hWnd_apply, TRUE );
						}
					}
				}
				break;

				case EDIT_IP_ADDRESS:
				{
					if ( HIWORD( wParam ) == EN_CHANGE )
					{
						if ( state_changed != NULL )
						{
							*state_changed = true;
							_EnableWindow( *g_hWnd_apply, TRUE );
						}
					}
				}
				break;

				case BTN_TYPE_HOST:
				{
					if ( _SendMessageW( g_hWnd_chk_type_hostname, BM_GETCHECK, 0, 0 ) == BST_CHECKED )
					{
						_ShowWindow( g_hWnd_ip_address, SW_HIDE );
						_ShowWindow( g_hWnd_hostname, SW_SHOW );
					}

					if ( state_changed != NULL )
					{
						*state_changed = true;
						_EnableWindow( *g_hWnd_apply, TRUE );
					}
				}
				break;

				case BTN_TYPE_IP_ADDRESS:
				{
					if ( _SendMessageW( g_hWnd_chk_type_ip_address, BM_GETCHECK, 0, 0 ) == BST_CHECKED )
					{
						_ShowWindow( g_hWnd_hostname, SW_HIDE );
						_ShowWindow( g_hWnd_ip_address, SW_SHOW );
					}

					if ( state_changed != NULL )
					{
						*state_changed = true;
						_EnableWindow( *g_hWnd_apply, TRUE );
					}
				}
				break;

				case BTN_USE_AUTHENTICATION:
				{
					if ( _SendMessageW( g_hWnd_chk_use_authentication, BM_GETCHECK, 0, 0 ) == BST_CHECKED )
					{
						Enable_Disable_Authentication_Windows( TRUE );
					}
					else
					{
						Enable_Disable_Authentication_Windows( FALSE );
					}

					if ( state_changed != NULL )
					{
						*state_changed = true;
						_EnableWindow( *g_hWnd_apply, TRUE );
					}
				}
				break;

				case BTN_ENABLE_SSL:
				{
					if ( _SendMessageW( g_hWnd_chk_enable_ssl, BM_GETCHECK, 0, 0 ) == BST_CHECKED )
					{
						// Revert to saved port of ssl is already enabled.
						if ( cfg_enable_ssl )
						{
							char port[ 6 ];
							_memzero( port, sizeof( char ) * 6 );
							__snprintf( port, 6, "%hu", cfg_port );
							_SendMessageA( g_hWnd_port, WM_SETTEXT, 0, ( LPARAM )port );
						}
						else	// Otherwise, use the default https port.
						{
							_SendMessageA( g_hWnd_port, WM_SETTEXT, 0, ( LPARAM )"443" );
						}
						Enable_Disable_SSL_Windows( TRUE );
					}
					else
					{
						// Revert to saved port if ssl is disabled.
						if ( !cfg_enable_ssl )
						{
							char port[ 6 ];
							_memzero( port, sizeof( char ) * 6 );
							__snprintf( port, 6, "%hu", cfg_port );
							_SendMessageA( g_hWnd_port, WM_SETTEXT, 0, ( LPARAM )port );
						}
						else	// Otherwise, use the default http port.
						{
							_SendMessageA( g_hWnd_port, WM_SETTEXT, 0, ( LPARAM )"80" );
						}
						Enable_Disable_SSL_Windows( FALSE );
					}

					if ( state_changed != NULL )
					{
						*state_changed = true;
						_EnableWindow( *g_hWnd_apply, TRUE );
					}
				}
				break;

				case BTN_TYPE_PKCS:
				{
					if ( _SendMessageW( g_hWnd_chk_type_pkcs, BM_GETCHECK, 0, 0 ) == BST_CHECKED )
					{
						_ShowWindow( g_hWnd_static_certificate_cer_location, SW_HIDE );
						_ShowWindow( g_hWnd_certificate_cer_location, SW_HIDE );
						_ShowWindow( g_hWnd_btn_certificate_cer_location, SW_HIDE );

						_ShowWindow( g_hWnd_static_certificate_key_location, SW_HIDE );
						_ShowWindow( g_hWnd_certificate_key_location, SW_HIDE );
						_ShowWindow( g_hWnd_btn_certificate_key_location, SW_HIDE );

						_ShowWindow( g_hWnd_static_certificate_pkcs_location, SW_SHOW );
						_ShowWindow( g_hWnd_certificate_pkcs_location, SW_SHOW );
						_ShowWindow( g_hWnd_btn_certificate_pkcs_location, SW_SHOW );

						_ShowWindow( g_hWnd_static_certificate_pkcs_password, SW_SHOW );
						_ShowWindow( g_hWnd_certificate_pkcs_password, SW_SHOW );
					}

					if ( state_changed != NULL )
					{
						*state_changed = true;
						_EnableWindow( *g_hWnd_apply, TRUE );
					}
				}
				break;

				case BTN_TYPE_PAIR:
				{
					if ( _SendMessageW( g_hWnd_chk_type_pair, BM_GETCHECK, 0, 0 ) == BST_CHECKED )
					{
						_ShowWindow( g_hWnd_static_certificate_pkcs_location, SW_HIDE );
						_ShowWindow( g_hWnd_certificate_pkcs_location, SW_HIDE );
						_ShowWindow( g_hWnd_btn_certificate_pkcs_location, SW_HIDE );

						_ShowWindow( g_hWnd_static_certificate_pkcs_password, SW_HIDE );
						_ShowWindow( g_hWnd_certificate_pkcs_password, SW_HIDE );

						_ShowWindow( g_hWnd_static_certificate_cer_location, SW_SHOW );
						_ShowWindow( g_hWnd_certificate_cer_location, SW_SHOW );
						_ShowWindow( g_hWnd_btn_certificate_cer_location, SW_SHOW );

						_ShowWindow( g_hWnd_static_certificate_key_location, SW_SHOW );
						_ShowWindow( g_hWnd_certificate_key_location, SW_SHOW );
						_ShowWindow( g_hWnd_btn_certificate_key_location, SW_SHOW );
					}

					if ( state_changed != NULL )
					{
						*state_changed = true;
						_EnableWindow( *g_hWnd_apply, TRUE );
					}
				}
				break;

				case BTN_SERVER_DIRECTORY:
				{
					// Open a browse for folder dialog box.
					BROWSEINFO bi;
					_memzero( &bi, sizeof( BROWSEINFO ) );
					bi.hwndOwner = hWnd;
					bi.lpszTitle = ST_Select_the_root_directory;
					bi.ulFlags = BIF_EDITBOX | BIF_NEWDIALOGSTYLE | BIF_VALIDATE;

					bool destroy = true;
					#ifndef OLE32_USE_STATIC_LIB
						if ( ole32_state == OLE32_STATE_SHUTDOWN )
						{
							destroy = InitializeOle32();
						}
					#endif

					if ( destroy )
					{
						// OleInitialize calls CoInitializeEx
						_OleInitialize( NULL );
					}

					LPITEMIDLIST lpiidl = _SHBrowseForFolderW( &bi );
					if ( lpiidl )
					{
						wchar_t *directory = ( wchar_t * )GlobalAlloc( GPTR, sizeof( wchar_t ) * MAX_PATH );

						// Get the directory path from the id list.
						_SHGetPathFromIDListW( lpiidl, ( LPTSTR )directory );

						if ( directory[ 0 ] != NULL )
						{
							if ( t_document_root_directory != NULL )
							{
								GlobalFree( t_document_root_directory );
							}

							t_document_root_directory = directory;

							_SendMessageW( g_hWnd_web_server_directory, WM_SETTEXT, 0, ( LPARAM )t_document_root_directory );

							if ( state_changed != NULL )
							{
								*state_changed = true;
								_EnableWindow( *g_hWnd_apply, TRUE );
							}
						}
						else
						{
							GlobalFree( directory );
						}

						if ( destroy )
						{
							_CoTaskMemFree( lpiidl );
						}
						else	// Warn of leak if we can't free.
						{
							_MessageBoxW( NULL, L"Item ID List was not freed.", L"Web Server", 0 );
						}
					}

					if ( destroy )
					{
						_OleUninitialize();
					}
				}
				break;

				case BTN_CERTIFICATE_PKCS:
				{
					wchar_t *file_name = ( wchar_t * )GlobalAlloc( GPTR, sizeof( wchar_t ) * MAX_PATH );

					if ( certificate_pkcs_file_name != NULL )
					{
						_wcsncpy_s( file_name, MAX_PATH, certificate_pkcs_file_name, MAX_PATH );
						file_name[ MAX_PATH - 1 ] = 0;	// Sanity.
					}

					OPENFILENAME ofn;
					_memzero( &ofn, sizeof( OPENFILENAME ) );
					ofn.lStructSize = sizeof( OPENFILENAME );
					ofn.hwndOwner = hWnd;
					ofn.lpstrFilter = L"Personal Information Exchange (*.pfx;*.p12)\0*.pfx;*.p12\0All Files (*.*)\0*.*\0";
					ofn.lpstrTitle = ST_Load_PKCS_NUM12_File;
					ofn.lpstrFile = file_name;
					ofn.nMaxFile = MAX_PATH;
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_READONLY;

					if ( _GetOpenFileNameW( &ofn ) )
					{
						if ( certificate_pkcs_file_name != NULL )
						{
							GlobalFree( certificate_pkcs_file_name );
						}

						certificate_pkcs_file_name = file_name;

						_SendMessageW( g_hWnd_certificate_pkcs_location, WM_SETTEXT, 0, ( LPARAM )certificate_pkcs_file_name );

						if ( state_changed != NULL )
						{
							*state_changed = true;
							_EnableWindow( *g_hWnd_apply, TRUE );
						}
					}
					else
					{
						GlobalFree( file_name );
					}
				}
				break;

				case BTN_CERTIFICATE_CER:
				{
					wchar_t *file_name = ( wchar_t * )GlobalAlloc( GPTR, sizeof( wchar_t ) * MAX_PATH );

					if ( certificate_cer_file_name != NULL )
					{
						_wcsncpy_s( file_name, MAX_PATH, certificate_cer_file_name, MAX_PATH );
						file_name[ MAX_PATH - 1 ] = 0;	// Sanity.
					}

					OPENFILENAME ofn;
					_memzero( &ofn, sizeof( OPENFILENAME ) );
					ofn.lStructSize = sizeof( OPENFILENAME );
					ofn.hwndOwner = hWnd;
					ofn.lpstrFilter = L"X.509 Certificate (*.cer;*.crt)\0*.cer;*.crt\0All Files (*.*)\0*.*\0";
					ofn.lpstrTitle = ST_Load_X_509_Certificate_File;
					ofn.lpstrFile = file_name;
					ofn.nMaxFile = MAX_PATH;
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_READONLY;

					if ( _GetOpenFileNameW( &ofn ) )
					{
						if ( certificate_cer_file_name != NULL )
						{
							GlobalFree( certificate_cer_file_name );
						}

						certificate_cer_file_name = file_name;

						_SendMessageW( g_hWnd_certificate_cer_location, WM_SETTEXT, 0, ( LPARAM )certificate_cer_file_name );

						if ( state_changed != NULL )
						{
							*state_changed = true;
							_EnableWindow( *g_hWnd_apply, TRUE );
						}
					}
					else
					{
						GlobalFree( file_name );
					}
				}
				break;

				case BTN_CERTIFICATE_KEY:
				{
					wchar_t *file_name = ( wchar_t * )GlobalAlloc( GPTR, sizeof( wchar_t ) * MAX_PATH );

					if ( certificate_key_file_name != NULL )
					{
						_wcsncpy_s( file_name, MAX_PATH, certificate_key_file_name, MAX_PATH );
						file_name[ MAX_PATH - 1 ] = 0;	// Sanity.
					}

					OPENFILENAME ofn;
					_memzero( &ofn, sizeof( OPENFILENAME ) );
					ofn.lStructSize = sizeof( OPENFILENAME );
					ofn.hwndOwner = hWnd;
					ofn.lpstrFilter = L"Private Key (*.key)\0*.key\0All Files (*.*)\0*.*\0";
					ofn.lpstrTitle = ST_Load_Private_Key_File;
					ofn.lpstrFile = file_name;
					ofn.nMaxFile = MAX_PATH;
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_READONLY;

					if ( _GetOpenFileNameW( &ofn ) )
					{
						if ( certificate_key_file_name != NULL )
						{
							GlobalFree( certificate_key_file_name );
						}

						certificate_key_file_name = file_name;

						_SendMessageW( g_hWnd_certificate_key_location, WM_SETTEXT, 0, ( LPARAM )certificate_key_file_name );

						if ( state_changed != NULL )
						{
							*state_changed = true;
							_EnableWindow( *g_hWnd_apply, TRUE );
						}
					}
					else
					{
						GlobalFree( file_name );
					}
				}
				break;

				case CB_SSL_VERSION:
				{
					if ( HIWORD( wParam ) == CBN_SELCHANGE )
					{
						if ( state_changed != NULL )
						{
							*state_changed = true;
							_EnableWindow( *g_hWnd_apply, TRUE );
						}
					}
				}
				break;

				case BTN_KEEP_ALIVE:
				case BTN_VERIFY_ORIGIN:
				case BTN_AUTO_START:
				{
					if ( state_changed != NULL )
					{
						*state_changed = true;
						_EnableWindow( *g_hWnd_apply, TRUE );
					}
				}
				break;
			}

			return 0;
		}
		break;

		case WM_DESTROY:
		{
			GlobalFree( certificate_pkcs_file_name );
			certificate_pkcs_file_name = NULL;

			GlobalFree( certificate_cer_file_name );
			certificate_cer_file_name = NULL;

			GlobalFree( certificate_key_file_name );
			certificate_key_file_name = NULL;

			GlobalFree( t_document_root_directory );
			t_document_root_directory = NULL;

			_DeleteObject( hFont_copy );
			hFont_copy = NULL;

			return 0;
		}
		break;

		default:
		{
			return _DefWindowProcW( hWnd, msg, wParam, lParam );
		}
		break;
	}

	return TRUE;
}
