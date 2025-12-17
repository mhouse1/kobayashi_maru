#include "FreeRTOS.h"
#include "task.h"
#include "qp_port.h"
#include "qp.h"

// Example QP/C active object (replace with your own)
Q_DEFINE_THIS_FILE

void vApplicationIdleHook(void) {
    // Optional: Idle hook for FreeRTOS
}

static void BlinkyTask_run(QActive * const me) {
    for (;;) {
        // Example: toggle LED or do something
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

static QActive * const ao_blinky = (QActive *)0; // Placeholder

int main(void) {
    // QP/C framework initialization
    QF_init();

    // BSP initialization (add your own)
    // BSP_init();

    // QP/C application initialization (add your own)
    // Blinky_ctor();

    // Start active objects (replace with your own)
    // QACTIVE_START(ao_blinky, ...);

    // Start the QF (RTOS) kernel
    return QF_run();
}
