#include "who_human_face_recognition.hpp"

#include "esp_log.h"
#include "dl_image.hpp"
#include "fb_gfx.h"

#include "human_face_detect_msr01.hpp"
#include "human_face_detect_mnp01.hpp"
#include "face_recognition_tool.hpp"

#include "who_button.h"


#if CONFIG_MFN_V1
#if CONFIG_S8
#include "face_recognition_112_v1_s8.hpp"
#elif CONFIG_S16
#include "face_recognition_112_v1_s16.hpp"
#endif
#endif

#include "who_ai_utils.hpp"
#include "CloudDataHandle.h"

extern key_state_t key_state;


uint8_t boxPosition[5];
static uint16_t StopMultipleAttaneId=0;


using namespace std;
using namespace dl;

static const char *TAG = "human_face_recognition";

volatile uint8_t CmdEvent;
char personName[20];
volatile uint16_t personId;


//---------------time flag--------------------
extern volatile uint8_t sleepEnable;
extern TickType_t sleepTimeOut; 
TickType_t TimeOut;
TickType_t erolTimeOut;

//---------------------------------------



static QueueHandle_t xQueueFrameI = NULL;
static QueueHandle_t xQueueEvent = NULL;
static QueueHandle_t xQueueFrameO = NULL;
static QueueHandle_t xQueueResult = NULL;

static QueueHandle_t xQueueCloud = NULL;




static recognizer_state_t gEvent = DETECT;
static bool gReturnFB = true;
static face_info_t recognize_result;

SemaphoreHandle_t xMutex;

typedef enum
{
    SHOW_STATE_IDLE,
    SHOW_STATE_DELETE,
    SHOW_STATE_RECOGNIZE,
    SHOW_STATE_ENROLL,
    SHOW_DUPLICATE,
    INVALID
} show_state_t;

#define RGB565_MASK_RED 0xF800
#define RGB565_MASK_GREEN 0x07E0
#define RGB565_MASK_BLUE 0x001F
#define FRAME_DELAY_NUM 16


static void rgb_print(camera_fb_t *fb, uint32_t color, const char *str)
{
    // fb_gfx_print(fb, (fb->width - (strlen(str) * 14)) / 2, 10, color, str);// old

        fb_gfx_print(fb, (fb->width - (strlen(str) * 14)) / 2, 200, color, str);// edited

}

