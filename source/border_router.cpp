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

#ifdef YOTTA_CFG_K64F_BORDER_ROUTER_DEBUG_TRACES
#if YOTTA_CFG_K64F_BORDER_ROUTER_DEBUG_TRACES == 1
#define HAVE_DEBUG 1
#endif
#else
#define HAVE_DEBUG 1
#endif

#include "ns_trace.h"
#define TRACE_GROUP "app"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define APP_DEFINED_HEAP_SIZE 32500
static uint8_t app_stack_heap[APP_DEFINED_HEAP_SIZE];
static uint8_t mac[6] = {0};

static SlipMACDriver *pslipmacdriver;
static Serial pc(USBTX, USBRX);
static DigitalOut led1(LED1);

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
    const char *driver = STR(YOTTA_CFG_K64F_BORDER_ROUTER_BACKHAUL_DRIVER);

    if (strcmp(driver, "SLIP") == 0) {
        int8_t slipdrv_id;
        pslipmacdriver = new SlipMACDriver(SERIAL_TX, SERIAL_RX);
        tr_debug("Using SLIP backhaul driver...");

        if (pslipmacdriver == NULL) {
            tr_error("Unable to create SLIP driver");
            return;
        }

        slipdrv_id = pslipmacdriver->Slip_Init(mac);

        if (slipdrv_id >= 0) {
            backhaul_driver_status_cb(1, slipdrv_id);
            return;
        }

        tr_error("Backhaul driver init failed, retval = %d", slipdrv_id);
    } else if (strcmp(driver, "ETH") == 0) {
        tr_debug("Using ETH backhaul driver...");
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
    // set the baud rate for output printing
    pc.baud(YOTTA_CFG_K64F_BORDER_ROUTER_BAUD);

    // set heap size and memory error handler for this application
    ns_dyn_mem_init(app_stack_heap, APP_DEFINED_HEAP_SIZE, app_heap_error_handler, 0);

    trace_init(); // set up the tracing library
    set_trace_print_function(trace_printer);
    set_trace_config(TRACE_MODE_COLOR | TRACE_ACTIVE_LEVEL_DEBUG | TRACE_CARRIAGE_RETURN);

    const char *mac_src = STR(YOTTA_CFG_K64F_BORDER_ROUTER_BACKHAUL_MAC_SRC);

    if (strcmp(mac_src, "BOARD") == 0) {
        /* Setting the MAC Address from UID (A yotta function)
         * Takes UID Mid low and UID low and shuffles them around. */
        mbed_mac_address((char *)mac);
    } else if (strcmp(mac_src, "CONFIG") == 0) {
        /* MAC is defined by the user through yotta configuration */
        const uint8_t mac48[] = YOTTA_CFG_K64F_BORDER_ROUTER_BACKHAUL_MAC;

        for (int i = 0; i < sizeof(mac); ++i) {
            mac[i] = mac48[i];
        }
    }

    // run LED toggler in the Minar scheduler
    minar::Scheduler::postCallback(mbed::util::FunctionPointer0<void>
                                   (toggle_led1).bind()).period(minar::milliseconds(500));

    tr_info("Starting K64F border router...");
    border_router_start();
}



/**
 * \brief Error handler for errors in dynamic memory handling.
 */
static void app_heap_error_handler(heap_fail_t event)
{
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
