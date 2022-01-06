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
#include <stdio.h>

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

char* parse_iothub_name(const char* connectionString)
{
    if (connectionString == NULL)
    {
        return NULL;
    }

    char* cs = strdup(connectionString);

    char* hostName = strtok(cs, ".");
    int prefixLen = strlen("HostName=");
    int len = strlen(hostName) - prefixLen + 1;
    char* iotHubName = (char*)malloc(len);
    if (iotHubName == NULL)
    {
        return NULL;
    }
    memcpy(iotHubName, hostName + prefixLen, len - 1);
    iotHubName[len - 1] = '\0';
    free(cs);
    return iotHubName;
}


#define INTERVAL 2000

const char* onSuccess = "\"Successfully invoke device method\"";
const char* notFound = "\"No method found\"";
static const char* EVENT_SUCCESS = "success";
static const char* EVENT_FAILED = "failed";

static int interval = INTERVAL;

void* send_telemetry_data_multi_thread(char* iotHubName, const char* eventName, const char* message)
{
    AIParams* params = (AIParams*)malloc(sizeof(AIParams));
    if (params != NULL)
    {
        params->iotHubName = iotHubName;
        params->event = eventName;
        params->message = message;
        pthread_create(&thread, NULL, send_ai, (void*)params);
    }
    else
    {
        free(iotHubName);
    }
}

static void start()
{
    //sendingMessage = true;
    printf("*** START ***\n");
}

static void stop()
{
    //sendingMessage = false;
    printf("*** STOP ***\n");
}

int deviceMethodCallback(
    const char* methodName,
    const unsigned char* payload,
    size_t size,
    unsigned char** response,
    size_t* response_size,
    void* userContextCallback)
{
    printf("Try to invoke method %s\r\n", methodName);
    const char* responseMessage = onSuccess;
    int result = 200;

    if (strcmp(methodName, "start") == 0)
    {
        start();
    }
    else if (strcmp(methodName, "stop") == 0)
    {
        stop();
    }
    else
    {
        printf("No method %s found\r\n", methodName);
        responseMessage = notFound;
        result = 404;
    }

    *response_size = strlen(responseMessage);
    *response = (unsigned char*)malloc(*response_size);
    strncpy((char*)(*response), responseMessage, *response_size);

    return result;
}

void twinCallback(
    DEVICE_TWIN_UPDATE_STATE updateState,
    const unsigned char* payLoad,
    size_t size,
    void* userContextCallback)
{
    char* temp = (char*)malloc(size + 1);
    for (int i = 0; i < size; i++)
    {
        temp[i] = (char)(payLoad[i]);
    }
    temp[size] = '\0';
    MULTITREE_HANDLE tree = NULL;

    if (JSON_DECODER_OK == JSONDecoder_JSON_To_MultiTree(temp, &tree))
    {
        MULTITREE_HANDLE child = NULL;

        if (MULTITREE_OK != MultiTree_GetChildByName(tree, "desired", &child))
        {
            LogInfo("This device twin message contains desired message only");
            child = tree;
        }
        const void* value = NULL;
        if (MULTITREE_OK == MultiTree_GetLeafValue(child, "interval", &value))
        {
            interval = atoi((const char*)value);
        }
    }
    MultiTree_Destroy(tree);
    free(temp);
}

IOTHUBMESSAGE_DISPOSITION_RESULT receiveMessageCallback(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback)
{
    const unsigned char* buffer = NULL;
    size_t size = 0;

    if (IOTHUB_MESSAGE_OK != IoTHubMessage_GetByteArray(message, &buffer, &size))
    {
        return IOTHUBMESSAGE_ABANDONED;
    }

    // message needs to be converted to zero terminated string
    char* temp = (char*)malloc(size + 1);

    if (temp == NULL)
    {
        return IOTHUBMESSAGE_ABANDONED;
    }

    strncpy(temp, buffer, size);
    temp[size] = '\0';

    (void)printf("Receiving message: %s\r\n", temp);
    free(temp);

    return IOTHUBMESSAGE_ACCEPTED;
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

    this->connectionstring = connectionString;
}

void CAzureIot::Run()
{
    IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;

    if (platform_init() != 0)
    {
        fprintf(stderr,"Failed to initialize the platform.\n");
        exit(1);
    }
    else
    {
        if ((iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(this->connectionstring.c_str(), MQTT_Protocol)) == NULL)
        {
            fprintf(stderr, "iotHubClientHandle is NULL!\n");
            exit(1);// send_telemetry_data(NULL, EVENT_FAILED, "Cannot create iotHubClientHandle");
        }
        else
        {
            // set C2D and device method callback
            IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, receiveMessageCallback, NULL);
            IoTHubClient_LL_SetDeviceMethodCallback(iotHubClientHandle, deviceMethodCallback, NULL);
            IoTHubClient_LL_SetDeviceTwinCallback(iotHubClientHandle, twinCallback, NULL);

            IoTHubClient_LL_SetOption(iotHubClientHandle, "product_info", "HappyPath_RaspberryPi-C");

            char* iotHubName = parse_iothub_name(this->connectionstring.c_str());
            send_telemetry_data_multi_thread(iotHubName, EVENT_SUCCESS, "IoT hub connection is established");
            int count = 0;

        }
    }
}