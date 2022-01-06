#include <azure_c_shared_utility/xlogging.h>
#include <azure_c_shared_utility/platform.h>
#include <azure_c_shared_utility/threadapi.h>
#include <azure_c_shared_utility/crt_abstractions.h>
#include <iothub_client.h>
#include <iothub_client_options.h>
#include <iothub_message.h>
#include <iothubtransportmqtt.h>
#include <jsondecoder.h>
#include <pthread.h>

#include "azureiothub.h"
#include <stdlib.h>

static char* get_device_id(const char* str)
{
    const char* substr = strstr(str, "DeviceId=");

    if (substr == NULL)
        return NULL;

    // skip "DeviceId="
    substr += 9;

    const char* semicolon = strstr(substr, ";");
    int length = semicolon == NULL ? strlen(substr) : semicolon - substr;
    char* device_id = (char*)calloc(1, length + 1);
    memcpy(device_id, substr, length);
    device_id[length] = '\0';

    return device_id;
}

CAzureIot::CAzureIot(const std::string& connectionString)
{
    char device_id[257];
    char* device_id_src = get_device_id(connectionString.c_str());

    if (device_id_src == NULL)
    {
        fprintf(stderr,"invalid connection string");
        exit(1);
    }

    snprintf(device_id, sizeof(device_id), "%s", device_id_src);
    free(device_id_src);

}