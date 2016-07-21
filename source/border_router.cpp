/*
 * Copyright (c) 2016 ARM Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string.h>
#include "mbed-drivers/mbed.h"
#include "nsdynmemLIB.h"
#include "nanostack-border-router/borderrouter_tasklet.h"
#include "sal-nanostack-driver-k64f-eth/k64f_eth_nanostack_port.h"
#include "sal-stack-nanostack-slip/Slip.h"

#if YOTTA_CFG_K64F_BORDER_ROUTER_DEBUG_TRACES==1
#define HAVE_DEBUG 1
#endif

#ifdef  MBED_CONF_APP_DEBUG_TRACE
#if MBED_CONF_APP_DEBUG_TRACE == 1
#define APP_TRACE_LEVEL TRACE_ACTIVE_LEVEL_DEBUG
#else
#define APP_TRACE_LEVEL TRACE_ACTIVE_LEVEL_INFO
#endif
#endif //MBED_CONF_APP_DEBUG_TRACE

#ifdef MBED_CONF_RTOS_PRESENT
#include "ns_hal_init.h"
#include "cmsis_os.h"
#include "arm_hal_interrupt.h"
#endif

#include "ns_trace.h"

#define TRACE_GROUP "app"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#ifndef MBED_CONF_RTOS_PRESENT
#define APP_DEFINED_HEAP_SIZE 63000
#else
#define APP_DEFINED_HEAP_SIZE MBED_CONF_APP_HEAP_SIZE
#endif //MBED_CONF_RTOS_PRESENT

static uint8_t app_stack_heap[APP_DEFINED_HEAP_SIZE];
static uint8_t mac[6] = {0};

static SlipMACDriver *pslipmacdriver;
static Serial pc(USBTX, USBRX);
static DigitalOut led1(LED1);

#ifdef MBED_CONF_RTOS_PRESENT
static Ticker led_ticker;
#endif

static void app_heap_error_handler(heap_fail_t event);

static void toggle_led1()
{
    led1 = !led1;
}

/**
 * \brief Prints string to serial (adds CRLF).
 */
static void trace_printer(const char *str)
{
    pc.printf("%s\r\n", str);
}

/**
 * \brief Initializes the SLIP MAC backhaul driver.
 * This function is called by the border router module.
 */
void backhaul_driver_init(void (*backhaul_driver_status_cb)(uint8_t, int8_t))
{
    const char *driver;

#ifndef MBED_CONF_RTOS_PRESENT
    driver = STR(YOTTA_CFG_K64F_BORDER_ROUTER_BACKHAUL_DRIVER);
#else
    driver = STR(MBED_CONF_APP_BACKHAUL_DRIVER);
#endif

    if (strcmp(driver, "SLIP") == 0) {
        int8_t slipdrv_id;
#if defined(MBED_CONF_APP_SLIP_HW_FLOW_CONTROL) || defined(YOTTA_CFG_K64F_BORDER_ROUTER_SLIP_HW_FLOW_CONTROL)
        pslipmacdriver = new SlipMACDriver(SERIAL_TX, SERIAL_RX, SERIAL_RTS, SERIAL_CTS);
#else
        pslipmacdriver = new SlipMACDriver(SERIAL_TX, SERIAL_RX);
#endif

        if (pslipmacdriver == NULL) {
            tr_error("Unable to create SLIP driver");
            return;
        }

        tr_info("Using SLIP backhaul driver...");

#ifndef MBED_CONF_RTOS_PRESENT
        slipdrv_id = pslipmacdriver->Slip_Init(mac, YOTTA_CFG_K64F_BORDER_ROUTER_BACKHAUL_SERIAL_BAUD);
#else
        slipdrv_id = pslipmacdriver->Slip_Init(mac, MBED_CONF_APP_SLIP_SERIAL_BAUD_RATE);
#endif

        if (slipdrv_id >= 0) {
            backhaul_driver_status_cb(1, slipdrv_id);
            return;
        }

        tr_error("Backhaul driver init failed, retval = %d", slipdrv_id);
    } else if (strcmp(driver, "ETH") == 0) {
        tr_info("Using ETH backhaul driver...");
        arm_eth_phy_device_register(mac, backhaul_driver_status_cb);
        return;
    }

    tr_error("Unsupported backhaul driver: %s", driver);
}

/**
 * \brief The entry point for this application.
 * Sets up the application and starts the border router module.
 */

void app_start(int, char **)
{

#ifndef MBED_CONF_RTOS_PRESENT
    // set the baud rate for output printing
    pc.baud(YOTTA_CFG_K64F_BORDER_ROUTER_BAUD);
    // set heap size and memory error handler for this application
    ns_dyn_mem_init(app_stack_heap, APP_DEFINED_HEAP_SIZE, app_heap_error_handler, 0);

#else
    pc.baud(MBED_CONF_APP_TRACE_BAUD_RATE);
    ns_hal_init(app_stack_heap, APP_DEFINED_HEAP_SIZE, app_heap_error_handler, 0);
#endif

    trace_init(); // set up the tracing library
    set_trace_print_function(trace_printer);
    set_trace_config(TRACE_MODE_COLOR | APP_TRACE_LEVEL | TRACE_CARRIAGE_RETURN);

    const char *mac_src;

#ifndef MBED_CONF_RTOS_PRESENT
    mac_src = STR(YOTTA_CFG_K64F_BORDER_ROUTER_BACKHAUL_MAC_SRC);
#else
    mac_src = STR(MBED_CONF_APP_BACKHAUL_MAC_SRC);
#endif

    if (strcmp(mac_src, "BOARD") == 0) {
        /* Setting the MAC Address from UID (A yotta function)
         * Takes UID Mid low and UID low and shuffles them around. */
        mbed_mac_address((char *)mac);
    } else if (strcmp(mac_src, "CONFIG") == 0) {
        /* MAC is defined by the user through yotta configuration */
#ifndef MBED_CONF_RTOS_PRESENT
        const uint8_t mac48[] = YOTTA_CFG_K64F_BORDER_ROUTER_BACKHAUL_MAC;
#else
        const uint8_t mac48[] = MBED_CONF_APP_BACKHAUL_MAC;
#endif

        for (uint32_t i = 0; i < sizeof(mac); ++i) {
            mac[i] = mac48[i];
        }
    }

#ifndef MBED_CONF_RTOS_PRESENT
    // run LED toggler in the Minar scheduler
    minar::Scheduler::postCallback(mbed::util::FunctionPointer0<void>
                                  (toggle_led1).bind()).period(minar::milliseconds(500));
#else
    led_ticker.attach_us(toggle_led1, 500000);
#endif
    tr_info("Starting K64F border router...");
    border_router_start();
}

#ifdef MBED_CONF_RTOS_PRESENT
int main(int argc, char **argv)
{
    app_start(argc, argv);
}
#endif



/**
 * \brief Error handler for errors in dynamic memory handling.
 */
static void app_heap_error_handler(heap_fail_t event)
{
    tr_error("Dyn mem error %x", (int8_t)event);

    switch (event) {
        case NS_DYN_MEM_NULL_FREE:
            break;
        case NS_DYN_MEM_DOUBLE_FREE:
            break;
        case NS_DYN_MEM_ALLOCATE_SIZE_NOT_VALID:
            break;
        case NS_DYN_MEM_POINTER_NOT_VALID:
            break;
        case NS_DYN_MEM_HEAP_SECTOR_CORRUPTED:
            break;
        case NS_DYN_MEM_HEAP_SECTOR_UNITIALIZED:
            break;
        default:
            break;
    }

    while (1);
}
