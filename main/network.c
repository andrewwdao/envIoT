/** ------------------------------------------------------------*-
  NETWORK - source file
  (c) Minh-An Dao - Anh Khoi Tran 2020
  version 1.00 - 07/10/2020
---------------------------------------------------------------
 * Configuration to connect to Wifi and Ethernet.
 * 
 * ref: esp-idf\examples\common_components\protocol_examples_common
 *      https://github.com/espressif/esp-idf/issues/894 
 --------------------------------------------------------------*/
#ifndef __NETWORK_C
#define __NETWORK_C
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_pm.h"
#include "esp_wifi_default.h"
#if CONFIG_CONNECT_ETHERNET
#include "esp_eth.h"
#include "driver/gpio.h"
#endif

#include "lwip/err.h"
#include "lwip/sys.h"

#include "network.h"

// ------ Private constants -----------------------------------
/**
 * @brief format the ip to the correct form.
 * @note  define of IP4_ADDR - https://github.com/espressif/esp-idf/blob/master/components/mdns/test_afl_fuzz_host/esp32_compat.h
 */

#ifdef CONFIG_STATIC_IP
#define IP4_ADDR(ipaddr, a,b,c,d) \
        (ipaddr)->addr = ((uint32_t)((d) & 0xff) << 24) | \
                         ((uint32_t)((c) & 0xff) << 16) | \
                         ((uint32_t)((b) & 0xff) << 8)  | \
                          (uint32_t)((a) & 0xff)
#endif

#ifdef CONFIG_CONNECT_IPV6
#define MAX_IP6_ADDRS_PER_NETIF (5)
#define NR_OF_IP_ADDRESSES_TO_WAIT_FOR (_activ_if*2)

#if defined(CONFIG_CONNECT_IPV6_PREF_LOCAL_LINK)
#define CONNECT_PREFERRED_IPV6_TYPE ESP_IP6_ADDR_IS_LINK_LOCAL
#elif defined(CONFIG_CONNECT_IPV6_PREF_GLOBAL)
#define CONNECT_PREFERRED_IPV6_TYPE ESP_IP6_ADDR_IS_GLOBAL
#elif defined(CONFIG_CONNECT_IPV6_PREF_SITE_LOCAL)
#define CONNECT_PREFERRED_IPV6_TYPE ESP_IP6_ADDR_IS_SITE_LOCAL
#elif defined(CONFIG_CONNECT_IPV6_PREF_UNIQUE_LOCAL)
#define CONNECT_PREFERRED_IPV6_TYPE ESP_IP6_ADDR_IS_UNIQUE_LOCAL
#endif // if-elif CONFIG_CONNECT_IPV6_PREF_...

#else
#define NR_OF_IP_ADDRESSES_TO_WAIT_FOR (_activ_if)
#endif
// ------ Private function prototypes -------------------------
#if CONFIG_CONNECT_WIFI
static esp_netif_t* __wifi_start(void);
static void __wifi_stop(void);
#endif
#if CONFIG_CONNECT_ETHERNET
static esp_netif_t* __eth_start(void);
static void __eth_stop(void);
#endif
// ------ Private variables -----------------------------------
/** @brief tag used for ESP serial console messages */
static const char *TAG = "NETWORK";

static int _activ_if = 0; //active interfaces
static xSemaphoreHandle _semph_got_ips;
static esp_ip4_addr_t _ip_addr;
#if CONFIG_CONNECT_WIFI
static esp_netif_t *_sta_netif = NULL;
#endif
#if CONFIG_CONNECT_ETHERNET
static esp_netif_t *_eth_netif = NULL;
#endif

#ifdef CONFIG_CONNECT_IPV6
static esp_ip6_addr_t _ipv6_addr;

/* types of ipv6 addresses to be displayed on ipv6 events */
static const char *_ipv6_addr_types[] = {
    "ESP_IP6_ADDR_IS_UNKNOWN",
    "ESP_IP6_ADDR_IS_GLOBAL",
    "ESP_IP6_ADDR_IS_LINK_LOCAL",
    "ESP_IP6_ADDR_IS_SITE_LOCAL",
    "ESP_IP6_ADDR_IS_UNIQUE_LOCAL",
    "ESP_IP6_ADDR_IS_IPV4_MAPPED_IPV6"
    };
