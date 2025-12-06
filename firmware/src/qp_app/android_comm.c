/*
 * Android Communication Active Object
 * FRDM-MCXN947 4WD Robot
 * 
 * Handles UART communication with Google Pixel 10 Pro.
 * Receives GPS, accelerometer, and vision data.
 * Sends robot status and telemetry.
 */

#include "robot.h"
#include <string.h>
#include <stdlib.h>

/*==========================================================================*/
/* Android Communication Active Object */
/*==========================================================================*/

#define RX_BUFFER_SIZE  256
#define TX_BUFFER_SIZE  256

typedef struct {
    QActive super;
    
    /* Receive buffer */
    char rx_buffer[RX_BUFFER_SIZE];
    uint16_t rx_index;
    
    /* Transmit buffer */
    char tx_buffer[TX_BUFFER_SIZE];
    
    /* Communication state */
    bool connected;
    uint32_t last_heartbeat;
    uint32_t rx_count;
    uint32_t tx_count;
    
    /* Time event for periodic operations */
    QTimeEvt timeEvt;
} AndroidComm;

/* Message types from Pixel 10 Pro */
#define MSG_GPS     "$GPS"
#define MSG_IMU     "$IMU"
#define MSG_VIS     "$VIS"
#define MSG_CMD     "$CMD"

/* Message types to Pixel 10 Pro */
#define MSG_STS     "$STS"
#define MSG_POS     "$POS"
#define MSG_ACK     "$ACK"

/* Local instance */
static AndroidComm l_androidComm;

/* Global pointer */
QActive * const AO_AndroidComm = &l_androidComm.super;

/*==========================================================================*/
/* State Handlers */
/*==========================================================================*/

static QState AndroidComm_initial(AndroidComm * const me, QEvt const * const e);
static QState AndroidComm_disconnected(AndroidComm * const me, QEvt const * const e);
static QState AndroidComm_connected(AndroidComm * const me, QEvt const * const e);

/*==========================================================================*/
/* Helper Functions */
/*==========================================================================*/

static void AndroidComm_sendMessage(AndroidComm * const me, const char *msg) {
    const char *p = msg;
    while (*p) {
        BSP_uart_putchar(1, *p++);
    }
    BSP_uart_putchar(1, '\r');
    BSP_uart_putchar(1, '\n');
    me->tx_count++;
}

static void AndroidComm_parseGPS(AndroidComm * const me, const char *data) {
    /* Parse: $GPS,lat,lon,alt,speed,heading,sats,fix* */
    GPSEvt *evt = Q_NEW(GPSEvt, SIG_GPS_UPDATE);
    if (evt != (GPSEvt *)0) {
        /* Simple parsing - in real implementation use proper parser */
        char *ptr = (char *)data;
        char *token;
        int field = 0;
        
        /* Skip message type */
        ptr = strchr(ptr, ',');
        if (ptr) ptr++;
        
        while (ptr && field < 7) {
            token = ptr;
            ptr = strchr(ptr, ',');
            if (ptr) *ptr++ = '\0';
            
            switch (field) {
                case 0: evt->latitude = atol(token); break;
                case 1: evt->longitude = atol(token); break;
                case 2: evt->altitude = atol(token); break;
                case 3: evt->speed = (uint16_t)atoi(token); break;
                case 4: evt->heading = (uint16_t)atoi(token); break;
                case 5: evt->satellites = (uint8_t)atoi(token); break;
                case 6: evt->fix_quality = (uint8_t)atoi(token); break;
            }
            field++;
        }
        
        /* Post to sensor fusion */
        QActive_post(AO_SensorFusion, &evt->super, 0U);
    }
    (void)me;
}

