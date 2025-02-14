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

#ifndef _SHARED_OBJECTS_H
#define _SHARED_OBJECTS_H

struct contactinfo;

struct ringtoneinfo
{
	wchar_t *ringtone_path;
	wchar_t *ringtone_file;
};

struct callerinfo
{
	char *call_to;
	char *call_from;
	char *caller_id;
	char *forward_to;

	char *call_reference_id;

	bool ignored;			// The phone number has been ignored
	bool forwarded;			// The phone number has been forwarded
};

struct displayinfo
{
	union
	{
		struct	// Keep these in alphabetical order.
		{
			wchar_t *caller_id;					// Caller ID
			wchar_t *w_time;					// Date and Time
			wchar_t *w_forward_caller_id;		// Forward Caller ID
			wchar_t *w_forward_phone_number;	// Forward Phone Number
			wchar_t *forward_to;				// Forward to
			wchar_t *w_ignore_caller_id;		// Ignore Caller ID
			wchar_t *w_ignore_phone_number;		// Ignore Phone Number
			wchar_t *phone_number;				// Phone Number
			wchar_t *reference;					// Reference
			wchar_t *sent_to;					// Sent to
		};

		wchar_t *display_values[ 10 ];
	};

	callerinfo ci;

	ULARGE_INTEGER time;

	contactinfo *contact_info;

	unsigned int forward_cid_match_count;	// Number of forward cid matches.
	unsigned int ignore_cid_match_count;	// Number of ignore cid matches.

	bool ignore_phone_number;		// phone number is in ignore_list
	bool forward_phone_number;		// phone number is in forward_list

	bool process_incoming;	// false, true = ignore or forward
};

struct CONTACT
{
	union
	{
		struct	// Keep these in alphabetical order.
		{
			char *cell_phone_number;	// Cell Phone Number
			char *business_name;		// Company
			char *department;			// Department
			char *email_address;		// Email Address
			char *fax_number;			// Fax Number
			char *first_name;			// First Name
			char *home_phone_number;	// Home Phone Number
			char *designation;			// Job Title
			char *last_name;			// Last Name
			char *nickname;				// Nickname
			char *office_phone_number;	// Office Phone Number
			char *other_phone_number;	// Other Phone Number
			char *category;				// Profession
			char *title;				// Title
			char *web_page;				// Web Page
			char *work_phone_number;	// Work Phone Number
		};

		char *contact_values[ 16 ];
	};

	char *ringtone;

	char *cell_phone_number_id;
	char *contact_entry_id;
	char *email_address_id;
	char *fax_number_id;
	char *home_phone_number_id;
	char *office_phone_number_id;
	char *other_phone_number_id;
	char *web_page_id;
	char *work_phone_number_id;

	char *picture_location;		// Picture URL. For downloading.
};

struct contactinfo
{
	CONTACT contact;

	union
	{
		struct	// Keep these in alphabetical order.
		{
			wchar_t *cell_phone_number;		// Cell Phone Number
			wchar_t *business_name;			// Company
			wchar_t *department;			// Department
			wchar_t *email_address;			// Email Address
			wchar_t *fax_number;			// Fax Number
			wchar_t *first_name;			// First Name
			wchar_t *home_phone_number;		// Home Phone Number
			wchar_t *designation;			// Job Title
			wchar_t *last_name;				// Last Name
			wchar_t *nickname;				// Nickname
			wchar_t *office_phone_number;	// Office Phone Number
			wchar_t *other_phone_number;	// Other Phone Number
			wchar_t *category;				// Profession
			wchar_t *title;					// Title
			wchar_t *web_page;				// Web Page
			wchar_t *work_phone_number;		// Work Phone Number
		};

		wchar_t *contactinfo_values[ 16 ];
	};

	wchar_t *picture_path;			// Local path to picture.
	ringtoneinfo *ringtone_info;	// Local path and file name of the ringtone.

	bool displayed;					// Set to true if it's displayed in the contact list.
};

struct ignoreinfo
{
	union
	{
		struct	// Keep these in alphabetical order.
		{
			char *c_phone_number;
			char *c_total_calls;
		};

		char *c_ignoreinfo_values[ 2 ];
	};

	union
	{
		struct	// Keep these in alphabetical order.
		{
			wchar_t *phone_number;
			wchar_t *total_calls;
		};

		wchar_t *ignoreinfo_values[ 2 ];
	};

	unsigned int count;		// Number of times the call has been ignored.
	unsigned char state;	// 0 = keep, 1 = remove.
};

struct forwardinfo
{
	union
	{
		struct	// Keep these in alphabetical order.
		{
			char *c_forward_to;		// Forward to
			char *c_call_from;		// Phone Number
			char *c_total_calls;	// Total Calls
		};

		char *c_forwardinfo_value[ 3 ];
	};

	union
	{
		struct	// Keep these in alphabetical order.
		{
			wchar_t *forward_to;	// Forward to
			wchar_t *call_from;		// Phone Number
			wchar_t *total_calls;	// Total Calls
		};

		wchar_t *forwardinfo_values[ 3 ];
	};

	unsigned int count;		// Number of times the call has been forwarded.
	unsigned char state;	// 0 = keep, 1 = remove.
};

struct ignorecidinfo
{
	union
	{
		struct	// Keep these in alphabetical order.
		{
			char *c_caller_id;
			char *c_match_case;
			char *c_match_whole_word;
			char *c_total_calls;
		};

		char *c_ignorecidinfo_values[ 4 ];
	};

	union
	{
		struct	// Keep these in alphabetical order.
		{
			wchar_t *caller_id;
			wchar_t *w_match_case;
			wchar_t *w_match_whole_word;
			wchar_t *total_calls;
		};

		wchar_t *ignorecidinfo_values[ 4 ];
	};

	unsigned int count;		// Number of times the call has been ignored.
	unsigned char state;	// 0 = keep, 1 = remove.

	bool match_case;
	bool match_whole_word;

	bool active;			// A caller ID value has been matched.
};

struct forwardcidinfo
{
	union
	{
		struct	// Keep these in alphabetical order.
		{
			char *c_caller_id;
			char *c_forward_to;
			char *c_match_case;
			char *c_match_whole_word;
			char *c_total_calls;
		};

		char *c_forwardcidinfo_values[ 5 ];
	};

	union
	{
		struct	// Keep these in alphabetical order.
		{
			wchar_t *caller_id;
			wchar_t *forward_to;
			wchar_t *w_match_case;
			wchar_t *w_match_whole_word;
			wchar_t *total_calls;
		};

		wchar_t *forwardcidinfo_values[ 5 ];
	};

	unsigned int count;		// Number of times the call has been forwarded.
	unsigned char state;	// 0 = keep, 1 = remove.

	bool match_case;
	bool match_whole_word;

	bool active;			// A caller ID value has been matched.
};

#endif