#endif


// ------ PUBLIC variable definitions -------------------------

//--------------------------------------------------------------
// FUNCTION DEFINITIONS
//--------------------------------------------------------------
/**
 * @brief Checks the netif description if it contains specified prefix.
 * All netifs created withing common connect component are prefixed with the module TAG,
 * so it returns true if the specified netif is owned by this module
 */
// static bool _is_our_netif(const char *prefix, esp_netif_t *netif)
// {
//     return strncmp(prefix, esp_netif_get_desc(netif), strlen(prefix)-1) == 0;
// }
/**
 * @brief tear down connection, release resources
 */
static void __stop(void)
{
#if CONFIG_CONNECT_WIFI
    __wifi_stop();
    _activ_if--;
#endif
#if CONFIG_CONNECT_ETHERNET
    __eth_stop();
    _activ_if--;
#endif
}
/**
 * @brief Event for getting ip
 */
static void __on_got_ip(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    // if (!_is_our_netif(TAG, event->esp_netif)) {
    //     ESP_LOGW(TAG, "[Ignored] Got IPv4 from another interface %s", esp_netif_get_desc(event->esp_netif));
    //     return;
    // }
    // Warning: the interface desc is used in tests to capture actual connection details (IP, gw, mask)
    ESP_LOGW(TAG, "Connected by %s", esp_netif_get_desc(event->esp_netif));
    ESP_ERROR_CHECK(esp_netif_get_ip_info(event->esp_netif, &event->ip_info));
    ESP_LOGW(TAG, "- IPv4 address: " IPSTR, IP2STR(&event->ip_info.ip));
    // ESP_LOGW(TAG, "%s Got IPv4: " IPSTR, esp_netif_get_desc(event->esp_netif), IP2STR(&event->ip_info.ip));
    memcpy(&_ip_addr, &event->ip_info.ip, sizeof(_ip_addr));
    xSemaphoreGive(_semph_got_ips);
}

#ifdef CONFIG_CONNECT_IPV6

static void __on_got_ipv6(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data)
{
    ip_event_got_ip6_t *event = (ip_event_got_ip6_t *)event_data;
    // if (!_is_our_netif(TAG, event->esp_netif)) {
    //     ESP_LOGW(TAG, "Got IPv6 from another netif: ignored");
    //     return;
    // }
    esp_ip6_addr_type_t ipv6_type = esp_netif_ip6_get_addr_type(&event->ip6_info.ip);
    ESP_LOGW(TAG, "- IPv6 address:" IPV6STR, IPV62STR(event->ip6_info.ip));
    ESP_LOGW(TAG, "- Type: %s", _ipv6_addr_types[ipv6_type]);
    if (ipv6_type == CONNECT_PREFERRED_IPV6_TYPE) {
        memcpy(&_ipv6_addr, &event->ip6_info.ip, sizeof(_ipv6_addr));
        xSemaphoreGive(_semph_got_ips);
    }
}

#endif // CONFIG_CONNECT_IPV6

#ifdef CONFIG_CONNECT_WIFI

esp_event_handler_instance_t _ins_wifi_event;
esp_event_handler_instance_t _ins_wifi_got_ip;
#ifdef CONFIG_CONNECT_IPV6
esp_event_handler_instance_t _ins_wifi_got_ipv6;
#endif // CONFIG_CONNECT_IPV6

/**
 * @brief handler for Wifi events
 */