static void AndroidComm_parseIMU(AndroidComm * const me, const char *data) {
    /* Parse: $IMU,ax,ay,az,gx,gy,gz,mx,my,mz* */
    IMUEvt *evt = Q_NEW(IMUEvt, SIG_IMU_UPDATE);
    if (evt != (IMUEvt *)0) {
        char *ptr = (char *)data;
        int field = 0;
        
        ptr = strchr(ptr, ',');
        if (ptr) ptr++;
        
        while (ptr && field < 9) {
            char *token = ptr;
            ptr = strchr(ptr, ',');
            if (ptr) *ptr++ = '\0';
            
            switch (field) {
                case 0: evt->accel_x = (int16_t)atoi(token); break;
                case 1: evt->accel_y = (int16_t)atoi(token); break;
                case 2: evt->accel_z = (int16_t)atoi(token); break;
                case 3: evt->gyro_x = (int16_t)atoi(token); break;
                case 4: evt->gyro_y = (int16_t)atoi(token); break;
                case 5: evt->gyro_z = (int16_t)atoi(token); break;
                case 6: evt->mag_x = (int16_t)atoi(token); break;
                case 7: evt->mag_y = (int16_t)atoi(token); break;
                case 8: evt->mag_z = (int16_t)atoi(token); break;
            }
            field++;
        }
        
        QActive_post(AO_SensorFusion, &evt->super, 0U);
    }
    (void)me;
}

static void AndroidComm_parseVision(AndroidComm * const me, const char *data) {
    /* Parse: $VIS,x,y,w,h,class,conf* */
    VisionEvt *evt = Q_NEW(VisionEvt, SIG_VISION_TARGET);
    if (evt != (VisionEvt *)0) {
        char *ptr = (char *)data;
        int field = 0;
        
        ptr = strchr(ptr, ',');
        if (ptr) ptr++;
        
        while (ptr && field < 6) {
            char *token = ptr;
            ptr = strchr(ptr, ',');
            if (ptr) *ptr++ = '\0';
            
            switch (field) {
                case 0: evt->target_x = (uint16_t)atoi(token); break;
                case 1: evt->target_y = (uint16_t)atoi(token); break;
                case 2: evt->target_width = (uint16_t)atoi(token); break;
                case 3: evt->target_height = (uint16_t)atoi(token); break;
                case 4: evt->target_class = (uint8_t)atoi(token); break;
                case 5: evt->confidence = (uint8_t)atoi(token); break;
            }
            field++;
        }
        
        /* Post to turret for tracking */
        QActive_post(AO_TurretCtrl, &evt->super, 0U);
    }
    (void)me;
}

static void AndroidComm_parseCommand(AndroidComm * const me, const char *data) {
    /* Parse: $CMD,type,param1,param2,...* */
    AndroidCmdEvt *evt = Q_NEW(AndroidCmdEvt, SIG_ANDROID_CMD);
    if (evt != (AndroidCmdEvt *)0) {
        char *ptr = (char *)data;
        
        ptr = strchr(ptr, ',');
        if (ptr) {
            ptr++;
            evt->cmd_type = (uint8_t)atoi(ptr);
            
            /* Copy remaining payload */
            ptr = strchr(ptr, ',');
            if (ptr) {
                ptr++;
                size_t len = strlen(ptr);
                if (len > 31) len = 31;
                memcpy(evt->payload, ptr, len);
                evt->payload[len] = '\0';
                evt->payload_len = (uint8_t)len;
            }
        }
        
        QActive_post(AO_Supervisor, &evt->super, 0U);
    }
    (void)me;
}

static void AndroidComm_processMessage(AndroidComm * const me, const char *msg) {
    me->rx_count++;
    me->last_heartbeat = BSP_get_tick();
    
    if (strncmp(msg, MSG_GPS, 4) == 0) {
        AndroidComm_parseGPS(me, msg);
    } else if (strncmp(msg, MSG_IMU, 4) == 0) {
        AndroidComm_parseIMU(me, msg);
    } else if (strncmp(msg, MSG_VIS, 4) == 0) {
        AndroidComm_parseVision(me, msg);
    } else if (strncmp(msg, MSG_CMD, 4) == 0) {
        AndroidComm_parseCommand(me, msg);
    }
}

