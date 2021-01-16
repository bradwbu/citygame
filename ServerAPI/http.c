#include "http.h"
#include "microhttpd.h"
#include "json.h"
#include "earray.h"
#include "ServerAPI.h"
#include "serverCmdStats.h"
#include "serverMonitorNet.h"
#include "serverMonitorNet.h"

#define INVALID_REQUEST_PAGE "<html><head><title>Invalid Request</title></head><body><h1>Invalid request</h1></body></html>"
static struct MHD_Response *invalid_request_response = 0;
#define INTERNAL_ERROR_PAGE "<html><head><title>Internal Server Error</title></head><body><h1>Internal Server Error</h1></body></html>"
static struct MHD_Response *internal_error_response = 0;

static int invalidRequest(struct MHD_Connection *conn)
{
	return MHD_queue_response(conn, MHD_HTTP_BAD_REQUEST, invalid_request_response);
}

static int internalError(struct MHD_Connection *conn)
{
	return MHD_queue_response(conn, MHD_HTTP_INTERNAL_SERVER_ERROR, internal_error_response);
}

static int sendJson(struct MHD_Connection *conn, JsonNode *json)
{
	struct MHD_Response *resp;
	char *jsonstr;
	int ret;

	jsonstr = jsonEStr(json);
	if (!jsonstr) {
		return internalError(conn);
	}

	resp = MHD_create_response_from_buffer(estrLength(&jsonstr), jsonstr, MHD_RESPMEM_MUST_COPY);
	estrDestroy(&jsonstr);

	MHD_add_response_header(resp, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
	ret = MHD_queue_response(conn, 200, resp);
	MHD_destroy_response(resp);

	return ret;
}

static JsonNode *statusOne(ServerAPIShard *shard)
{
	ServerMonitorState *state = shard->state;
	JsonNode *jsonshard, *jsonstatus;

	jsonshard = jsonNode(shard->name, NULL, false, false);
	jsonstatus = jsonNode("status", svrMonAlive(state) ? "up" : "down", true, false);
	eaPush(&jsonshard->children, jsonstatus);

	return jsonshard;
}

static JsonNode *statsOne(ServerAPIShard *shard, void (*statsFunc)(ServerMonitorState*, JsonNode*))
{
	ServerMonitorState *state = shard->state;
	JsonNode *jsonshard = statusOne(shard);

	if (svrMonAlive(state)) {
		EnterCriticalSection(&state->stats_lock);
		statsFunc(state, jsonshard);
		LeaveCriticalSection(&state->stats_lock);
	}

	return jsonshard;
}

static JsonNode *allstatsOne(ServerAPIShard *shard)
{
	ServerMonitorState *state = shard->state;
	JsonNode *jsonshard = statusOne(shard);

	if (svrMonAlive(state)) {
		EnterCriticalSection(&state->stats_lock);
		serverCmdDbStats(state, jsonshard);
		serverCmdLauncherStats(state, jsonshard);
		serverCmdMapStats(state, jsonshard);
		LeaveCriticalSection(&state->stats_lock);
	}

	return jsonshard;
}

static int sendStatsOne(struct MHD_Connection *conn, ServerAPIShard *shard, void (*statsFunc)(ServerMonitorState*, JsonNode*))
{
	int ret = MHD_NO;
	JsonNode *json;

	json = jsonNode(NULL, NULL, false, false);
	eaPush(&json->children, statsOne(shard, statsFunc));
	ret = sendJson(conn, json);
	jsonDestroy(json);
	return ret;
}

static int sendAllStatsOne(struct MHD_Connection *conn, ServerAPIShard *shard)
{
	int ret = MHD_NO;
	JsonNode *json;

	json = jsonNode(NULL, NULL, false, false);
	eaPush(&json->children, allstatsOne(shard));
	ret = sendJson(conn, json);
	jsonDestroy(json);
	return ret;
}

static int sendStatsAll(struct MHD_Connection *conn, ServerAPIConfig *config, void (*statsFunc)(ServerMonitorState*, JsonNode*))
{
	int ret = MHD_NO;
	JsonNode *json;
	int i;

	json = jsonNode(NULL, NULL, false, false);
	for (i = 0; i < eaSize(&config->shards); i++) {
		ServerAPIShard *shard = config->shards[i];
		eaPush(&json->children, statsOne(shard, statsFunc));
	}
	ret = sendJson(conn, json);
	jsonDestroy(json);
	return ret;
}

static int sendAllStatsAll(struct MHD_Connection *conn, ServerAPIConfig *config)
{
	int ret = MHD_NO;
	JsonNode *json;
	int i;

	json = jsonNode(NULL, NULL, false, false);
	for (i = 0; i < eaSize(&config->shards); i++) {
		ServerAPIShard *shard = config->shards[i];
		eaPush(&json->children, allstatsOne(shard));
	}
	ret = sendJson(conn, json);
	jsonDestroy(json);
	return ret;
}

static int sendStatusOne(struct MHD_Connection *conn, ServerAPIShard *shard)
{
	int ret = MHD_NO;
	JsonNode *json;

	json = jsonNode(NULL, NULL, false, false);
	eaPush(&json->children, statusOne(shard));
	ret = sendJson(conn, json);
	jsonDestroy(json);
	return ret;
}

static int sendStatusAll(struct MHD_Connection *conn, ServerAPIConfig *config)
{
	int ret = MHD_NO;
	JsonNode *json;
	int i;

	json = jsonNode(NULL, NULL, false, false);
	for (i = 0; i < eaSize(&config->shards); i++) {
		ServerAPIShard *shard = config->shards[i];
		eaPush(&json->children, statusOne(shard));
	}
	ret = sendJson(conn, json);
	jsonDestroy(json);
	return ret;
}

static int dispatchGetRequest(ServerAPIConfig *config,
						   struct MHD_Connection *conn,
						   const char *shardname,
						   const char *action)
{
	ServerAPIShard *shard;

	if (!stricmp(shardname, "shards"))
		return sendStatusAll(conn, config);

	if (!action)
		return invalidRequest(conn);

	if (!stricmp(shardname, "all")) {
		if (!stricmp(action, "dbserver"))
			return sendStatsAll(conn, config, serverCmdDbStats);
		else if (!stricmp(action, "launchers"))
			return sendStatsAll(conn, config, serverCmdLauncherStats);
		else if (!stricmp(action, "maps"))
			return sendStatsAll(conn, config, serverCmdMapStats);
		else if (!stricmp(action, "status"))
			return sendStatusAll(conn, config);
		else if (!stricmp(action, "allstats"))
			return sendAllStatsAll(conn, config);
	}

	if (shardname[0] && stashFindPointer(config->shardidx, shardname, &shard)) {
		if (!stricmp(action, "dbserver"))
			return sendStatsOne(conn, shard, serverCmdDbStats);
		else if (!stricmp(action, "launchers"))
			return sendStatsOne(conn, shard, serverCmdLauncherStats);
		else if (!stricmp(action, "maps"))
			return sendStatsOne(conn, shard, serverCmdMapStats);
		else if (!stricmp(action, "status"))
			return sendStatusOne(conn, shard);
		else if (!stricmp(action, "allstats"))
			return sendAllStatsOne(conn, shard);
	}

	return invalidRequest(conn);
}

static int httpRequest(void *cls,
						struct MHD_Connection *conn,
						const char *url,
						const char *method,
						const char *version,
						const char *upload_data,
						size_t *upload_data_size,
						void **ptr)
{
	ServerAPIConfig *config = (ServerAPIConfig*)cls;
	char *urlcopy = _strdup(url);
	char *shardname = urlcopy;
	char *action;
	int ret = MHD_NO;

	EXCEPTION_HANDLER_BEGIN

	if (shardname[0] == '/')
		shardname++;
	action = strtok(shardname, "/");
	action = strtok(NULL, "/");

	if (!strcmp(method, "GET")) {
		return dispatchGetRequest(config, conn, shardname, action);
	} else {
		ret = invalidRequest(conn);
	}

	free(urlcopy);
	EXCEPTION_HANDLER_END
	return ret;
}

void startHttp(ServerAPIConfig *config)
{
	stopHttp(config);

	if (!invalid_request_response) {
		invalid_request_response = MHD_create_response_from_buffer(strlen(INVALID_REQUEST_PAGE),
			(void*)INVALID_REQUEST_PAGE, MHD_RESPMEM_PERSISTENT);
		MHD_add_response_header(invalid_request_response, MHD_HTTP_HEADER_CONTENT_TYPE,
			"text/html");
	}
	if (!internal_error_response) {
		internal_error_response = MHD_create_response_from_buffer(strlen(INVALID_REQUEST_PAGE),
			(void*)INVALID_REQUEST_PAGE, MHD_RESPMEM_PERSISTENT);
		MHD_add_response_header(internal_error_response, MHD_HTTP_HEADER_CONTENT_TYPE,
			"text/html");
	}

	config->httpserver = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD | MHD_ALLOW_SUSPEND_RESUME,
		(uint16_t)config->port, NULL, NULL,
		httpRequest, config,
		MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int) (120 /* seconds */),
		MHD_OPTION_END);
}

void stopHttp(ServerAPIConfig *config)
{
	if (config->httpserver) {
		MHD_stop_daemon(config->httpserver);
		config->httpserver = NULL;
	}
}