static void __on_wifi_event(void *esp_netif, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    switch (event_id) {
    case WIFI_EVENT_STA_START:
        ESP_LOGI(TAG, "Starting wifi...\n");
        ESP_LOGI(TAG, "Connecting to %s...\n", CONFIG_WIFI_SSID);
        ESP_ERROR_CHECK(esp_wifi_connect());
        break;
    case WIFI_EVENT_STA_CONNECTED:
#ifdef CONFIG_CONNECT_IPV6
        esp_netif_create_ip6_linklocal(esp_netif);
#endif
#if CONFIG_CONNECT_ETHERNET
        __eth_stop();
        _activ_if--;
#endif
        // esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
        // ESP_LOGI(TAG, "Ethernet Up");
        // ESP_LOGI(TAG, "HW MAC Addr: %02x:%02x:%02x:%02x:%02x:%02x", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        ESP_LOGI(TAG, "Wi-Fi disconnected, reconnect to %s...\n", CONFIG_WIFI_SSID);
        esp_err_t err = esp_wifi_connect();
        if (err == ESP_ERR_WIFI_NOT_STARTED) return;
        ESP_ERROR_CHECK(err);
        break;
    default:
        break;
    }
}

/**
 * @brief wifi start function with power saving mode
 */
static esp_netif_t* __wifi_start(void)
{
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

#ifdef CONFIG_STATIC_IP
    //set static ip - https://esp32.com/viewtopic.php?f=2&t=14689
    //define of IP4_ADDR - https://github.com/espressif/esp-idf/blob/master/components/mdns/test_afl_fuzz_host/esp32_compat.h
    esp_netif_dhcpc_stop(sta_netif);
    esp_netif_ip_info_t ip_info;
    IP4_ADDR(&ip_info.ip, 192, 168, 1, 174);
   	IP4_ADDR(&ip_info.gw, 192, 168, 1, 1);
   	IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);
    esp_netif_set_ip_info(sta_netif, &ip_info);
#endif

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // -------- create interface description to distinguish eth and 
    // esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_WIFI_STA();
    // // Prefix the interface description with the module TAG
    // // Warning: the interface desc is used in tests to capture actual connection details (IP, gw, mask)
    // char *desc;
    // asprintf(&desc, "%s: %s", TAG, esp_netif_config.if_desc);
    // esp_netif_config.if_desc = desc;
    // esp_netif_config.route_prio = 128;
    // esp_netif_t *sta_netif = esp_netif_create_wifi(WIFI_IF_STA, &esp_netif_config);
    // free(desc);
    // esp_wifi_set_default_wifi_sta_handlers();

    // Register user defined event handers
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                               ESP_EVENT_ANY_ID,
                                               &__on_wifi_event, sta_netif,
                                               &_ins_wifi_event));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                               IP_EVENT_STA_GOT_IP,
                                               &__on_got_ip, NULL,
                                               &_ins_wifi_got_ip));
#ifdef CONFIG_CONNECT_IPV6
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                IP_EVENT_GOT_IP6,
                                                &__on_got_ipv6, NULL,
                                                &_ins_wifi_got_ipv6));
#endif

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
            .listen_interval = CONFIG_LISTEN_INTERVAL,
            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting below line */
	        .threshold.authmode = WIFI_AUTH_WPA2_PSK
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
#if CONFIG_PM_ENABLE
    // Configure dynamic frequency scaling:
    // maximum and minimum frequencies are set in sdkconfig,
    // automatic light sleep is enabled if tickless idle support is enabled.
#if CONFIG_IDF_TARGET_ESP32
    esp_pm_config_esp32_t pm_config = {
#elif CONFIG_IDF_TARGET_ESP32S2
    esp_pm_config_esp32s2_t pm_config = {
#endif
            .max_freq_mhz = CONFIG_MAX_CPU_FREQ_MHZ,
            .min_freq_mhz = CONFIG_MIN_CPU_FREQ_MHZ,
#if CONFIG_FREERTOS_USE_TICKLESS_IDLE
            .light_sleep_enable = true
#endif
    };
    ESP_ERROR_CHECK(esp_pm_configure(&pm_config));
#endif // CONFIG_PM_ENABLE
    ESP_LOGI(TAG, "entering Wifi Power save mode...\n");
#if CONFIG_POWER_SAVE_MIN
    esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
#elif CONFIG_POWER_SAVE_MAX
    esp_wifi_set_ps(WIFI_PS_MAX_MODEM);
#elif CONFIG_POWER_SAVE_NONE
    esp_wifi_set_ps(WIFI_PS_NONE);
#else
    esp_wifi_set_ps(WIFI_PS_NONE);
#endif
    return sta_netif;
}
/**
 * @brief wifi stop function
 */
