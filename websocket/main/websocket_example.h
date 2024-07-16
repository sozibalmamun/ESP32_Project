#ifndef WEBSOCKET_EXAMPLE_H
#define WEBSOCKET_EXAMPLE_H

#include <stdio.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_websocket_client.h"
#include "esp_event.h"




#ifdef __cplusplus
extern "C" {
#endif


// #define THT     "wss://grozziieget.zjweiting.com:3091/CloudSocket-Dev/websocket/" worked

#define THT     "wss://grozziieget.zjweiting.com:3091/CloudSocket-Dev/websocket/"// testing 

// #define HOST    "wss://grozziieget.zjweiting.com/"// testing
#define HOST    "grozziieget.zjweiting.com"// done

#define PORT    3091
#define  PATH  "/CloudSocket-Dev/websocket/"

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



const char testdata[] = "-----BEGIN CERTIFICATE-----MIIGfzCCBOegAwIBAgIRANBUG6rsL5QvgWixLuHWMVcwDQYJKoZIhvcNAQEMBQAwWTELMAkGA1UEBhMCQ04xJTAjBgNVBAoTHFRydXN0QXNpYSBUZWNobm9sb2dpZXMsIEluYy4xIzAhBgNVBAMTGlRydXN0QXNpYSBSU0EgRFYgVExTIENBIEcyMB4XDTIzMTEyMDAwMDAwMFoXDTI0MTExOTIzNTk1OVowJDEiMCAGA1UEAxMZZ3JvenppaWVnZXQuemp3ZWl0aW5nLmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKiNfh80wv6vspWnJtj1aDff8/5jIy2L/SZU3hye3xyleASlFh1ARVUNWBWeoajJ1eB5AwwsXtz1CtS+ZYOYUzsnhEokMzO3Ae+3EipgUbNZGwMKTq3MJF+EMOACUBIMszxbF4VUHZMRyJ9BdvlC1be2IkfNY3CFnGyW+RO63hHxl/MR+L7qIacZQWetl/Aq6bcgzG+dH5QocXyOpd/Zi4UXHme0IuInYEqPEto4HE5W0Z9iq0XCWgAXzl5Hfo2NL5+57QZEf1dBHBhCqk6oIoxEaE3K95T8iVeW584j+vbd8iTdDIwBUOzqJbHSHVP1HZg1uFODCeCYxhouY5NRZbsCAwEAAaOCAvUwggLxMB8GA1UdIwQYMBaAFF86fBEQfgxncWHci6O1AANn9VccMB0GA1UdDgQWBBQRtVDPMqfYxoD/l92X3uGqFiKe+jAOBgNVHQ8BAf8EBAMCBaAwDAYDVR0TAQH/BAIwADAdBgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwSQYDVR0gBEIwQDA0BgsrBgEEAbIxAQICMTAlMCMGCCsGAQUFBwIBFhdodHRwczovL3NlY3RpZ28uY29tL0NQUzAIBgZngQwBAgEwfQYIKwYBBQUHAQEEcTBvMEIGCCsGAQUFBzAChjZodHRwOi8vY3J0LnRydXN0LXByb3ZpZGVyLmNuL1RydXN0QXNpYVJTQURWVExTQ0FHMi5jcnQwKQYIKwYBBQUHMAGGHWh0dHA6Ly9vY3NwLnRydXN0LXByb3ZpZGVyLmNuMCQGA1UdEQQdMBuCGWdyb3p6aWllZ2V0Lnpqd2VpdGluZy5jb20wggGABgorBgEEAdZ5AgQCBIIBcASCAWwBagB3AHb/iD8KtvuVUcJhzPWHujS0pM27KdxoQgqf5mdMWjp0AAABi+wMtZcAAAQDAEgwRgIhAP4l2Kce6/yN7E7BsKlvpKPZZmui2rG6R8lSXZZn2iUmAiEAiRlZGSZcOUUiGorK9I3NAOfOQCbOGmUsC6qq9si5oNAAdgA/F0tP1yJHWJQdZRyEvg0S7ZA3fx+FauvBvyiF7PhkbgAAAYvsDLW+AAAEAwBHMEUCIQCO1IlqWnHfHmdw9M8XpITVH5a0jgJ2PqDdulLhX6LndQIgWlf8A5RYpOxGjzAilH/AZPMXXGM2eCE0bD+/Ubaao18AdwDuzdBk1dsazsVct520zROiModGfLzs3sNRSFlGcR+1mwAAAYvsDLXEAAAEAwBIMEYCIQCTVnVdgKYWftOgTtikYkHyQFA8b4l7F48PS2pIaxWwXAIhAMLau0syoJx1iNmteK3HVx2tZn44Dzosudtr+oX/ruCBMA0GCSqGSIb3DQEBDAUAA4IBgQAcneTRxTtzc+uFOUbfNkwfoC9DYvpnMEsI1+IZs+d61+HyuLf2Pz7a7v+B9sQoxmCpjcpyV+OnE+nUHWbuQKW4fI3/jUemf7kL2mCNmB6Y4He2NgV5asPCONDgoK4yBFAvu2kEJy7kZ4L5iUvVBjY8yJ2PiHWZlg/RWzO6WtSsnIMsnE2VyK+EbwUy4bt0pg2nt7A8YGoMZePMLSVRbnTO8WENPN0ZhgJSuNlZWxfgEOEpmNB5JolTZJ1zhlF26596r2JQsIKl+rclsQYgYvuDpOvmkRShlV3StvL7EnQjqgeTHX1N4f7yNMZaknvckKvTpW33Isx1qCzmPMP6rRs7hEiIG1hoNhWKD7qKQ2gGRbMEpL75vSbEhjaE9ALtKZS4Lf6kxveiOk7bayyykRzoroVoVvb0l+6jnrVk0k/JtPr5rpSODkapZnULmJ4AlWg4Sw+kYm5VH5ikWYXnfNHym2WCP0nCaCcJ7RuujQSnRlXPc+EyB2K1fSF8m9Cf0U=-----END CERTIFICATE-----";

void stomp_client_connect(void);
void stomp_client_subscribe(char* topic);
bool stompSend(char * buff, char* topic);
void stomp_client_handle_message( const char *message);
void stomp_client_int( stompInfo_cfg_t stompSetup );
static void websocket_app_start(void);



#ifdef __cplusplus
}
#endif



#endif
