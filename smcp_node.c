/*
 *  smcp_node.c
 *  SMCP
 *
 *  Created by Robert Quattlebaum on 8/31/10.
 *  Copyright 2010 deepdarc. All rights reserved.
 *
 */

#ifndef VERBOSE_DEBUG
#define VERBOSE_DEBUG 0
#endif

#include "assert_macros.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "smcp.h"
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include "ll.h"

/*
   #include <poll.h>
   #include <net/if.h>
   #include <sys/socket.h>
   #include <sys/sysctl.h>
   #include <sys/errno.h>
   #include <netdb.h>
 */

#include "smcp_node.h"
#include "smcp_helpers.h"
#include "smcp_logging.h"

#pragma mark -
#pragma mark Macros


#pragma mark -
#pragma mark Node Funcs

smcp_node_t
smcp_node_alloc() {
	return (smcp_node_t)calloc(sizeof(struct smcp_node_s), 1);
}

void
smcp_node_dealloc(smcp_node_t x) {
	free(x);
}


static bt_compare_result_t
smcp_node_compare(
	smcp_node_t lhs, smcp_node_t rhs
) {
	if(lhs->name == rhs->name)
		return 0;
	if(!lhs->name)
		return 1;
	if(!rhs->name)
		return -1;
	return strcmp(lhs->name, rhs->name);
}

static bt_compare_result_t
smcp_node_compare_cstr(
	smcp_node_t lhs, const char* rhs
) {
	if(lhs->name == rhs)
		return 0;
	if(!lhs->name)
		return 1;
	if(!rhs)
		return -1;
	return strcmp(lhs->name, rhs);
}

static bt_compare_result_t
smcp_node_ncompare_cstr(
	smcp_node_t lhs, const char* rhs, int*len
) {
	if(lhs->name == rhs)
		return 0;
	if(!lhs->name)
		return 1;
	if(!rhs)
		return -1;

	bt_compare_result_t ret = strncmp(lhs->name, rhs, *len);

	if(ret == 0) {
		int lhs_len = strlen(lhs->name);
		if(lhs_len > *len)
			ret = 1;
		else if(lhs_len < *len)
			ret = -1;
	}

	return ret;
}

smcp_device_node_t
smcp_node_add_subdevice(
	smcp_node_t node, const char* name
) {
	smcp_device_node_t ret = NULL;

	require(!node || (node->type == SMCP_NODE_DEVICE), bail);

	ret = (smcp_device_node_t)smcp_node_alloc();

	require(ret, bail);

	if(node) {
		require(name, bail);
		ret->name = name;
		bt_insert(
			    (void**)&((smcp_device_node_t)node)->subdevices,
			ret,
			    (bt_compare_func_t)smcp_node_compare,
			    (bt_delete_func_t)smcp_node_delete,
			NULL
		);
		ret->parent = node;
	} else {
		if(!name)
			name = "";
	}
	ret->type = SMCP_NODE_DEVICE;

bail:
	return ret;
}

smcp_action_node_t
smcp_node_add_action(
	smcp_node_t node, const char* name
) {
	smcp_action_node_t ret = NULL;

	require(node, bail);
	require((node->type == SMCP_NODE_DEVICE) ||
		    (node->type == SMCP_NODE_VARIABLE), bail);

	ret = (smcp_action_node_t)smcp_node_alloc();

	require(ret, bail);

	ret->type = SMCP_NODE_ACTION;
	ret->name = name;

	if(node) {
		bt_insert(
			    (void**)&((smcp_device_node_t)node)->actions,
			ret,
			    (bt_compare_func_t)smcp_node_compare,
			    (bt_delete_func_t)smcp_node_delete,
			NULL
		);
		ret->parent = node;
	}

bail:
	return ret;
}

smcp_variable_node_t
smcp_node_add_variable(
	smcp_node_t node, const char* name
) {
	smcp_variable_node_t ret = NULL;

	require(node, bail);
	require((node->type == SMCP_NODE_DEVICE), bail);

	ret = (smcp_variable_node_t)smcp_node_alloc();

	require(ret, bail);

	ret->type = SMCP_NODE_VARIABLE;
	ret->name = name;

	if(node) {
		bt_insert(
			    (void**)&((smcp_device_node_t)node)->variables,
			ret,
			    (bt_compare_func_t)smcp_node_compare,
			    (bt_delete_func_t)smcp_node_delete,
			NULL
		);
		ret->parent = node;
	}

bail:
	return ret;
}

smcp_event_node_t
smcp_node_add_event(
	smcp_node_t node, const char* name
) {
	smcp_event_node_t ret = NULL;

	require(node, bail);
	require((node->type == SMCP_NODE_DEVICE) ||
		    (node->type == SMCP_NODE_VARIABLE), bail);

	ret = (smcp_event_node_t)smcp_node_alloc();

	require(ret, bail);

	ret->type = SMCP_NODE_EVENT;
	ret->name = name;

	if(node) {
		bt_insert(
			    (void**)&((smcp_device_node_t)node)->events,
			ret,
			    (bt_compare_func_t)smcp_node_compare,
			    (bt_delete_func_t)smcp_node_delete,
			NULL
		);
		ret->parent = node;
	}

bail:
	return ret;
}

