#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>


#define TRL3_SELFTEST_REG (*(volatile uint32_t *)0x5000FF00)

void trl3_selftest(void)
{
    TRL3_SELFTEST_REG = 0xA5A5A5A5;   // Write test pattern
    uint32_t v = TRL3_SELFTEST_REG; // Read back

    if (v == 0xA5A5A5A5) {
        printk("TRL-3 self-test PASS\n");
    } else {
        printk("TRL-3 self-test FAIL: 0x%08x\n", v);
    }
}

void hello_thread(void *, void *, void *) {
    while (1) {
        printk("hello\n");
        k_sleep(K_MSEC(500));  // shorter sleep so messages appear quickly
    }
}

void world_thread(void *, void *, void *) {
    while (1) {
        printk("world\n");
        k_sleep(K_MSEC(500));
    }
}

// Thread stacks and data
K_THREAD_STACK_DEFINE(hello_stack, 512);
K_THREAD_STACK_DEFINE(world_stack, 512);
static struct k_thread hello_thread_data;
static struct k_thread world_thread_data;

void main(void) {
    printk("=== main started ===\n");

    // Create threads
    k_thread_create(&hello_thread_data, hello_stack, K_THREAD_STACK_SIZEOF(hello_stack),
                   hello_thread, NULL, NULL, NULL, 5, 0, K_NO_WAIT);

    k_thread_create(&world_thread_data, world_stack, K_THREAD_STACK_SIZEOF(world_stack),
                   world_thread, NULL, NULL, NULL, 5, 0, K_NO_WAIT);

    // Let main yield to allow threads to run immediately
    while (1) {
        k_sleep(K_MSEC(1000));
    }
}
