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
#include <stdlib.h>
#include "smcp.h"
#include <string.h>
#include <ctype.h>
#include "ll.h"
#include "url-helpers.h"

#include "smcp.h"
#include "smcp_node.h"
#include "smcp_helpers.h"
#include "smcp_logging.h"

#pragma mark -
#pragma mark Macros


#pragma mark -
#pragma mark Node Funcs

void
smcp_node_dealloc(smcp_node_t x) {
	free(x);
}

smcp_node_t
smcp_node_alloc() {
	smcp_node_t ret = (smcp_node_t)calloc(sizeof(struct smcp_node_s), 1);

	ret->finalize = &smcp_node_dealloc;
	return ret;
}

bt_compare_result_t
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
smcp_node_ncompare_cstr(
	smcp_node_t lhs, const char* rhs, int*len
) {
	bt_compare_result_t ret;

	if(lhs->name == rhs)
		return 0;
	if(!lhs->name)
		return 1;
	if(!rhs)
		return -1;

	ret = strncmp(lhs->name, rhs, *len);

	if(ret == 0) {
		int lhs_len = strlen(lhs->name);
		if(lhs_len > *len)
			ret = 1;
		else if(lhs_len < *len)
			ret = -1;
	}

	return ret;
}

smcp_node_t
smcp_node_init(
	smcp_node_t self, smcp_node_t node, const char* name
) {
	smcp_node_t ret = NULL;

	require(self || (self = smcp_node_alloc()), bail);

	ret = (smcp_node_t)self;

	ret->request_handler = &smcp_default_request_handler;

	if(node) {
		require(name, bail);
		ret->name = name;
		bt_insert(
			    (void**)&((smcp_node_t)node)->children,
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

	// Delete all child objects.
	while(((smcp_node_t)node)->children)
		smcp_node_delete(((smcp_node_t)node)->children);

	if(node->parent)
		owner = (void**)&((smcp_node_t)node->parent)->children;

	if(owner) {
		bt_remove(owner,
			node,
			(bt_compare_func_t)smcp_node_compare,
			(void*)node->finalize,
			NULL
		);
	}

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

	strlcat(path, "/", max_path_len);

	if(node->name) {
		size_t len = strlen(path);
		if(max_path_len>len)
			url_encode_cstr(path+len, node->name, max_path_len - len);
	}

bail:
	return ret;
}

smcp_node_t
smcp_node_find(
	smcp_node_t node,
	const char* name,	// Unescaped.
	int name_len
) {
	return (smcp_node_t)bt_find(
		(void**)&((smcp_node_t)node)->children,
		name,
		(bt_compare_func_t)smcp_node_ncompare_cstr,
		&name_len
	);
}

int
smcp_node_find_next_with_path(
	smcp_node_t node,
	const char* orig_path,	// Escaped.
	smcp_node_t* next
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
	} else {
		// Device or Variable.
		int namelen;
		for(namelen = 0; path[namelen]; namelen++) {
			if((path[namelen] == '/') || (path[namelen] == '?') ||
			        (path[namelen] == '!'))
				break;
		}

		// Warning: This could be dangerous!
		// We should evaluate the liklihood of blowing
		// the stack here. TODO: Investigate potential oveflow!
		{
#if HAS_ALLOCA
			char unescaped_name[namelen+1];
#else
			char *unescaped_name = malloc(namelen+1);
#endif
			size_t escaped_len =url_decode_str(
				unescaped_name,
				namelen+1,
				path,
				namelen
			)
;
			*next = smcp_node_find(
				node,
				unescaped_name,
				escaped_len
			);
#if HAS_ALLOCA
			free(unescaped_name);
#endif
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

	*closest = node;
	do {
		ret += smcp_node_find_next_with_path(*closest, path + ret, &node);
		if(node)
			*closest = node;
	} while(node && path[ret]);

bail:
	return ret;
}

smcp_node_t
smcp_node_get_root(smcp_node_t node) {
	if(node && node->parent)
		return smcp_node_get_root(node->parent); // Recursion should be optimized away.
	return node;
}
