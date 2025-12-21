#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

// -------------------------
// Thread functions
// -------------------------
void hello_thread(void *, void *, void *) {
    while (1) {
        printk("[HelloThread] tick=%u: hello\n", k_uptime_get_32());
        k_sleep(K_MSEC(1000));
    }
}

void world_thread(void *, void *, void *) {
    while (1) {
        printk("[WorldThread] tick=%u: world\n", k_uptime_get_32());
        k_sleep(K_MSEC(1000));
    }
}

// -------------------------
// Thread stack & control blocks
// -------------------------
K_THREAD_STACK_DEFINE(hello_stack, 512);
K_THREAD_STACK_DEFINE(world_stack, 512);

static struct k_thread hello_thread_data;
static struct k_thread world_thread_data;

// -------------------------
// Main
// -------------------------
void main(void) {
    // C1: CPU boot
    printk("[Main] CPU boot: main() entered\n");

    // C2: Memory map validity
    printk("[Main] Stack & memory regions initialized\n");

    // Create threads to verify C3 (UART output) & C4 (scheduler)
    k_thread_create(&hello_thread_data, hello_stack, K_THREAD_STACK_SIZEOF(hello_stack),
                    hello_thread, NULL, NULL, NULL, 5, 0, K_NO_WAIT);

    k_thread_create(&world_thread_data, world_stack, K_THREAD_STACK_SIZEOF(world_stack),
                    world_thread, NULL, NULL, NULL, 5, 0, K_NO_WAIT);

    printk("[Main] Threads created, scheduler running\n");
}
