idf_component_register(SRCS "network.c" "mqtt.c" "led.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES esp_netif mqtt
                    )
