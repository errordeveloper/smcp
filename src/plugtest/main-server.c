/*	@file main-server.c
**	@author Robert Quattlebaum <darco@deepdarc.com>
**	@desc Plugtest Server Entrypoint
**
**	Copyright (C) 2011,2012 Robert Quattlebaum
**
**	Permission is hereby granted, free of charge, to any person
**	obtaining a copy of this software and associated
**	documentation files (the "Software"), to deal in the
**	Software without restriction, including without limitation
**	the rights to use, copy, modify, merge, publish, distribute,
**	sublicense, and/or sell copies of the Software, and to
**	permit persons to whom the Software is furnished to do so,
**	subject to the following conditions:
**
**	The above copyright notice and this permission notice shall
**	be included in all copies or substantial portions of the
**	Software.
**
**	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
**	KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
**	WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
**	PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
**	OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
**	OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
**	OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
**	SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <smcp/assert_macros.h>

#include <smcp/smcp.h>
#include <smcp/smcp-opts.h>
#include "plugtest-server.h"

int
main(int argc, char * argv[]) {
	smcp_t smcp = smcp_create(0);
	struct plugtest_server_s plugtest_server;

	fprintf(stderr,"DEBUG = %d\n",DEBUG);
	fprintf(stderr,"VERBOSE_DEBUG = %d\n",VERBOSE_DEBUG);
	fprintf(stderr,"SMCP_EMBEDDED = %d\n",SMCP_EMBEDDED);
	fprintf(stderr,"SMCP_USE_BSD_SOCKETS = %d\n",SMCP_USE_BSD_SOCKETS);
	fprintf(stderr,"SMCP_DEFAULT_PORT = %d\n",SMCP_DEFAULT_PORT);
	fprintf(stderr,"SMCP_MAX_PATH_LENGTH = %d\n",SMCP_MAX_PATH_LENGTH);
	fprintf(stderr,"SMCP_MAX_URI_LENGTH = %d\n",SMCP_MAX_URI_LENGTH);
	fprintf(stderr,"SMCP_MAX_PACKET_LENGTH = %d\n",(int)SMCP_MAX_PACKET_LENGTH);
	fprintf(stderr,"SMCP_MAX_CONTENT_LENGTH = %d\n",SMCP_MAX_CONTENT_LENGTH);
	fprintf(stderr,"SMCP_USE_CASCADE_COUNT = %d\n",SMCP_USE_CASCADE_COUNT);
	fprintf(stderr,"SMCP_MAX_CASCADE_COUNT = %d\n",SMCP_MAX_CASCADE_COUNT);
	fprintf(stderr,"SMCP_ADD_NEWLINES_TO_LIST_OUTPUT = %d\n",SMCP_ADD_NEWLINES_TO_LIST_OUTPUT);
	fprintf(stderr,"SMCP_AVOID_PRINTF = %d\n",SMCP_AVOID_PRINTF);
	fprintf(stderr,"SMCP_AVOID_MALLOC = %d\n",SMCP_AVOID_MALLOC);
	fprintf(stderr,"SMCP_CONF_USE_DNS = %d\n",SMCP_CONF_USE_DNS);
	fprintf(stderr,"SMCP_CONF_MAX_TRANSACTIONS = %d\n",SMCP_CONF_MAX_TRANSACTIONS);
	fprintf(stderr,"SMCP_CONF_MAX_ALLOCED_NODES = %d\n",SMCP_CONF_MAX_ALLOCED_NODES);
	fprintf(stderr,"SMCP_CONF_MAX_TIMEOUT = %d\n",SMCP_CONF_MAX_TIMEOUT);
	fprintf(stderr,"SMCP_CONF_DUPE_BUFFER_SIZE = %d\n",SMCP_CONF_DUPE_BUFFER_SIZE);
	fprintf(stderr,"SMCP_CONF_TIMER_NODE_INCLUDE_COUNT = %d\n",SMCP_CONF_TIMER_NODE_INCLUDE_COUNT);
	fprintf(stderr,"SMCP_ENABLE_PAIRING = %d\n",SMCP_ENABLE_PAIRING);
	fprintf(stderr,"SMCP_CONF_OBSERVING_ONLY = %d\n",SMCP_CONF_OBSERVING_ONLY);
	fprintf(stderr,"SMCP_OBSERVATION_KEEPALIVE_INTERVAL = %d\n",SMCP_OBSERVATION_KEEPALIVE_INTERVAL);
	fprintf(stderr,"SMCP_OBSERVATION_DEFAULT_MAX_AGE = %d\n",SMCP_OBSERVATION_DEFAULT_MAX_AGE);
	fprintf(stderr,"SMCP_CONF_PAIRING_STATS = %d\n",SMCP_CONF_PAIRING_STATS);
	fprintf(stderr,"SMCP_PAIRING_DEFAULT_ROOT_PATH = %s\n",SMCP_PAIRING_DEFAULT_ROOT_PATH);
	fprintf(stderr,"SMCP_VARIABLE_MAX_VALUE_LENGTH = %d\n",SMCP_VARIABLE_MAX_VALUE_LENGTH);
	fprintf(stderr,"SMCP_VARIABLE_MAX_KEY_LENGTH = %d\n",SMCP_VARIABLE_MAX_KEY_LENGTH);

	plugtest_server_init(&plugtest_server,smcp_get_root_node(smcp));

	fprintf(stderr,"\nPlugtest server listening on port %d.\n", smcp_get_port(smcp));

	while(1)
		smcp_process(smcp,1000);

	return 0;
}
