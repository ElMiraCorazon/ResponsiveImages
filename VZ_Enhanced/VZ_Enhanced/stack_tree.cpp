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

#include "stack_tree.h"

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

StackTree *ST_CreateNode( StackTree data )
{
	StackTree *dll = ( StackTree * )GlobalAlloc( GMEM_FIXED, sizeof( StackTree ) );
	dll->parent = NULL;
	dll->sibling = NULL;
	dll->element_name = data.element_name;
	dll->element_name_length = data.element_name_length;
	dll->element_length = data.element_length;
	dll->child = NULL;
	dll->closing_element = NULL;

	return dll;
}

void ST_PushNode( StackTree **head, StackTree *node )
{
	if ( node == NULL )
	{
		return;
	}

	if ( *head == NULL )
	{
		*head = node;
		return;
	}

	node->sibling = *head;
	*head = node;
}

StackTree *ST_PopNode( StackTree **head )
{
	if ( *head == NULL )
	{
		return NULL;
	}

	StackTree *node = *head;
	*head = ( *head )->sibling;
	return node;
}
