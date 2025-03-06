
#ifndef NVS_H
#define NVS_H

#include "nvs_flash.h"
#include "nvs.h"

static const char *NVS_NAMESPACE = "Config";
static const char *TIME_FORMAT_KEY = "Format";

static const char *NVS_SUBCRIP_NAME_SPACE = "con";
static const char *NVS_SUBCRIP_NAME_SPACE_FORMAT_KEY = "Sub";



static const char *NVS_MUSIC_NAME_SPACE = "mcon";
static const char *NVS_MUSIC_NAME_SPACE_FORMAT_KEY = "mSub";







#ifdef __cplusplus
extern "C" {
#endif



void save_time_format(bool is_12_hour);
void saveSubscription(bool enable);

void welcomeMusic(bool enable);
bool checkMusicEnable();






#ifdef __cplusplus
}
#endif


#endif