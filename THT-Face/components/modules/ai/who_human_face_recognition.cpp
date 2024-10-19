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

static const char *TAG = "recognition_task";

volatile uint8_t CmdEvent =0;
char personName[20];
uint16_t personId;



//---------------time flag--------------------
extern  uint8_t sleepEnable;
extern TickType_t sleepTimeOut; 
TickType_t TimeOut,faceDetectTimeOut;
TickType_t enrolTimeOut;

//---------------------------------------



static QueueHandle_t xQueueFrameI = NULL;
static QueueHandle_t xQueueEvent = NULL;
static QueueHandle_t xQueueFrameO = NULL;
static QueueHandle_t xQueueResult = NULL;


//------------------------------------------------------------------------------------
static QueueHandle_t xQueueCloud = NULL;
extern TaskHandle_t detectionFaceProcesingTaskHandler; // Handle for the stompSenderTask

//---------------------------------------------------------------------------------------


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
    SHOW_STATE_SYNC,
    SHOW_DUPLICATE_ENROLL,
    SHOW_DUPLICATE_SYNC,
    SHOW_ALINE
} show_state_t;

#define RGB565_MASK_RED 0xF800
#define RGB565_MASK_GREEN 0x14ff
#define RGB565_MASK_BLUE 0x437e
#define FRAME_DELAY_NUM 16 


