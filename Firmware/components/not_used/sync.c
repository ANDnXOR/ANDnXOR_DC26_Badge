/*****************************************************************************
 * Made with beer and late nights in California.
 *
 * (C) Copyright 2017-2018 AND!XOR LLC (http://andnxor.com/).
 *
 * PROPRIETARY AND CONFIDENTIAL UNTIL AUGUST 7th, 2018 then,
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ADDITIONALLY:
 * If you find this source code useful in anyway, use it in another electronic
 * conference badge, or just think it's neat. Consider buying us a beer
 * (or two) and/or a badge (or two). We are just as obsessed with collecting
 * badges as we are in making them.
 *
 * Contributors:
 * 	@andnxor
 * 	@zappbrandnxor
 * 	@hyr0n1
 * 	@exc3ls1or
 * 	@lacosteaef
 * 	@bitstr3m
 *****************************************************************************/
#include "system.h"

const static char *TAG = "MRMEESEEKS::Sync";

const static char *manifest = NULL;
static uint32_t manifest_offset = 0;

typedef struct {
	char url[256];
	char path[256];
} sync_file_t;

static int __sync_file_callback(request_t *req, char *data, int len) {
	sync_file_t *p_sf = (sync_file_t *) req->context;
	FILE *file = fopen(p_sf->path, "w");
	if (file != NULL) {
		fwrite(data, 1, len, file);
		fclose(file);
	} else {
		ESP_LOGE(TAG, "Could not open %s for writing.", p_sf->path);
	}
	return 0;
}

static void __download_file(sync_file_t *p_sf) {
	ESP_LOGI(TAG, "Downloading file %s to %s", p_sf->url, p_sf->path);

	//Delete file if it exists
	if (util_file_exists(p_sf->path)) {
		unlink(p_sf->path);
	}

	request_t *req;
	req = req_new(p_sf->url);
	req_setopt(req, REQ_FUNC_DOWNLOAD_CB, __sync_file_callback);
	req_setopt(req, REQ_SET_HEADER, "User-Agent: "USER_AGENT);
	req->context = p_sf;

	util_heap_stats_dump();
	req_perform(req);
	req_clean(req);
}

static void __sync() {
	char hash[SHA256_DIGEST_SIZE_BYTES + 1];

	cJSON_Hooks hooks;
	hooks.free_fn = free;
	hooks.malloc_fn = util_heap_alloc_ext;

	//Parse the manifest
	cJSON_InitHooks(&hooks);
	cJSON *root = cJSON_Parse(manifest);
	int count = cJSON_GetArraySize(root);

	util_heap_stats_dump();
	for (int i = 0; i < count; i++) {
		cJSON *file = cJSON_GetArrayItem(root, i);
		if (cJSON_GetArraySize(file) == 2) {
			char *url = cJSON_GetArrayItem(file, 0)->valuestring;
			char *sha256 = cJSON_GetArrayItem(file, 1)->valuestring;
			ESP_LOGD(TAG, "URL: %s SHA256: %s", url, sha256);

			//Build local path and continue sync process
			if (strlen(url) > strlen(SYNC_BASE_URL)) {
				char path[256];
				sprintf(path, "%s%s", SYNC_LOCAL_PATH, url + strlen(SYNC_BASE_URL));
//				ESP_LOGD(TAG, "Local Path: %s", path);

				bool sync_file = true;
				if (util_file_exists(path)) {
					util_file_sha256(path, hash);
					ESP_LOGD(TAG, "Existing file hash: %s, manifest hash: %s", hash, sha256);
					sync_file = (strcmp(hash, sha256) != 0);
				}
				//File does not exist, ensure its directories do too
				else {
					char temp_path[256];
					memcpy(temp_path, path, 256);
					*strrchr(temp_path, '/') = 0;
//					ESP_LOGD(TAG, "creating path recursively %s for %s", temp_path, path);
					util_mkdir_p(temp_path);
				}

				if (sync_file) {
					ESP_LOGD(TAG, "Need to sync %s (%s)", url, path);
					sync_file_t sf;
					memset(sf.url, 0, 256);
					memset(sf.path, 0, 256);
					memcpy(sf.url, url, MIN(strlen(url), 256));
					memcpy(sf.path, path, MIN(strlen(path), 256));

					util_heap_stats_dump();
					__download_file(&sf);
				} else {
					ESP_LOGD(TAG, "%s is up-to-date.", path);
				}
			}
		}
	}

	util_heap_stats_dump();
	cJSON_free(root);
	ESP_LOGD(TAG, "DONE JSON");
	util_heap_stats_dump();
}

int sync_callback(request_t *req, char *data, int len) {

	req_list_t *found = req->response->header;
	while (found->next != NULL) {
		found = found->next;
		ESP_LOGD(TAG, "Response header %s:%s", (char* )found->key, (char* )found->value);
	}
	//or
	found = req_list_get_key(req->response->header, "Content-Length");
	if (found) {
		ESP_LOGD(TAG, "Get header %s:%s", (char* )found->key, (char* )found->value);
	}

	//Store in local buffer
	memcpy(manifest + manifest_offset, data, len);
	manifest_offset += len;

	return 0;
}

void sync_start() {
	TaskHandle_t handle;
	xTaskCreatePinnedToCore(&sync_task, "sync_task", 8192, NULL, TASK_PRIORITY_MEDIUM, &handle, 1);
}

void sync_task(void *pvParameters) {
	ESP_LOGD(TAG, "Sync task started, waiting for connect");

	wifi_config_t wifi_config = {
		.sta = {
			.ssid = CONFIG_WIFI_SSID,
			.password = CONFIG_WIFI_PASSWORD,
		},
	};

	ESP_LOGD(TAG, "Connecting to '%s'", wifi_config.sta.ssid);
	wifi_start(wifi_config);

	request_t *req;
	if (wifi_is_connected()) {
		ESP_LOGD(TAG, "Connected to AP");
		util_heap_stats_dump();

		manifest = util_heap_alloc_ext(100000);
		memset(manifest, 0, 100000);
		manifest_offset = 0;

		req = req_new(SYNC_MANIFEST_URL);
		req_setopt(req, REQ_FUNC_DOWNLOAD_CB, sync_callback);
		req_setopt(req, REQ_SET_HEADER, "User-Agent: "USER_AGENT);

		util_heap_stats_dump();
		req_perform(req);
		util_heap_stats_dump();
		req_clean(req);

		//Now that we have the manifest, do the sync
		__sync();

		free(manifest);

		wifi_stop();
	} else {
		ESP_LOGE(TAG, "Unable to connect to '%s'", wifi_config.sta.ssid);
	}
	vTaskDelete(NULL);
}