static void __wifi_stop(void)
{
    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT,
                                                 ESP_EVENT_ANY_ID,
                                                 _ins_wifi_event));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT,
                                                 IP_EVENT_STA_GOT_IP,
                                                 _ins_wifi_got_ip));
#ifdef CONFIG_CONNECT_IPV6
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT,
                                                 IP_EVENT_GOT_IP6,
                                                 _ins_wifi_got_ipv6));
#endif
    esp_err_t err = esp_wifi_stop();
    if (err == ESP_ERR_WIFI_NOT_INIT) {
        return;
    }
    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(esp_wifi_deinit());
    ESP_ERROR_CHECK(esp_wifi_clear_default_wifi_driver_and_handlers(_sta_netif));
    esp_netif_destroy(_sta_netif);
    _sta_netif = NULL;
}
#endif // CONFIG_CONNECT_WIFI

#ifdef CONFIG_CONNECT_ETHERNET

static esp_eth_handle_t _eth_handle = NULL;
static esp_eth_mac_t *_mac = NULL;
static esp_eth_phy_t *_phy = NULL;
static void *_eth_glue = NULL;
esp_event_handler_instance_t _ins_eth_event;
esp_event_handler_instance_t _ins_eth_gotip;
#ifdef CONFIG_CONNECT_IPV6
esp_event_handler_instance_t _ins_eth_gotipv6;
#endif // CONFIG_CONNECT_IPV6

/**
 * @brief Event handler for Ethernet events
 */
static void __on_eth_event(void *esp_netif, esp_event_base_t event_base,
                           int32_t event_id, void *event_data)
{
    uint8_t mac_addr[6] = {0};
    /* we can get the ethernet driver handle from event data */
    esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;

    switch (event_id) {
    case ETHERNET_EVENT_START:
        ESP_LOGI(TAG, "Ethernet Started");
        break;
    case ETHERNET_EVENT_STOP:
        ESP_LOGI(TAG, "Ethernet Stopped");
        break;
    case ETHERNET_EVENT_CONNECTED:
#ifdef CONFIG_CONNECT_IPV6
        esp_netif_create_ip6_linklocal(esp_netif);
#endif
#if CONFIG_CONNECT_WIFI
        __wifi_stop();
        _activ_if--;
#endif
        esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
        ESP_LOGI(TAG, "Ethernet Up");
        ESP_LOGI(TAG, "HW MAC Addr: %02x:%02x:%02x:%02x:%02x:%02x", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
        break;
    case ETHERNET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "Ethernet Down");
        break;
    default:
        break;
    }
}

static esp_netif_t* __eth_start(void)
{
    // char *desc;
    // esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_ETH();
    // // Prefix the interface description with the module TAG
    // // Warning: the interface desc is used in tests to capture actual connection details (IP, gw, mask)
    // asprintf(&desc, "%s: %s", TAG, esp_netif_config.if_desc);
    // esp_netif_config.if_desc = desc;
    // esp_netif_config.route_prio = 64;
    // esp_netif_config_t netif_config = {
    //         .base = &esp_netif_config,
    //         .stack = ESP_NETIF_NETSTACK_DEFAULT_ETH
    // };
    // esp_netif_t *netif = esp_netif_new(&netif_config);
    // assert(netif);
    // free(desc);
    // // Set default handlers to process TCP/IP stuffs
    // ESP_ERROR_CHECK(esp_eth_set_default_handlers(netif));

    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
    esp_netif_t *eth_netif = esp_netif_new(&cfg);

#ifdef CONFIG_STATIC_IP
    //set static ip - https://esp32.com/viewtopic.php?f=2&t=14689
    //define of IP4_ADDR - https://github.com/espressif/esp-idf/blob/master/components/mdns/test_afl_fuzz_host/esp32_compat.h
    esp_netif_dhcpc_stop(eth_netif);
    esp_netif_ip_info_t ip_info;
    IP4_ADDR(&ip_info.ip, 192, 168, 1, 174);
   	IP4_ADDR(&ip_info.gw, 192, 168, 1, 1);
   	IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);
    esp_netif_set_ip_info(eth_netif, &ip_info);