static void rgb_print(camera_fb_t *fb, uint32_t color, const char *str)
{
   
    // int text_width = strlen(str) * 14; // Assuming each character is about 14 pixels wide
    // int text_height = 20;              // Assuming the text height is about 20 pixels
    // int rect_width = text_width + 6;  // Adding padding around the text
    // int rect_height = text_height + 4;

    // // Calculate the rectangle's position to center the text in the framebuffer
    // int rect_x = (fb->width - rect_width) / 2;
    // int rect_y = 200 - 2; // Adjust the Y offset for the rectangle

    // // Set the rectangle with the chosen background color
    // fillRect(fb, rect_x, rect_y, rect_width, rect_height, color);

    // fillRoundedRect(fb, 50, 50, 100, 150, 20, 0x07E0);  // Draws a green rounded rectangle

    fillRoundedRect(fb, (fb->width - (strlen(str) * 14)) / 2 -6, 200-3, (strlen(str) * 14)+12 , 28, 3, color);  // Draws a filled rounded rectangle

    fb_gfx_print(fb, (fb->width - (strlen(str) * 14)) / 2, 200, 0xffff, str);// edited

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

bool copy_rectangle(const camera_fb_t *src, imageData_t **dst, int16_t x_start, int16_t x_end, int16_t y_start, int16_t y_end) {
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
    uint8_t rect_width = x_end - x_start;
    uint8_t rect_height = y_end - y_start;

    (*dst)->width = rect_width;
    (*dst)->height = rect_height;



    (*dst)->len = rect_width * rect_height * bytes_per_pixel;

    // Validate the calculated size
    if ((*dst)->len <= 0 || (*dst)->len > src->len) {
        // printf("Invalid calculated len: %d\n", (*dst)->len);
        // heap_caps_free(*dst);
        *dst = NULL;
        return false;
    }

    // Allocate memory for the destination buffer
    (*dst)->buf = (uint8_t *)heap_caps_malloc((*dst)->len, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
    if (!(*dst)->buf) {
        // printf("Memory allocation for cropFrame buffer failed\n");
        heap_caps_free(*dst);
        *dst = NULL;
        return false;
    }

    // Copy the rectangle area from the source to the destination
    for (int16_t y = y_start; y < y_end; y++) {
        for (int16_t x = x_start; x < x_end; x++) {
            uint32_t src_index = (y * src->width + x) * bytes_per_pixel;
            uint32_t dst_index = ((y - y_start) * rect_width + (x - x_start)) * bytes_per_pixel;
            
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
    camera_fb_t *moveDetection = NULL;


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

            if (xQueueReceive(xQueueFrameI, &(frame), portMAX_DELAY))
            {

                if(sleepEnable==WAKEUP){

                    imageData_t *enrolFrame = NULL;

                    if(_gEvent==SYNCING){

                        CPUBgflag=1;

                        if (!syncFace(frame, &enrolFrame)) {//frame
                            CmdEvent = SYNC_ERROR;
                            key_state= KEY_IDLE;
                            vTaskDelay(10);       
                            ESP_LOGW("sync", "directory emty");
                        }
                        CPUBgflag=0;

                        // ESP_LOGI("display_faces", "Person ID: %d, Name: %s, Image w: %d h: %d", enrolFrame->id, enrolFrame->Name, enrolFrame->width, enrolFrame->height);
                    }
                    std::list<dl::detect::result_t> &detect_candidates = detector.infer((uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3});
                    std::list<dl::detect::result_t> &detect_results = detector2.infer((uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3}, detect_candidates);

                    if(_gEvent==DELETE){// deleting person 

                        vTaskDelay(10);
                        if(personId!=0){ 

                            is_detected= true;
                            key_state= KEY_IDLE;
                            ESP_LOGW("DELETE", "ID: %d",personId );
                        }
                        
                    }else if(_gEvent==ENROLING){

                        if (detect_results.size() == 1){

                            if(xTaskGetTickCount()>TimeOut+TIMEOUT_3000_MS){// enter enroll case


                                percentage=0;
                                is_detected = true;
                                _gEvent=ENROLL;
                                key_state= KEY_IDLE;


                            }else { // loading

                                uint16_t subvalue =(TIMEOUT_3000_MS-((TimeOut + TIMEOUT_3000_MS)- xTaskGetTickCount())) ;

                                float percentage_float = (subvalue /(float)TIMEOUT_3000_MS) * 100;
                                percentage = (int8_t)percentage_float;
                                // rgb_printf(frame, RGB565_MASK_BLUE, "Loading %d%s",(int)percentage,"%");

                                // fb_gfx_fillRect(frame, 100, 50, 50, 3, RGB565_MASK_BLUE);
                                // void fillRect(camera_fb_t *fb, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color);

                                // fb_gfx_fillRect(frame, 0, 235, 320, 1, 0xFFFF);
                                // fb_gfx_fillRect(frame, 0, 240, 320, 1, 0xFFFF);
                                fillRect(frame, 0, 235, 320, 5, 0xFFFF);
                                fillRect(frame, 0, 235, (uint16_t)(percentage*3.2), 5, RGB565_MASK_BLUE);

                            }

                        }else {

                            TimeOut= xTaskGetTickCount();
                            if (xTaskGetTickCount()-enrolTimeOut> TIMEOUT_15_S ){// enroll time out 
                                CmdEvent = ENROLMENT_TIMEOUT;
                                key_state= KEY_IDLE;
                                vTaskDelay(10);

                            } 
                            rgb_printf(frame, RGB565_MASK_BLUE, "Start Enroling");// debug due to display name
                            sleepTimeOut = TimeOut;
                            sleepEnable=WAKEUP;// sleep out when enroll event is genareted
                            percentage=0;


                        }

                    }else if(_gEvent==DETECT){


                        if (detect_results.size() == 1){


                            if(xTaskGetTickCount()>faceDetectTimeOut+TIMEOUT_150_MS){ 
                                
                                // faceDetectTimeOut= xTaskGetTickCount();
                                is_detected = true;

                                if(_gEvent!=ENROLING){
                                    _gEvent=RECOGNIZE;// enroling is the 1st priority
                                }

                            }else {

                                sleepTimeOut = xTaskGetTickCount();// imediate wake if display in sleep mode
                                sleepEnable=WAKEUP;

                            }                                  
                                    
                        }else{ faceDetectTimeOut= xTaskGetTickCount(); }

                    }else if(_gEvent==SYNCING){

                        is_detected = true;


                        if (xTaskGetTickCount()-enrolTimeOut> TIMEOUT_5000_MS ){
                            CmdEvent = SYNC_ERROR;
                            key_state= KEY_IDLE;
                            delete_face_data(enrolFrame->id,SYNC_DIR);               
                            vTaskDelay(10);


                        } 

                        if (detect_results.size() == 1){

                            if(xTaskGetTickCount()>faceDetectTimeOut+TIMEOUT_150_MS){ 

                                faceDetectTimeOut= xTaskGetTickCount();
                                is_detected = true;
                                _gEvent=SYNC;
                                key_state= KEY_IDLE;
                                ESP_LOGE("sync", "In syncing");
                            }

                        }else{

                            if (enrolFrame != NULL) {
                                if (enrolFrame->buf != NULL) {
                                    heap_caps_free(enrolFrame->buf);  // Free the image data buffer
                                }
                                if (enrolFrame->Name != NULL) {
                                    free(enrolFrame->Name);  // Free the dynamically allocated name string
                                }
                                free(enrolFrame);  // Finally, free the structure itself
                                enrolFrame = NULL; // Set the pointer to NULL to avoid dangling references
                            } 
                        }

                    }else if(_gEvent==DUMP){

                        // ESP_LOGW("Enrolled", "ID: %d",recognizer->get_enrolled_id_num());
                        key_state= KEY_IDLE;

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

                            // print_detection_result(detect_results);
                            if (recognize_result.id > 0){
                            //rgb_printf(frame, RGB565_MASK_RED, "Duplicate Face%s","!");// debug due to display name
                            CmdEvent=DUPLICATE;// 3 FOR DUPLICATE
                            frame_show_state = SHOW_DUPLICATE_ENROLL;
                            break;
                            }
                            // duplicat end


                            //----------------------------working with image--------------------------

                            // imageData_t *cropFrame = NULL;

                            print_detection_result(detect_candidates);
                            if(!copy_rectangle(frame,&enrolFrame, boxPosition[0],boxPosition[2], boxPosition[1], boxPosition[3]))
                            {
                                CPUBgflag=0;
                                frame_show_state = SHOW_ALINE;
                                heap_caps_free(enrolFrame);
                                break;
                            }
                            //--------------------------------------------------------------------------

                            draw_detection_result((uint16_t *)frame->buf, frame->height, frame->width, detect_candidates);

                            recognizer->enroll_id((uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3}, detect_results.front().keypoint, personName, true);// due to add name
                            enrolFrame->id = recognizer->get_enrolled_ids().back().id;
                            enrolFrame->Name = personName;

                            //-------------------------pass value to struc via task----------------------------
                            if(detectionFaceProcesingTaskHandler==NULL){

                                xQueueCloud = xQueueCreate(3, sizeof(int *));
                                facedataHandle(xQueueCloud);// core 0

                            }

                            if (xQueueCloud) {
                                printf("Sending enrolFrame to xQueueCloud...\n");
                                xQueueSend(xQueueCloud, &enrolFrame, portMAX_DELAY);

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
                                // ESP_LOGE("DELETE", "Invalided ID: %d", personId);
                                CmdEvent = ID_INVALID; // delete done 

                                break;

                            }else{

                                personId=0;// deafalt for test
                                CmdEvent=DELETED;// delete done
                                // ESP_LOGE("DELETE", "Person left %s", personName);

                            }
                            // ESP_LOGE("DELETE", "% d IDs left", recognizer->get_enrolled_id_num());

                            frame_show_state = SHOW_STATE_DELETE;
                            break;

                        case SYNC: {
                            // ESP_LOGE("sync", "end syncing");
                            recognize_result = recognizer->recognize((uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3}, detect_results.front().keypoint);

                            if (recognize_result.id > 0){
                            CmdEvent=SYNC_DUPLICATE;// 3 FOR DUPLICATE
                            frame_show_state = SHOW_DUPLICATE_SYNC;
                            delete_face_data(enrolFrame->id,SYNC_DIR);               

                            break;
                            }

                            recognizer->enroll_id((uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3}, detect_results.front().keypoint, enrolFrame->Name, true);// due to add name

                            memset(personName, 0, sizeof(personName));
                            strncpy(personName, enrolFrame->Name, sizeof(personName) - 1);
                            personName[sizeof(personName) - 1] = '\0';
                            delete_face_data(enrolFrame->id,SYNC_DIR);               

                            if (enrolFrame != NULL) {
                                if (enrolFrame->buf != NULL) {
                                    heap_caps_free(enrolFrame->buf);  // Free the image data buffer
                                }
                                if (enrolFrame->Name != NULL) {
                                    free(enrolFrame->Name);  // Free the dynamically allocated name string
                                }
                                free(enrolFrame);  // Finally, free the structure itself
                                enrolFrame = NULL; // Set the pointer to NULL to avoid dangling references
                            }

                            frame_show_state = SHOW_STATE_SYNC;

                            break;
                        }

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

                            // ESP_LOGI(TAG,"Deleted");
                            rgb_printf(frame, RGB565_MASK_RED, "Deleted %s", personName);

                            break;

                        case SHOW_STATE_RECOGNIZE:
                            if (recognize_result.id > 0){
                                // rgb_printf(frame, RGB565_MASK_GREEN, "ID %d", recognize_result.id);
                                rgb_printf(frame, RGB565_MASK_GREEN, "%s",recognize_result.name.c_str());// debug due to display name
                            }else{
                                rgb_print(frame, RGB565_MASK_RED, "Unregister");
                                // ESP_LOGI(TAG,"Not Recognize");
                            }
                            break;

                        case SHOW_STATE_ENROLL:{
                            CmdEvent=ENROLED;// 2 means enrol done

                            rgb_printf(frame, RGB565_MASK_BLUE, "Welcome %s", personName); 
                            personId=recognizer->get_enrolled_ids().back().id;
                            // rgb_printf(frame, RGB565_MASK_BLUE, "Enroll: ID %d", recognizer->get_enrolled_ids().back().id);
                            break;
                        }
                        case SHOW_DUPLICATE_ENROLL:{

                            rgb_printf(frame, RGB565_MASK_RED, "Duplicate Enrol%s","!"); 
                            break;
                        }
                        case SHOW_DUPLICATE_SYNC:

                            rgb_printf(frame, RGB565_MASK_RED, "Duplicate Sync%s","!");

                            break;
                        case SHOW_STATE_SYNC:

                            CmdEvent=SYNC_DONE;// 2 means enrol done
                            rgb_printf(frame, RGB565_MASK_BLUE, "Welcome %s", personName); 
                            personId=recognizer->get_enrolled_ids().back().id;


                            break;
                        case SHOW_ALINE:

                            enrolTimeOut= xTaskGetTickCount();// incrise 15000ms enrolment time
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
                        }
                        else if( frame_show_state==SHOW_STATE_RECOGNIZE){
                            if(frame_count>2)
                                frame_count = 0;

                            frame_show_state = SHOW_STATE_IDLE;
                        }
                    }

                    if (detect_results.size())
                    {

                        draw_detection_result((uint16_t *)frame->buf, frame->height, frame->width, detect_results);
                    }
                }// wake up section normal oparetion 
            }

            bool motion = false;

            if(sleepEnable==SLEEP){ 

                if (xQueueReceive(xQueueFrameI, &(moveDetection), portMAX_DELAY))// motion detection 
                {  

                    //------------------------motion detection -----------------------------
                    uint32_t moving_point_number = dl::image::get_moving_point_number((uint16_t *)frame->buf, (uint16_t *)moveDetection->buf, frame->height, frame->width, 8, 15);
                    if (moving_point_number > 50)
                    {
                        // ESP_LOGE(TAG, " Motion detected!");
                        motion =true;

                    }
                    // end---------------------------------------------------------------------

                }// motion end

            }
                
            if (xQueueFrameO)
            {

                xQueueSend(xQueueFrameO, &frame, portMAX_DELAY);
                //------------------------motion detection -----------------------------
                if(sleepEnable==SLEEP){
                    xQueueSend(xQueueFrameO, &moveDetection, portMAX_DELAY);
                    // ESP_LOGI(TAG,"xQueueFrameO");
                    if(motion){
                        sleepTimeOut = xTaskGetTickCount();
                        sleepEnable=WAKEUP;
                    }
                }


            }
            else if (gReturnFB)
            {
                esp_camera_fb_return(frame);
                //------------------------motion detection -----------------------------
                if(motion){

                    // ESP_LOGI(TAG,"esp_camera_fb_return");
                    esp_camera_fb_return(moveDetection);

                }

            }
            else
            {
                free(frame);
                //------------------------motion detection -----------------------------
                if(motion){

                    // ESP_LOGI(TAG,"free moveDetection");
                    free(moveDetection);

                }

            }

            if (xQueueResult && is_detected)
            {
                xQueueSend(xQueueResult, &recognize_result, portMAX_DELAY);


            }
            if(xQueueResult){

                if(motion){

                    ESP_LOGI(TAG,"xQueueResult");
                    xQueueSend(xQueueResult, &motion, portMAX_DELAY);

                }


            }

        }
    }
}




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

    xTaskCreatePinnedToCore(task_process_handler, TAG, 5 * 1024, NULL, 5, NULL, 0);
        // xTaskCreatePinnedToCore(task_process_handler, TAG, 4 * 1024, NULL, 5, NULL, 1);

    if (xQueueEvent)
        xTaskCreatePinnedToCore(task_event_handler, TAG, 1 * 1024, NULL,5, NULL, 1);
}