void
smcp_node_delete(smcp_node_t node) {
	void** owner = NULL;

	switch(node->type) {
	case SMCP_NODE_DEVICE:
		while(((smcp_device_node_t)node)->actions)
			smcp_node_delete(((smcp_device_node_t)node)->actions);
		while(((smcp_device_node_t)node)->events)
			smcp_node_delete(((smcp_device_node_t)node)->events);
		while(((smcp_device_node_t)node)->subdevices)
			smcp_node_delete(((smcp_device_node_t)node)->subdevices);
		while(((smcp_device_node_t)node)->variables)
			smcp_node_delete(((smcp_device_node_t)node)->variables);
		if(node->parent)
			owner =
			    (void**)&((smcp_device_node_t)node->parent)->subdevices;
		break;
	case SMCP_NODE_VARIABLE:
		while(((smcp_device_node_t)node)->actions)
			smcp_node_delete(((smcp_device_node_t)node)->actions);
		while(((smcp_device_node_t)node)->events)
			smcp_node_delete(((smcp_device_node_t)node)->events);
		if(node->parent)
			owner = (void**)&((smcp_device_node_t)node->parent)->variables;
		break;
	case SMCP_NODE_EVENT:
		if(node->parent)
			owner = (void**)&((smcp_device_node_t)node->parent)->events;
		break;
	case SMCP_NODE_ACTION:
		if(node->parent)
			owner = (void**)&((smcp_device_node_t)node->parent)->actions;
		break;
	}

	if(owner)
		bt_remove(owner, node, (bt_compare_func_t)smcp_node_compare,
			    (bt_delete_func_t)smcp_node_dealloc, NULL);
	else
		smcp_node_dealloc(node);
bail:
	return;
}

smcp_status_t
smcp_node_get_path(
	smcp_node_t node, char* path, size_t max_path_len
) {
	smcp_status_t ret = 0;

	require(node, bail);
	require(path, bail);

	if(node->parent) {
		// using recursion here just makes this code so much more pretty,
		// but it would be ideal to avoid using recursion at all,
		// to be nice to the stack. Just a topic of future investigation...
		ret = smcp_node_get_path(node->parent, path, max_path_len);
	} else {
		path[0] = 0;
	}

	if(node->type == SMCP_NODE_ACTION)
		strlcat(path, "?", max_path_len);
	else if(node->type == SMCP_NODE_EVENT)
		strlcat(path, "!", max_path_len);

	if(node->name)
		strlcat(path, node->name, max_path_len);

	if(node->type == SMCP_NODE_DEVICE)
		strlcat(path, "/", max_path_len);

bail:
	return ret;
}

extern int smcp_node_find_next_with_path(
	smcp_node_t node, const char* orig_path, smcp_node_t* next
) {
	const char* path = orig_path;

	require(next, bail);
	require(node, bail);
	require(path, bail);

	// Move past any preceding slashes.
	while(path[0] == '/')
		path++;

	if(path[0] == 0) {
		// Self.
		*next = node;
	} else if(path[0] == '?') {
		// Action.
		require((node->type == SMCP_NODE_DEVICE) ||
			    (node->type == SMCP_NODE_VARIABLE), bail);
		path++; // Move past question mark
		*next =
		    bt_find((void**)&((smcp_device_node_t)node)->actions,
			path,
			    (bt_compare_func_t)smcp_node_compare_cstr,
			NULL);
	} else if(path[0] == '!') {
		// Event.
		require((node->type == SMCP_NODE_DEVICE) ||
			    (node->type == SMCP_NODE_VARIABLE), bail);

		path++; // Move past exclamation point
		*next = bt_find(
			    (void**)&((smcp_device_node_t)node)->events,
			path,
			    (bt_compare_func_t)smcp_node_compare_cstr,
			NULL
		    );
	} else {
		// Device or Variable.
		int namelen;
		for(namelen = 0; path[namelen]; namelen++) {
			if((path[namelen] == '/') || (path[namelen] == '?') ||
			        (path[namelen] == '!'))
				break;
		}

		*next = bt_find(
			    (void**)&((smcp_device_node_t)node)->subdevices,
			path,
			    (bt_compare_func_t)smcp_node_ncompare_cstr,
			&namelen
		    );

		if(!*next) {
			// Wasn't a device, must be a variable.
			*next = bt_find(
				    (void**)&((smcp_device_node_t)node)->variables,
				path,
				    (bt_compare_func_t)smcp_node_ncompare_cstr,
				&namelen
			    );
		}

		if(!*next) {
			DEBUG_PRINTF(CSTR(
					"Unable to find node. node->name=%s, path=%s, namelen=%d"),
				node->name, path, namelen);
			goto bail;
		}
	}

	if(!*next) {
		DEBUG_PRINTF(CSTR(
				"Unable to find node. node->name=%s, path=%s"), node->name,
			path);
		goto bail;
	}

	// Move to next name
	while(path[0] && (path[0] != '/') && (path[0] != '!') &&
	        (path[0] != '?'))
		path++;

	// Move past any preceding slashes.
	while(path[0] == '/')
		path++;

bail:
	return path - orig_path;
}

smcp_node_t
smcp_node_find_with_path(
	smcp_node_t node, const char* path
) {
	smcp_node_t ret = NULL;

again:
	require(node, bail);
	require(path, bail);

	do {
		const char* nextPath = path;
		nextPath += smcp_node_find_next_with_path(node, path, &ret);
		node = ret;
		DEBUG_PRINTF(CSTR("%s: %p (nextPath = %s)"), path, node, nextPath);
		path = nextPath;
	} while(ret && path[0]);

bail:
	return ret;
}

extern int smcp_node_find_closest_with_path(
	smcp_node_t node, const char* path, smcp_node_t* closest
) {
	int ret = 0;

again:
	require(node, bail);
	require(path, bail);

	do {
		*closest = node;
		ret += smcp_node_find_next_with_path(*closest, path + ret, &node);
	} while(node && path[0]);

bail:
	return ret;
}