#endif

    // Set default handlers to process TCP/IP stuffs
    ESP_ERROR_CHECK(esp_eth_set_default_handlers(eth_netif));
    
    // Register user defined event handers
    ESP_ERROR_CHECK(esp_event_handler_instance_register(ETH_EVENT,
                                               ESP_EVENT_ANY_ID,
                                               &__on_eth_event, eth_netif,
                                               &_ins_eth_event));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                               IP_EVENT_ETH_GOT_IP,
                                               &__on_got_ip, NULL,
                                               &_ins_eth_gotip));

#ifdef CONFIG_CONNECT_IPV6
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                               IP_EVENT_GOT_IP6,
                                               &__on_got_ipv6, NULL,
                                               &_ins_eth_gotipv6));
    // ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ETHERNET_EVENT_CONNECTED, &on_eth_event, netif));
    // ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_GOT_IP6, &__on_got_ipv6, NULL));
#endif
    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.phy_addr = CONFIG_ETH_PHY_ADDR;
    phy_config.reset_gpio_num = CONFIG_ETH_PHY_RST_GPIO;
#if CONFIG_USE_INTERNAL_ETHERNET
    mac_config.smi_mdc_gpio_num = CONFIG_ETH_MDC_GPIO;
    mac_config.smi_mdio_gpio_num = CONFIG_ETH_MDIO_GPIO;
    _mac = esp_eth_mac_new_esp32(&mac_config);
#if CONFIG_ETH_PHY_IP101
    _phy = esp_eth_phy_new_ip101(&phy_config);
#elif CONFIG_ETH_PHY_RTL8201
    _phy = esp_eth_phy_new_rtl8201(&phy_config);
#elif CONFIG_ETH_PHY_LAN8720
    _phy = esp_eth_phy_new_lan8720(&phy_config);
#elif CONFIG_ETH_PHY_DP83848
    _phy = esp_eth_phy_new_dp83848(&phy_config);
#endif
#elif CONFIG_USE_DM9051
    gpio_install_isr_service(0);
    spi_device_handle_t spi_handle = NULL;
    spi_bus_config_t buscfg = {
        .miso_io_num = CONFIG_DM9051_MISO_GPIO,
        .mosi_io_num = CONFIG_DM9051_MOSI_GPIO,
        .sclk_io_num = CONFIG_DM9051_SCLK_GPIO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(CONFIG_DM9051_SPI_HOST, &buscfg, 1));
    spi_device_interface_config_t devcfg = {
        .command_bits = 1,
        .address_bits = 7,
        .mode = 0,
        .clock_speed_hz = CONFIG_DM9051_SPI_CLOCK_MHZ * 1000 * 1000,
        .spics_io_num = CONFIG_DM9051_CS_GPIO,
        .queue_size = 20
    };
    ESP_ERROR_CHECK(spi_bus_add_device(CONFIG_DM9051_SPI_HOST, &devcfg, &spi_handle));
    /* dm9051 ethernet driver is based on spi driver */
    eth_dm9051_config_t dm9051_config = ETH_DM9051_DEFAULT_CONFIG(spi_handle);
    dm9051_config.int_gpio_num = CONFIG_DM9051_INT_GPIO;
    _mac = esp_eth_mac_new_dm9051(&dm9051_config, &mac_config);
    _phy = esp_eth_phy_new_dm9051(&phy_config);
#elif CONFIG_USE_OPENETH
    phy_config.autonego_timeout_ms = 100;
    _mac = esp_eth_mac_new_openeth(&mac_config);
    _phy = esp_eth_phy_new_dp83848(&phy_config);
#endif

    // Install Ethernet driver
    esp_eth_config_t config = ETH_DEFAULT_CONFIG(_mac, _phy);
    ESP_ERROR_CHECK(esp_eth_driver_install(&config, &_eth_handle));
    // combine driver with netif
    _eth_glue = esp_eth_new_netif_glue(_eth_handle);
    /* attach Ethernet driver to TCP/IP stack */
    esp_netif_attach(eth_netif, _eth_glue);
    /* start Ethernet driver state machine */
    esp_eth_start(_eth_handle);
    return eth_netif;
}

