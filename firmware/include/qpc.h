/*
 * QP/C Framework Configuration
 * FRDM-MCXN947 Robot Application
 * 
 * This file provides stub definitions for the QP/C framework.
 * In a real implementation, this would include the actual QP/C headers.
 */

#ifndef QPC_H
#define QPC_H

#include <stdint.h>
#include <stdbool.h>

/*==========================================================================*/
/* QP/C Framework Version */
/*==========================================================================*/

#define QP_VERSION_STR  "7.3.0"

/*==========================================================================*/
/* QP/C Event Types */
/*==========================================================================*/

/* Base event structure */
typedef struct QEvt {
    uint16_t sig;           /* Signal of the event */
    uint8_t poolId;         /* Pool ID (for dynamic events) */
    uint8_t volatile refCtr; /* Reference counter */
} QEvt;

/* Reserved signals */
enum {
    Q_ENTRY_SIG = 0,
    Q_EXIT_SIG,
    Q_INIT_SIG,
    Q_USER_SIG
};

/*==========================================================================*/
/* QP/C State Machine Types */
/*==========================================================================*/

/* Forward declarations */
struct QHsm;
struct QMsm;
struct QActive;

/* State handler return type */
typedef uint8_t QState;

/* State handler function pointer */
typedef QState (*QStateHandler)(void * const me, QEvt const * const e);

/* Hierarchical State Machine (HSM) base class */
typedef struct QHsm {
    QStateHandler state;
    QStateHandler temp;
} QHsm;

/* Active Object base class */
typedef struct QActive {
    QHsm super;
    uint8_t prio;
    /* Event queue (simplified) */
    QEvt const **queue;
    uint8_t queueLen;
    uint8_t head;
    uint8_t tail;
    uint8_t nUsed;
} QActive;

/*==========================================================================*/
/* QP/C Macros */
/*==========================================================================*/

/* State machine return values */
#define Q_RET_HANDLED       ((QState)0)
#define Q_RET_IGNORED       ((QState)1)
#define Q_RET_TRAN          ((QState)2)
#define Q_RET_SUPER         ((QState)3)

/* State transition macros */
#define Q_HANDLED()         (Q_RET_HANDLED)
#define Q_IGNORED()         (Q_RET_IGNORED)
#define Q_TRAN(target)      (((QHsm *)me)->temp = (QStateHandler)(target), Q_RET_TRAN)
#define Q_SUPER(super)      (((QHsm *)me)->temp = (QStateHandler)(super), Q_RET_SUPER)

/* Event casting macros */
#define Q_EVT_CAST(class_)  ((class_ const *)e)

/* Assertion macros */
#define Q_ASSERT(test)      ((void)0)
#define Q_REQUIRE(test)     ((void)0)
#define Q_ENSURE(test)      ((void)0)

/*==========================================================================*/
/* QP/C Time Events */
/*==========================================================================*/

typedef struct QTimeEvt {
    QEvt super;
    struct QTimeEvt *next;
    QActive *act;
    uint32_t ctr;
    uint32_t interval;
} QTimeEvt;

/*==========================================================================*/
/* QP/C Function Prototypes */
/*==========================================================================*/

/* Framework initialization */
void QF_init(void);
void QF_run(void);
void QF_stop(void);

/* Active Object operations */
void QActive_ctor(QActive * const me, QStateHandler initial);
void QActive_start(QActive * const me, uint8_t prio,
                   QEvt const **qSto, uint16_t qLen,
                   void *stkSto, uint32_t stkSize,
                   void const * const par);
bool QActive_post(QActive * const me, QEvt const * const e, uint16_t margin);
void QActive_postLIFO(QActive * const me, QEvt const * const e);

/* Event pool management */
void QF_poolInit(void * const poolSto, uint32_t poolSize, uint32_t evtSize);
QEvt *QF_newX(uint32_t evtSize, uint16_t margin, uint16_t sig);
void QF_gc(QEvt const * const e);

/* Time event operations */
void QTimeEvt_ctorX(QTimeEvt * const me, QActive * const act, uint16_t sig, uint8_t tickRate);
bool QTimeEvt_armX(QTimeEvt * const me, uint32_t nTicks, uint32_t interval);
bool QTimeEvt_disarm(QTimeEvt * const me);
void QF_tickX(uint8_t tickRate);

/* Helper macros for event allocation */
#define Q_NEW(evtT_, sig_)  ((evtT_ *)QF_newX(sizeof(evtT_), 1U, (sig_)))
#define Q_NEW_X(evtT_, margin_, sig_)  ((evtT_ *)QF_newX(sizeof(evtT_), (margin_), (sig_)))

#endif /* QPC_H */
