#ifndef STOMECLIENT_H
#define STOMECLIENT_H

#include <stdio.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
// #include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_websocket_client.h"
#include "esp_event.h"


#include <string.h>          // For handling strings
#include "stdbool.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include "lwip/err.h"        // Light weight IP packets error handling
#include "lwip/sys.h"        // System applications for lightweight IP apps
#include "nvs.h"
#include "esp_err.h"
#include "esp_vfs.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"







#ifdef __cplusplus
extern "C" {
#endif


#define     DEVICE_NAME         "THT-Face"
#define     PUBLISH_TOPIC       "/app/cloud"
#define     SUBCRIBE_TOPIC       "/topic/cloud"




#define IDLEENROL 0
#define ENROLING 0x01
#define ENROLED 0x02
#define DUPLICATE 0x03

#define TIMEOUT_50_MS         5
#define TIMEOUT_100_MS        10
#define TIMEOUT_120_MS        12
#define TIMEOUT_150_MS        15
#define TIMEOUT_200_MS        20
#define TIMEOUT_300_MS        30
#define TIMEOUT_500_MS        50
#define TIMEOUT_1000_MS       100
#define TIMEOUT_2000_MS       200
#define TIMEOUT_3000_MS       300
#define TIMEOUT_4000_MS       400
#define TIMEOUT_5000_MS       500
#define TIMEOUT_6000_MS       600
#define TIMEOUT_7000_MS       700
#define TIMEOUT_9000_MS       900
#define TIMEOUT_10000_MS      1000
#define TIMEOUT_12000_MS      1200
#define TIMEOUT_20000_MS      2000
#define TIMEOUT_15_S          1500
#define TIMEOUT_30_S          3000
#define TIMEOUT_45_S          4500
#define TIMEOUT_1_MIN         6000
#define TIMEOUT_2_MIN         12000
#define TIMEOUT_5_MIN         30000
#define LISTEN_BACKLOG 1
#define ACK_SIZE 1024

uint32_t crc_table[256];
uint16_t crc16_table[256];







#define     THT             "wss://grozziieget.zjweiting.com:3091/CloudSocket-Dev/websocket/"
#define     HOST            "grozziieget.zjweiting.com"
#define     PORT            3091
#define     PATH            "/CloudSocket-Dev/websocket/"

#define     CHANK_SIZE     256 //512//760//256

typedef struct {
    const char                  *uri;                       /*!< Websocket URI, the information on the URI can be overrides the other fields below, if any */
    const char                  *host;                      /*!< Domain or IP as string */
    int                         port;                       /*!< Port to connect, default depend on esp_websocket_transport_t (80 or 443) */
    const char                  *path;                       /*!< Websocket URI, the information on the URI can be overrides the other fields below, if any */
} stompInfo_cfg_t;


esp_websocket_client_handle_t client;
const char echo_org_ssl_ca_cert[]  = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIFBzCCA++gAwIBAgIRALIM7VUuMaC/NDp1KHQ76aswDQYJKoZIhvcNAQELBQAw\n" \
"ezELMAkGA1UEBhMCR0IxGzAZBgNVBAgMEkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4G\n" \
"A1UEBwwHU2FsZm9yZDEaMBgGA1UECgwRQ29tb2RvIENBIExpbWl0ZWQxITAfBgNV\n" \
"BAMMGEFBQSBDZXJ0aWZpY2F0ZSBTZXJ2aWNlczAeFw0yMjAxMTAwMDAwMDBaFw0y\n" \
"ODEyMzEyMzU5NTlaMFkxCzAJBgNVBAYTAkNOMSUwIwYDVQQKExxUcnVzdEFzaWEg\n" \
"VGVjaG5vbG9naWVzLCBJbmMuMSMwIQYDVQQDExpUcnVzdEFzaWEgUlNBIERWIFRM\n" \
"UyBDQSBHMjCCAaIwDQYJKoZIhvcNAQEBBQADggGPADCCAYoCggGBAKjGDe0GSaBs\n" \
"Yl/VhMaTM6GhfR1TAt4mrhN8zfAMwEfLZth+N2ie5ULbW8YvSGzhqkDhGgSBlafm\n" \
"qq05oeESrIJQyz24j7icGeGyIZ/jIChOOvjt4M8EVi3O0Se7E6RAgVYcX+QWVp5c\n" \
"Sy+l7XrrtL/pDDL9Bngnq/DVfjCzm5ZYUb1PpyvYTP7trsV+yYOCNmmwQvB4yVjf\n" \
"IIpHC1OcsPBntMUGeH1Eja4D+qJYhGOxX9kpa+2wTCW06L8T6OhkpJWYn5JYiht5\n" \
"8exjAR7b8Zi3DeG9oZO5o6Qvhl3f8uGU8lK1j9jCUN/18mI/5vZJ76i+hsgdlfZB\n" \
"Rh5lmAQjD80M9TY+oD4MYUqB5XrigPfFAUwXFGehhlwCVw7y6+5kpbq/NpvM5Ba8\n" \
"SeQYUUuMA8RXpTtGlrrTPqJryfa55hTuX/ThhX4gcCVkbyujo0CYr+Uuc14IOyNY\n" \
"1fD0/qORbllbgV41wiy/2ZUWZQUodqHWkjT1CwIMbQOY5jmrSYGBwwIDAQABo4IB\n" \
"JjCCASIwHwYDVR0jBBgwFoAUoBEKIz6W8Qfs4q8p74Klf9AwpLQwHQYDVR0OBBYE\n" \
"FF86fBEQfgxncWHci6O1AANn9VccMA4GA1UdDwEB/wQEAwIBhjASBgNVHRMBAf8E\n" \
"CDAGAQH/AgEAMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjAiBgNVHSAE\n" \
"GzAZMA0GCysGAQQBsjEBAgIxMAgGBmeBDAECATBDBgNVHR8EPDA6MDigNqA0hjJo\n" \
"dHRwOi8vY3JsLmNvbW9kb2NhLmNvbS9BQUFDZXJ0aWZpY2F0ZVNlcnZpY2VzLmNy\n" \
"bDA0BggrBgEFBQcBAQQoMCYwJAYIKwYBBQUHMAGGGGh0dHA6Ly9vY3NwLmNvbW9k\n" \
"b2NhLmNvbTANBgkqhkiG9w0BAQsFAAOCAQEAHMUom5cxIje2IiFU7mOCsBr2F6CY\n" \
"eU5cyfQ/Aep9kAXYUDuWsaT85721JxeXFYkf4D/cgNd9+hxT8ZeDOJrn+ysqR7NO\n" \
"2K9AdqTdIY2uZPKmvgHOkvH2gQD6jc05eSPOwdY/10IPvmpgUKaGOa/tyygL8Og4\n" \
"3tYyoHipMMnS4OiYKakDJny0XVuchIP7ZMKiP07Q3FIuSS4omzR77kmc75/6Q9dP\n" \
"v4wa90UCOn1j6r7WhMmX3eT3Gsdj3WMe9bYD0AFuqa6MDyjIeXq08mVGraXiw73s\n" \
"Zale8OMckn/BU3O/3aFNLHLfET2H2hT6Wb3nwxjpLIfXmSVcVd8A58XH0g==\n" \
"-----END CERTIFICATE-----\n";



// const char testdata[] = "-----BEGIN CERTIFICATE-----MIIGfzCCBOegAwIBAgIRANBUG6rsL5QvgWixLuHWMVcwDQYJKoZIhvcNAQEMBQAwWTELMAkGA1UEBhMCQ04xJTAjBgNVBAoTHFRydXN0QXNpYSBUZWNobm9sb2dpZXMsIEluYy4xIzAhBgNVBAMTGl
// ydXN0QXNpYSBSU0EgRFYgVExTIENBIEcyMB4XDTIzMTEyMDAwMDAwMFoXDTI0MTExOTIzNTk1OVowJDEiMCAGA1UEAxMZZ3JvenppaWVnZXQuemp3ZWl0aW5nLmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKiNfh80wv6vspWnJtj1aDff8/5jIy2L/SZU
// 3hye3xyleASlFh1ARVUNWBWeoajJ1eB5AwwsXtz1CtS+ZYOYUzsnhEokMzO3Ae+3EipgUbNZGwMKTq3MJF+EMOACUBIMszxbF4VUHZMRyJ9BdvlC1be2IkfNY3CFnGyW+RO63hHxl/MR+L7qIacZQWetl/Aq6bcgzG+dH5QocXyOpd/Zi4UXHme0IuInYEqPEto4HE5W0Z9iq0X
// CWgAXzl5Hfo2NL5+57QZEf1dBHBhCqk6oIoxEaE3K95T8iVeW584j+vbd8iTdDIwBUOzqJbHSHVP1HZg1uFODCeCYxhouY5NRZbsCAwEAAaOCAvUwggLxMB8GA1UdIwQYMBaAFF86fBEQfgxncWHci6O1AANn9VccMB0GA1UdDgQWBBQRtVDPMqfYxoD/l92X3uGqFiKe+jAOBgN
// VHQ8BAf8EBAMCBaAwDAYDVR0TAQH/BAIwADAdBgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwSQYDVR0gBEIwQDA0BgsrBgEEAbIxAQICMTAlMCMGCCsGAQUFBwIBFhdodHRwczovL3NlY3RpZ28uY29tL0NQUzAIBgZngQwBAgEwfQYIKwYBBQUHAQEEcTBvMEIGCCsGAQUF
// BzAChjZodHRwOi8vY3J0LnRydXN0LXByb3ZpZGVyLmNuL1RydXN0QXNpYVJTQURWVExTQ0FHMi5jcnQwKQYIKwYBBQUHMAGGHWh0dHA6Ly9vY3NwLnRydXN0LXByb3ZpZGVyLmNuMCQGA1UdEQQdMBuCGWdyb3p6aWllZ2V0Lnpqd2VpdGluZy5jb20wggGABgorBgEEAdZ5AgQCBIIB
// cASCAWwBagB3AHb/iD8KtvuVUcJhzPWHujS0pM27KdxoQgqf5mdMWjp0AAABi+wMtZcAAAQDAEgwRgIhAP4l2Kce6/yN7E7BsKlvpKPZZmui2rG6R8lSXZZn2iUmAiEAiRlZGSZcOUUiGorK9I3NAOfOQCbOGmUsC6qq9si5oNAAdgA/F0tP1yJHWJQdZRyEvg0S7ZA3fx+FauvBvyiF7P
// hkbgAAAYvsDLW+AAAEAwBHMEUCIQCO1IlqWnHfHmdw9M8XpITVH5a0jgJ2PqDdulLhX6LndQIgWlf8A5RYpOxGjzAilH/AZPMXXGM2eCE0bD+/Ubaao18AdwDuzdBk1dsazsVct520zROiModGfLzs3sNRSFlGcR+1mwAAAYvsDLXEAAAEAwBIMEYCIQCTVnVdgKYWftOgTtikYkHyQFA8b
// 4l7F48PS2pIaxWwXAIhAMLau0syoJx1iNmteK3HVx2tZn44Dzosudtr+oX/ruCBMA0GCSqGSIb3DQEBDAUAA4IBgQAcneTRxTtzc+uFOUbfNkwfoC9DYvpnMEsI1+IZs+d61+HyuLf2Pz7a7v+B9sQoxmCpjcpyV+OnE+nUHWbuQKW4fI3/jUemf7kL2mCNmB6Y4He2NgV5asPCONDgoK4y
// BFAvu2kEJy7kZ4L5iUvVBjY8yJ2PiHWZlg/RWzO6WtSsnIMsnE2VyK+EbwUy4bt0pg2nt7A8YGoMZePMLSVRbnTO8WENPN0ZhgJSuNlZWxfgEOEpmNB5JolTZJ1zhlF26596r2JQsIKl+rclsQYgYvuDpOvmkRShlV3StvL7EnQ
// jqgeTHX1N4f7yNMZaknvckKvTpW33Isx1qCzmPMP6rRs7hEiIG1hoNhWKD7qKQ2gGRbMEpL75vSbEhjaE9ALtKZS4Lf6kxveiOk7bayyykRzoroVoVvb0l+6jnrVk0k/JtPMIIGfzCCBOegAwIBAgIRANBUG6rsL5QvgWixLuHWMVcwDQYJKoZ
// IhvcNAQEMBQAwWTELMAkGA1UEBhMCQ04xJTAjBgNVBAoTHFRydXN0QXNpYSBUZWNobm9sb2dpZXMsIEluYy4xIzAhBgNVBAMTGlRydXN0QXNpYSBSU0EgRFYgVExTIENBIEcyMB4XDTIzMTEyMDAwMDAwMFoXDTI0MTExOTIzNTk1OVowJDEiMCA
// GA1UEAxMZZ3JvenppaWVnZXQuemp3ZWl0aW5nLmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKiNfh80wv6vspWnJtj1aDff8/5jIy2L/SZU3hye3xyleASlFh1ARVUNWBWeoajJ1eB5AwwsXtz1CtS+ZYOYUzsnhEokMzO3Ae+3E
// ipgUbNZGwMKTq3MJF+EMOACUBIMszxbF4VUHZMRyJ9BdvlC1be2IkfNY3CFnGyW+RO63hHxl/MR+L7qIacZQWetl/Aq6bcgzG+dH5QocXyOpd/Zi4UXHme0IuInYEqPEto4HE5W0Z9iq0XCWgAXzl5Hfo2NL5+57QZEf1dBHBhCqk6oIoxEaE3K95T8iVeW
// 584j+vbd8iTdDIwBUOzqJbHSHVP1HZg1uFODCeCYxhouY5NRZbsCAwEAAaOCAvUwggLxMB8GA1UdIwQYMBaAFF86fBEQfgxncWHci6O1AANn9VccMB0GA1UdDgQWBBQRtVDPMqfYxoD/l92X3uGqFiKe+jAOBgNVHQ8BAf8EBAMCBaAwDAYDVR0TAQH/BAI
// wADAdBgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwSQYDVR0gBEIwQDA0BgsrBgEEAbIxAQICMTAlMCMGCCsGAQUFBwIBFhdodHRwczovL3NlY3RpZ28uY29tL0NQUzAIBgZngQwBAgEwfQYIKwYBBQUHAQEEcTBvMEIGCCsGAQUFBzAChjZodH
// RwOi8vY3J0LnRydXN0LXByb3ZpZGVyLmNuL1RydXN0QXNpYVJTQURWVExTQ0FHMi5jcnQwKQYIKwYBBQUHMAGGHWh0dHA6Ly9vY3NwLnRydXN0LXByb3ZpZGVyLmNuMCQGA1UdEQQdMBuCGWdyb3p6aWllZ2V0Lnpqd2VpdGluZy5j
// b20wggGABgorBgEEAdZ5AgQCBIIBcASCAWwBagB3AHb/iD8KtvuVUcJhzPWHujS0pM27KdxoQgqf5mdMWjp0AAABi+wMtZcAAAQDAEgwRgIhAP4l2Kce6/yN7E7BsKlvpKPZZmui2rG6R8lSXZZn2iUmAiEAiRlZGSZcOUUiGorK9I
// 3NAOfOQCbOGmUsC6qq9si5oNAAdgA/F0tP1yJHWJQdZRyEvg0S7ZA3fx+FauvBvyiF7PhkbgAAAYvsDLW+AAAEAwBHMEUCIQCO1IlqWnHfHmdw9M8XpITVH5a0jgJ2PqDdulLhX6LndQIgWlf8A5RYpOxGjzAilH/AZPMXXGM2eCE0
// bD+/Ubaao18AdwDuzdBk1dsazsVct520zROiModGfLzs3sNRSFlGcR+1mwAAAYvsDLXEAAAEAwBIMEYCIQCTVnVdgKYWftOgTtikYkHyQFA8b4l7F48PS2pIaxWwXAIhAMLau0syoJx1iNmteK3HVx2tZn44Dzosudtr+oX/ruCBMA
// 0GCSqGSIb3DQEBDAUAA4IBgQAcneTRxTtzc+uFOUbfNkwfoC9DYvpnMEsI1+IZs+d61+HyuLf2Pz7a7v+B9sQoxmCpjcpyV+OnE+nUHWbuQKW4fI3/jUemf7kL2mCNmB6Y4He2NgV5asPCONDgoK4yBFAvu2kEJy7kZ4L5iUvVBjY8
// yJ2PiHWZlg/RWzO6WtSsnIMsnE2VyK+EbwUy4bt0pg2nt7A8YGoMZePMLSVRbnTO8WENPN0ZhgJSuNlZWxfgEOEpmNB5JolTZJ1zhlF26596r2JQsIKl+rclsQYgYvuDpOvmkRShlV3StvL7EnQjqgeTHX1N4f7yNMZaknvckKvTpW3
// 3Isx1qCzmPMP6rRs7hEiIG1hoNhWKD7qKQ2gGRbMEpL75vSbEhjaE9ALtKZS4Lf6kxveiOk7bayyykRzoroVoVvb0l+6jnrVk0k/JtPr5rpSODkapZnULmJ4AlWg4Sw+kYm5VH5ikWYXnfNHym2WCP0nCaCcJ7RuujQSnRlXPc+EyB2
// K1fSF8m9Cf0Ur5rpSODkapZnULmJ4AlWg4Sw+kYm5VH5ikWYXnfNHym2WCP0nCaCcJ7RuujQSnRlXPc+EyB2K1fSF8m9Cf0U=-----END CERTIFICATE-----";


void stomp_client_connect(void);
void stomp_client_subscribe(char* topic);
bool stompSend(char * buff, char* topic);
void stomeAck(const char * message);

void stomp_client_handle_message( const char *message);
void stomp_client_int( stompInfo_cfg_t stompSetup );
void stompAppStart(void);
void dataHandele(const char *rx_buffer);




// esp_err_t read_wifi_credentials(char *ssid, size_t ssid_len, char *pass, size_t pass_len) ;
// esp_err_t save_wifi_credentials(const char *ssid, const char *pass) ;
// void print_hostname();
// void set_and_print_hostname();
void wifi_connection(void);


//save wifi info
// void save_wifi_info(const char* ssid, const char* pass);
// void read_wifi_info(char* ssid, char* pass);
void enrolOngoing(void);

// parsing
void process_command(const char* buffer);
uint32_t crc32(const char *buf, size_t len);
void init_crc32_table();
void init_crc16_table();
uint16_t crc16(const char *buf, size_t len);
uint16_t hex_to_uint16(const char* hex_str);

void resizeBuffer(void);
void extractMessage(const char *buffer, char *output);




#ifdef __cplusplus
}
#endif



#endif