static int rgb_printf(camera_fb_t *fb, uint32_t color, const char *format, ...)
{
    char loc_buf[64];
    char *temp = loc_buf;
    int len;
    va_list arg;
    va_list copy;
    va_start(arg, format);
    va_copy(copy, arg);
    len = vsnprintf(loc_buf, sizeof(loc_buf), format, arg);
    va_end(copy);
    if (len >= sizeof(loc_buf))
    {
        temp = (char *)malloc(len + 1);
        if (temp == NULL)
        {
            return 0;
        }
    }
    vsnprintf(temp, len + 1, format, arg);
    va_end(arg);
    rgb_print(fb, color, temp);
    if (len > 64)
    {
        free(temp);
    }
    return len;
}
bool copy_rectangle(const camera_fb_t *src, imageData_t **dst, int x_start, int x_end, int y_start, int y_end) {
    // Validate rectangle dimensions
    if (x_start >= x_end || y_start >= y_end) {
        printf("Invalid rectangle dimensions\n");
        return false;
    }

    // Allocate memory for the destination imageData_t structure
    *dst = (imageData_t *)heap_caps_malloc(sizeof(imageData_t), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
    if (*dst == NULL) {
        printf("Memory allocation for cropFrame structure failed\n");
        return false;
    }

    // Calculate rectangle dimensions
    int rect_width = x_end - x_start;
    int rect_height = y_end - y_start;
    int bytes_per_pixel = 2; // Assuming RGB565 format
    (*dst)->width = rect_width;
    (*dst)->height = rect_height;
    (*dst)->len = rect_width * rect_height * bytes_per_pixel;

    // Validate the calculated size
    if ((*dst)->len <= 0 || (*dst)->len > src->len) {
        printf("Invalid calculated len: %d\n", (*dst)->len);
        // heap_caps_free(*dst);
        *dst = NULL;
        return false;
    }

    // Allocate memory for the destination buffer
    (*dst)->buf = (uint8_t *)heap_caps_malloc((*dst)->len, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
    if (!(*dst)->buf) {
        printf("Memory allocation for cropFrame buffer failed\n");
        heap_caps_free(*dst);
        *dst = NULL;
        return false;
    }

    // Copy the rectangle area from the source to the destination
    for (int y = y_start; y < y_end; y++) {
        for (int x = x_start; x < x_end; x++) {
            int src_index = (y * src->width + x) * bytes_per_pixel;
            int dst_index = ((y - y_start) * rect_width + (x - x_start)) * bytes_per_pixel;
            
            // Perform the copy safely
            (*dst)->buf[dst_index] = src->buf[src_index];
            (*dst)->buf[dst_index + 1] = src->buf[src_index + 1];
        }
    }

    return true;
}




static void task_process_handler(void *arg)
{

    camera_fb_t *frame = NULL;


    HumanFaceDetectMSR01 detector(0.3F, 0.3F, 10, 0.3F);
    HumanFaceDetectMNP01 detector2(0.4F, 0.3F, 10);

#if CONFIG_MFN_V1
#if CONFIG_S8
    FaceRecognition112V1S8 *recognizer = new FaceRecognition112V1S8();
#elif CONFIG_S16
    FaceRecognition112V1S16 *recognizer = new FaceRecognition112V1S16();
#endif
#endif
    show_state_t frame_show_state = SHOW_STATE_IDLE;
    recognizer_state_t _gEvent;
    recognizer->set_partition(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "fr");
    int partition_result = recognizer->set_ids_from_flash();

    while (true)
    {
        xSemaphoreTake(xMutex, portMAX_DELAY);
        _gEvent = gEvent;
        gEvent = DETECT;
        xSemaphoreGive(xMutex);

        if (_gEvent)
        {
            bool is_detected = false;

            if (xQueueReceive(xQueueFrameI, &frame, portMAX_DELAY))
            {



                std::list<dl::detect::result_t> &detect_candidates = detector.infer((uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3});
                std::list<dl::detect::result_t> &detect_results = detector2.infer((uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3}, detect_candidates);

                if(_gEvent==DELETE){// deleting person 

                    vTaskDelay(10);
                    if(personId!=0){ 

                        is_detected= true;
                        key_state= KEY_IDLE;
                        ESP_LOGW("DELETE", "ID: %d",personId );
                    }
                    
                }else if(_gEvent==DETECT){

                    if (detect_results.size() == 1){
                        is_detected = true;
                        _gEvent=RECOGNIZE;
                                
                    }

                }
                else if(_gEvent==ENROLING){

                    if (detect_results.size() == 1){

                        if(xTaskGetTickCount()>TimeOut+TIMEOUT_3000_MS){

                            is_detected = true;
                            _gEvent=ENROLL;
                            key_state= KEY_IDLE;

                        }
      
                    }else {

                        TimeOut= xTaskGetTickCount();
                        if (xTaskGetTickCount()-erolTimeOut> TIMEOUT_15_S ){
                            ESP_LOGI("ENROL", "TIME OUT\n");
                            CmdEvent = ENROLMENT_TIMEOUT;
                            key_state= KEY_IDLE;
                            vTaskDelay(10);

                        } 
                        rgb_printf(frame, RGB565_MASK_GREEN, "Start Enroling");// debug due to display name

                    }


                }
                
                if (is_detected)
                {
                    switch (_gEvent)
                    {
                    case ENROLL:{
                        CPUBgflag=1;
                        // vTaskDelay(10);
                        // duplicate 
                        recognize_result = recognizer->recognize((uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3}, detect_results.front().keypoint);
                        //print_detection_result(detect_results);
                        if (recognize_result.id > 0){
                        //rgb_printf(frame, RGB565_MASK_RED, "Duplicate Face%s","!");// debug due to display name
                        CmdEvent=DUPLICATE;// 3 FOR DUPLICATE
                        frame_show_state = SHOW_DUPLICATE;
                        break;
                        }
                        // duplicat end


                        //----------------------------working with image--------------------------
                        imageData_t *cropFrame = NULL;

                        print_detection_result(detect_candidates);
                        draw_detection_result((uint16_t *)frame->buf, frame->height, frame->width, detect_candidates);
                        if(!copy_rectangle(frame,&cropFrame, boxPosition[0],boxPosition[2], boxPosition[1], boxPosition[3]))
                        {
                            frame_show_state = INVALID;
                            heap_caps_free(cropFrame);
                            break;
                        }
                        //--------------------------------------------------------------------------

                        // recognizer->enroll_id((uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3}, detect_results.front().keypoint, "", true);
                        recognizer->enroll_id((uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3}, detect_results.front().keypoint, personName, true);// due to add name
                        // ESP_LOGW("ENROLL", "ID %d is enrolled", recognizer->get_enrolled_ids().back().id);

                        //-------------------------pass value to struc via task----------------------------
                       // printf("\nCopy Image Info  L:%3d w:%3d h:%3d", cropFrame->len, cropFrame->width, cropFrame->height);

                        cropFrame->id= recognizer->get_enrolled_ids().back().id;
                        cropFrame->Name= personName;


                        if (xQueueCloud) {
                            // printf("Sending cropFrame to xQueueCloud...\n");
                            xQueueSend(xQueueCloud, &cropFrame, portMAX_DELAY);
                        } else {
                            printf("xQueueCloud is NULL, cannot send cropFrame.\n");
                        }

                        //---------------------------------------------------------------------------------

                        frame_show_state = SHOW_STATE_ENROLL;
                        break;
                    }
                    case RECOGNIZE:{
                        recognize_result = recognizer->recognize((uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3}, detect_results.front().keypoint);
                        // print_detection_result(detect_results);
                        if (recognize_result.id > 0){

                            CPUBgflag=1;
                            // ESP_LOGI("RECOGNIZE", "Similarity: %f, Match Name: %s", recognize_result.similarity, recognize_result.name.c_str());

                            if(xTaskGetTickCount()>TimeOut+TIMEOUT_5000_MS)StopMultipleAttaneId=0;

                            if(StopMultipleAttaneId!=recognize_result.id){

                                StopMultipleAttaneId=recognize_result.id;
                                //--------------------------------here save the log file here----------------------------------
                                time_library_time_t current_time;
                                get_time(&current_time, 0);
                                uint8_t tempTimeFrame[6];
                                memset(tempTimeFrame,0,sizeof(tempTimeFrame));
                                tempTimeFrame[0] = current_time.year-2000;
                                tempTimeFrame[1] = current_time.month;
                                tempTimeFrame[2] = current_time.day;
                                tempTimeFrame[3] = current_time.hour;
                                tempTimeFrame[4] = current_time.minute;
                                tempTimeFrame[5] = current_time.second;
                                write_log_attendance(recognize_result.id, tempTimeFrame);
                                TimeOut=xTaskGetTickCount();

                                //----------------------------------------------------------------------------------------------
                            }
                            CPUBgflag=0;

                        }
                        // ESP_LOGE("RECOGNIZE", "Similarity: %f, Match ID: %d", recognize_result.similarity, recognize_result.id);
                        frame_show_state = SHOW_STATE_RECOGNIZE;
                        break;
                    }
                    case DELETE:
                        // vTaskDelay(10);
                        // recognizer->delete_id(true);
                        if(recognizer->delete_id(personId,true)== -1 ){// invalide id if "-1"// custom id delete logic

                            personId=0;// deafalt for test
                            ESP_LOGE("DELETE", "Invalided ID: %d", personId);
                            CmdEvent=ID_INVALID; // delete done 

                        }else{

                            personId=0;// deafalt for test
                            CmdEvent=DELETED;// delete done
                            ESP_LOGE("DELETE", "IDs left %d", personId);

                        }
                        // ESP_LOGE("DELETE", "% d IDs left", recognizer->get_enrolled_id_num());

                        frame_show_state = SHOW_STATE_DELETE;
                        break;

                    default:
                        break;
                    }
                }

                if (frame_show_state != SHOW_STATE_IDLE)
                {
                    static int frame_count = 0;
                    switch (frame_show_state)
                    {
                    case SHOW_STATE_DELETE:
                        // rgb_printf(frame, RGB565_MASK_RED, "%d IDs left", recognizer->get_enrolled_id_num());   #define ID_INVALID      0X06
                        if(CmdEvent==DELETED){

                            rgb_printf(frame, RGB565_MASK_RED, "Deleted Id: %d", personId);

                        }else //rgb_printf(frame, RGB565_MASK_RED, "%d IDs invalided", personId);

                        break;

                    case SHOW_STATE_RECOGNIZE:
                        if (recognize_result.id > 0){
                            // rgb_printf(frame, RGB565_MASK_GREEN, "ID %d", recognize_result.id);
                            rgb_printf(frame, RGB565_MASK_GREEN, "Welcome %s",recognize_result.name.c_str());// debug due to display name
                        }else{
                            rgb_print(frame, RGB565_MASK_RED, "Unregister");
                            ESP_LOGI(TAG,"Not Recognize");
                        }
                        break;

                    case SHOW_STATE_ENROLL:{
                        CmdEvent=ENROLED;// 2 means enrol done

                        rgb_printf(frame, RGB565_MASK_BLUE, "Enroll: ID %d", recognizer->get_enrolled_ids().back().id); 
                        personId=recognizer->get_enrolled_ids().back().id;
                        // rgb_printf(frame, RGB565_MASK_BLUE, "Enroll: ID %d", recognizer->get_enrolled_ids().back().id);
                        break;
                    }
                    case SHOW_DUPLICATE:{
                        rgb_printf(frame, RGB565_MASK_RED, "Duplicate Face%s","!");//   
                        // vTaskDelay(10);

                        break;
                    }
                    case INVALID:

                        erolTimeOut= xTaskGetTickCount();// incrise 15000ms enrolment time
                        rgb_printf(frame, RGB565_MASK_RED, "Aline Face");// at invalid face
                        key_state= KEY_SHORT_PRESS;// back to enroling

                        break;
                    default:
                        break;
                    }

                    if (++frame_count > FRAME_DELAY_NUM)
                    {
                        frame_count = 0;
                        frame_show_state = SHOW_STATE_IDLE;
                    }else if( frame_show_state==SHOW_STATE_RECOGNIZE){
                        frame_count = 0;
                        frame_show_state = SHOW_STATE_IDLE;

                    }
                }

                if (detect_results.size())
                {
#if !CONFIG_IDF_TARGET_ESP32S3
                    // print_detection_result(detect_results);
#endif
                    draw_detection_result((uint16_t *)frame->buf, frame->height, frame->width, detect_results);
                }
            }

            if (xQueueFrameO)
            {

                xQueueSend(xQueueFrameO, &frame, portMAX_DELAY);
            }
            else if (gReturnFB)
            {
                esp_camera_fb_return(frame);
            }
            else
            {
                free(frame);
            }

            if (xQueueResult && is_detected)
            {
                xQueueSend(xQueueResult, &recognize_result, portMAX_DELAY);
            }
        }
    }
}


// static void task_process_handler(void *arg)
// {
//     camera_fb_t *frame = NULL;

//     HumanFaceDetectMSR01 detector(0.3F, 0.3F, 10, 0.3F);
//     HumanFaceDetectMNP01 detector2(0.4F, 0.3F, 10);

// #if CONFIG_MFN_V1
// #if CONFIG_S8
//     FaceRecognition112V1S8 *recognizer = new FaceRecognition112V1S8();
// #elif CONFIG_S16
//     FaceRecognition112V1S16 *recognizer = new FaceRecognition112V1S16();
// #endif
// #endif
//     show_state_t frame_show_state = SHOW_STATE_IDLE;
//     recognizer_state_t _gEvent;
//     recognizer->set_partition(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "fr");
//     int partition_result = recognizer->set_ids_from_flash();

//     while (true)
//     {
//         xSemaphoreTake(xMutex, portMAX_DELAY);
//         _gEvent = gEvent;
//         gEvent = DETECT;
//         xSemaphoreGive(xMutex);

//         if (_gEvent)
//         {
//             bool is_detected = false;

//             if (xQueueReceive(xQueueFrameI, &frame, portMAX_DELAY))
//             {
//                 std::list<dl::detect::result_t> &detect_candidates = detector.infer((uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3});
//                 std::list<dl::detect::result_t> &detect_results = detector2.infer((uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3}, detect_candidates);
               
               
//                 print_detection_result(detect_candidates);
//                 draw_detection_result((uint16_t *)frame->buf, frame->height, frame->width, detect_candidates);
//                 // draw_detection_result((uint16_t *)frame->buf, frame->height, frame->width, detect_results);
//                 if (detect_candidates.size() == 1){
//                     editImage(&frame);
//                 }
//             }

//             if (xQueueFrameO)
//             {

//                 xQueueSend(xQueueFrameO, &frame, portMAX_DELAY);
//             }
//             else if (gReturnFB)
//             {
//                 esp_camera_fb_return(frame);
//             }
//             else
//             {
//                 free(frame);
//             }

//             if (xQueueResult && is_detected)
//             {
//                 xQueueSend(xQueueResult, &recognize_result, portMAX_DELAY);
//             }
//         }
//     }
// }



static void task_event_handler(void *arg)
{
    recognizer_state_t _gEvent;
    while (true)
    {
        xQueueReceive(xQueueEvent, &(_gEvent), portMAX_DELAY);
        xSemaphoreTake(xMutex, portMAX_DELAY);
        gEvent = _gEvent;
        xSemaphoreGive(xMutex);
    }
}

void register_human_face_recognition(const QueueHandle_t frame_i,
                                     const QueueHandle_t event,
                                     const QueueHandle_t result,
                                     const QueueHandle_t frame_o,

                                     const QueueHandle_t cloud,

                                     const bool camera_fb_return)
{
    xQueueFrameI = frame_i;
    xQueueFrameO = frame_o;
    xQueueEvent = event;
    xQueueResult = result;

    xQueueCloud= cloud;


    gReturnFB = camera_fb_return;
    xMutex = xSemaphoreCreateMutex();

    xTaskCreatePinnedToCore(task_process_handler, TAG, 4 * 1024, NULL, 5, NULL, 1);
        // xTaskCreatePinnedToCore(task_process_handler, TAG, 4 * 1024, NULL, 5, NULL, 1);

    if (xQueueEvent)
        xTaskCreatePinnedToCore(task_event_handler, TAG, 4 * 1024, NULL, 3, NULL, 1);
}