static void __eth_stop(void)
{
    // esp_netif_t *_eth_netif = esp_netif_get_desc("eth");
    //ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_ETH_GOT_IP, &__on_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(ETH_EVENT,
                                                ESP_EVENT_ANY_ID,
                                                _ins_eth_event));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT,
                                                IP_EVENT_ETH_GOT_IP,
                                                _ins_eth_gotip));

#ifdef CONFIG_CONNECT_IPV6
    // ESP_ERROR_CHECK(esp_event_handler_instance_unregister(ETH_EVENT,
    //                                             ETHERNET_EVENT_CONNECTED,
    //                                             _ins_eth_connectv6));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT,
                                                IP_EVENT_GOT_IP6,
                                                _ins_eth_gotipv6));
    // ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_GOT_IP6, &__on_got_ipv6));
    // ESP_ERROR_CHECK(esp_event_handler_unregister(ETH_EVENT, ETHERNET_EVENT_CONNECTED, &on_eth_event));
#endif
    ESP_ERROR_CHECK(esp_eth_stop(_eth_handle));
    ESP_ERROR_CHECK(esp_eth_del_netif_glue(_eth_glue));
    ESP_ERROR_CHECK(esp_eth_clear_default_handlers(_eth_netif));
    ESP_ERROR_CHECK(esp_eth_driver_uninstall(_eth_handle));
    ESP_ERROR_CHECK(_phy->del(_phy));
    ESP_ERROR_CHECK(_mac->del(_mac));
    esp_netif_destroy(_eth_netif);
    _eth_netif = NULL;
}

#endif // CONFIG_CONNECT_ETHERNET

/**
 * @brief network initialize function - needed for any net interface
 */
esp_err_t network_init(void)
{
    if (_semph_got_ips != NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    // Initialize TCP/IP network interface (should be called only once in application)
    ESP_ERROR_CHECK(esp_netif_init());
    // Create default event loop that running in background
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    return ESP_OK;
}
/**
 * @brief Connect to Wi-Fi and/or Ethernet, wait for IP
 * @return ESP_OK on successful connection
 */
esp_err_t network_start(void)
{
#if CONFIG_CONNECT_WIFI
    _sta_netif = __wifi_start();
    _activ_if++;
#endif
#if CONFIG_CONNECT_ETHERNET
    _eth_netif = __eth_start();
    _activ_if++;
#endif
// #if CONFIG_CONNECT_WIFI && CONFIG_CONNECT_ETHERNET
//     /* if both intefaces at once, clear out to indicate that multiple netifs are active */
//     s_esp_netif = NULL;
// #endif
    _semph_got_ips = xSemaphoreCreateCounting(NR_OF_IP_ADDRESSES_TO_WAIT_FOR, 0);

    ESP_ERROR_CHECK(esp_register_shutdown_handler(&__stop));
    ESP_LOGI(TAG, "Waiting for any connection");
    xSemaphoreTake(_semph_got_ips, portMAX_DELAY);
    //after gaining connection, go to other tasks
    return ESP_OK;
}
/**
 * @brief network stop function (public)
 */
esp_err_t network_stop(void)
{
    if (_semph_got_ips == NULL) return ESP_ERR_INVALID_STATE;
    vSemaphoreDelete(_semph_got_ips);
    _semph_got_ips = NULL;
    ESP_ERROR_CHECK(esp_unregister_shutdown_handler(&__stop));
    __stop();
    return ESP_OK;
}

// esp_netif_t *get_netif(void)
// {
//     return s_esp_netif;
// }

// esp_netif_t *get_netif_from_desc(const char *desc)
// {
//     esp_netif_t *netif = NULL;
//     char *expected_desc;
//     asprintf(&expected_desc, "%s: %s", TAG, desc);
//     while ((netif = esp_netif_next(netif)) != NULL) {
//         if (strcmp(esp_netif_get_desc(netif), expected_desc) == 0) {
//             free(expected_desc);
//             return netif;
//         }
//     }
//     free(expected_desc);
//     return netif;
// }

#endif