static void AndroidComm_receiveChar(AndroidComm * const me, char c) {
    if (c == '\n' || c == '\r') {
        if (me->rx_index > 0) {
            me->rx_buffer[me->rx_index] = '\0';
            AndroidComm_processMessage(me, me->rx_buffer);
            me->rx_index = 0;
        }
    } else if (me->rx_index < RX_BUFFER_SIZE - 1) {
        me->rx_buffer[me->rx_index++] = c;
    }
}

/*==========================================================================*/
/* State Machine Implementation */
/*==========================================================================*/

static QState AndroidComm_initial(AndroidComm * const me, QEvt const * const e) {
    (void)e;
    
    me->rx_index = 0;
    me->connected = false;
    me->last_heartbeat = 0;
    me->rx_count = 0;
    me->tx_count = 0;
    
    /* Initialize UART for Pixel communication */
    BSP_uart_init(1, BSP_UART_BAUDRATE);
    
    /* Arm periodic timer (50ms) */
    QTimeEvt_armX(&me->timeEvt, BSP_TICKS_PER_SEC / 20, BSP_TICKS_PER_SEC / 20);
    
    return Q_TRAN(&AndroidComm_disconnected);
}

static QState AndroidComm_disconnected(AndroidComm * const me, QEvt const * const e) {
    QState status;
    
    switch (e->sig) {
        case Q_ENTRY_SIG:
            BSP_DEBUG_PRINT("AndroidComm: DISCONNECTED\r\n");
            me->connected = false;
            status = Q_HANDLED();
            break;
            
        case SIG_TIMEOUT:
            /* Check for incoming data */
            while (BSP_uart_rx_ready(1)) {
                char c = BSP_uart_getchar(1);
                AndroidComm_receiveChar(me, c);
            }
            
            /* Check if we received data recently */
            if (me->rx_count > 0) {
                status = Q_TRAN(&AndroidComm_connected);
            } else {
                status = Q_HANDLED();
            }
            break;
            
        default:
            status = Q_SUPER(&QHsm_top);
            break;
    }
    
    return status;
}

static QState AndroidComm_connected(AndroidComm * const me, QEvt const * const e) {
    QState status;
    
    switch (e->sig) {
        case Q_ENTRY_SIG:
            BSP_DEBUG_PRINT("AndroidComm: CONNECTED\r\n");
            me->connected = true;
            AndroidComm_sendMessage(me, "$ACK,CONNECTED*");
            status = Q_HANDLED();
            break;
            
        case SIG_TIMEOUT: {
            /* Check for incoming data */
            while (BSP_uart_rx_ready(1)) {
                char c = BSP_uart_getchar(1);
                AndroidComm_receiveChar(me, c);
            }
            
            /* Check for timeout (no data for 5 seconds) */
            uint32_t now = BSP_get_tick();
            if ((now - me->last_heartbeat) > 5000) {
                status = Q_TRAN(&AndroidComm_disconnected);
            } else {
                status = Q_HANDLED();
            }
            break;
        }
            
        case SIG_ANDROID_STATUS: {
            /* Send status to Pixel */
            snprintf(me->tx_buffer, TX_BUFFER_SIZE, 
                     "$STS,%d,%d,%lu*", 
                     me->connected ? 1 : 0,
                     BSP_adc_read(0),  /* Battery voltage */
                     (unsigned long)me->rx_count);
            AndroidComm_sendMessage(me, me->tx_buffer);
            status = Q_HANDLED();
            break;
        }
            
        default:
            status = Q_SUPER(&QHsm_top);
            break;
    }
    
    return status;
}

/*==========================================================================*/
/* Constructor */
/*==========================================================================*/

/* Placeholder for QHsm_top */
static QState QHsm_top(void * const me, QEvt const * const e) {
    (void)me;
    (void)e;
    return Q_IGNORED();
}

void AndroidComm_ctor(void) {
    AndroidComm *me = &l_androidComm;
    
    QActive_ctor(&me->super, (QStateHandler)&AndroidComm_initial);
    QTimeEvt_ctorX(&me->timeEvt, &me->super, SIG_TIMEOUT, 0U);
}
