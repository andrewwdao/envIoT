idf_component_register(SRCS "network.c" "mqtt.c" "led.c" "storage.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES esp_netif mqtt nvs_flash
                    EMBED_TXTFILES "ssl/cert.pem"
                    )
