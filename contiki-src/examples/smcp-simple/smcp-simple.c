
#include "smcp-task.h"
#include "net/resolv.h"
#include <smcp/smcp.h>

PROCESS_NAME(smcp_simple);
PROCESS(smcp_simple, "SMCP Simple Demo");

/*---------------------------------------------------------------------------*/
AUTOSTART_PROCESSES(
	&resolv_process,
	&smcp_task,
	&smcp_simple,
	NULL
);

/*---------------------------------------------------------------------------*/
static smcp_status_t
request_handler(void* context) {
	/*	This will respond to every GET request to `/hello-world' with
	**	"Hello world!". Everyone else gets a 4.04 Not Found. */

	printf("Got a request!\n");

	if(smcp_inbound_get_code() != COAP_METHOD_GET) {
		return SMCP_STATUS_NOT_IMPLEMENTED;
	}

	while(smcp_inbound_peek_option(NULL, NULL) != COAP_OPTION_URI_PATH)
		if(smcp_inbound_next_option(NULL, NULL) == COAP_OPTION_INVALID)
			break;

	if(smcp_inbound_option_strequal(COAP_OPTION_URI_PATH, "hello-world")) {

		smcp_outbound_begin_response(COAP_RESULT_205_CONTENT);

		smcp_outbound_add_option_uint(
			COAP_OPTION_CONTENT_TYPE,
			COAP_CONTENT_TYPE_TEXT_PLAIN
		);

		smcp_outbound_append_content("Hello world!", SMCP_CSTR_LEN);

		return smcp_outbound_send();
	} else if(smcp_inbound_option_strequal(COAP_OPTION_URI_PATH, ".well-known")) {
		smcp_inbound_next_option(NULL, NULL);

		if(smcp_inbound_option_strequal(COAP_OPTION_URI_PATH, "core")) {
			smcp_outbound_begin_response(COAP_RESULT_205_CONTENT);

			smcp_outbound_add_option_uint(
				COAP_OPTION_CONTENT_TYPE,
				COAP_CONTENT_TYPE_APPLICATION_LINK_FORMAT
			);

			smcp_outbound_append_content("</hello-world>", SMCP_CSTR_LEN);

			return smcp_outbound_send();
		}
	}

	return SMCP_STATUS_NOT_FOUND;
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(smcp_simple, ev, data)
{
	PROCESS_BEGIN();

	smcp_set_default_request_handler(instance, &request_handler, NULL);

	PROCESS_END();
}
