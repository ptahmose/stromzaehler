# CMakeList.txt : CMake project for Middleware, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (Middleware "smlreceive.cpp" "smlreceive.h" "ReadMessage.h" "ReadMessage.cpp" "utils.h" "utils.cpp" "azureiothub.cpp" "azureiothub.h")

# TODO: Add tests and install targets if needed.
target_include_directories(Middleware PRIVATE "/usr/local/include/azureiot"
                    "/usr/local/include/azureiot/inc/")
                
target_link_libraries(Middleware 
                          serializer
                          iothub_client
                          iothub_client_mqtt_transport
                          umqtt
                          aziotsharedutil
                          ssl
                          crypto
                          curl
                          pthread
                          m
                          ssl
                          crypto)
