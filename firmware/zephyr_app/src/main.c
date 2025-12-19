
#include <zephyr/kernel.h>

void hello_thread(void *, void *, void *) {
    while (1) {
        printk("hello\n");
        k_sleep(K_MSEC(1000));
    }
}

void world_thread(void *, void *, void *) {
    while (1) {
        printk("world\n");
        k_sleep(K_MSEC(1000));
    }
}

K_THREAD_STACK_DEFINE(hello_stack, 512);
K_THREAD_STACK_DEFINE(world_stack, 512);
static struct k_thread hello_thread_data;
static struct k_thread world_thread_data;

void main(void) {
    k_thread_create(&hello_thread_data, hello_stack, K_THREAD_STACK_SIZEOF(hello_stack),
                   hello_thread, NULL, NULL, NULL, 5, 0, K_NO_WAIT);
    k_thread_create(&world_thread_data, world_stack, K_THREAD_STACK_SIZEOF(world_stack),
                   world_thread, NULL, NULL, NULL, 5, 0, K_NO_WAIT);
}
