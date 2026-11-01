/***********************************************************************************************************************
 * File Name    : rs_i2c_if_cfg_data.c
 * Description  : Contains function definition.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2020 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "rs_i2c_if_cfg_data.h"
#include "rs_i2c_if_api.h"
#include "rm_isl76682.h"

/* I2C Communication Device */
rs_i2c_if_instance_ctrl_t g_rs_i2c_if_device_ctrl;
extern rs_i2c_if_bus_extended_cfg_t g_rs_i2c_if_bus_extended_cfg;

extern void rm_isl76682_callback(rs_i2c_if_callback_args_t *p_args);

/* Instance structure to use this module. */
#if BSP_CFG_RTOS
#if BSP_CFG_RTOS == 1
#if !defined(g_rs_i2c_if_bus_recursive_mutex)
TX_MUTEX g_rs_i2c_if_bus_recursive_mutex_handle;
CHAR g_rs_i2c_if_bus_recursive_mutex_name[] = "g_rs_i2c_if_bus recursive mutex";
#endif
#if !defined(g_rs_i2c_if_bus_blocking_semaphore)
TX_SEMAPHORE g_rs_i2c_if_bus_blocking_semaphore_handle;
CHAR g_rs_i2c_if_bus_blocking_semaphore_name[] = "g_rs_i2c_if_bus blocking semaphore";
#endif
#elif BSP_CFG_RTOS == 2
#if !defined(g_rs_i2c_if_bus_recursive_mutex)
SemaphoreHandle_t g_rs_i2c_if_bus_recursive_mutex_handle;
StaticSemaphore_t g_rs_i2c_if_bus_recursive_mutex_memory;
#endif
#if !defined(g_rs_i2c_if_bus_blocking_semaphore)
SemaphoreHandle_t g_rs_i2c_if_bus_blocking_semaphore_handle;
StaticSemaphore_t g_rs_i2c_if_bus_blocking_semaphore_memory;
#endif
#endif

#if !defined(g_rs_i2c_if_bus_recursive_mutex)
/* Recursive Mutex for I2C bus */
rs_i2c_if_mutex_t g_rs_i2c_if_bus_recursive_mutex =
{
    .p_mutex_handle = &g_rs_i2c_if_bus_recursive_mutex_handle,
#if BSP_CFG_RTOS == 1 // ThradX
    .p_mutex_name = &g_rs_i2c_if_bus_recursive_mutex_name[0],
#elif BSP_CFG_RTOS == 2 // FreeRTOS
    .p_mutex_memory = &g_rs_i2c_if_bus_recursive_mutex_memory,
#endif
};
#endif

#if !defined(g_rs_i2c_if_bus_blocking_semaphore)
/* Semaphore for blocking */
rs_i2c_if_semaphore_t g_rs_i2c_if_bus_blocking_semaphore =
{
    .p_semaphore_handle = &g_rs_i2c_if_bus_blocking_semaphore_handle,
#if BSP_CFG_RTOS == 1 // ThreadX
    .p_semaphore_name = &g_rs_i2c_if_bus_blocking_semaphore_name[0],
#elif BSP_CFG_RTOS == 2 // FreeRTOS
    .p_semaphore_memory = &g_rs_i2c_if_bus_blocking_semaphore_memory,
#endif
};
#endif
#endif

/* Shared I2C Bus */
#define RA_NOT_DEFINED (1)
rs_i2c_if_bus_extended_cfg_t g_rs_i2c_if_bus_extended_cfg =
{
#if !defined(g_i2c_master)
  .p_driver_instance = (void*) &g_i2c_master,
#elif !defined(RA_NOT_DEFINED)
    .p_driver_instance      = (void*)&RA_NOT_DEFINED,
#elif !defined(RA_NOT_DEFINED)
    .p_driver_instance      = (void*)&RA_NOT_DEFINED,
#endif
  .p_current_ctrl = NULL,
  .bus_timeout = 0xFFFFFFFF,
#if BSP_CFG_RTOS
#if !defined(g_rs_i2c_if_bus_blocking_semaphore)
    .p_blocking_semaphore = &g_rs_i2c_if_bus_blocking_semaphore,
#if !defined(g_rs_i2c_if_bus_recursive_mutex)
    .p_bus_recursive_mutex = &g_rs_i2c_if_bus_recursive_mutex,
#else
    .p_bus_recursive_mutex = NULL,
#endif
#else
    .p_bus_recursive_mutex = NULL,
    .p_blocking_semaphore = NULL,
#endif
#else
#endif

#if (0)
    .p_elc = (void*)&g_elc,
    .p_timer = (void*)&g_timer,
#else
  .p_elc = NULL,
  .p_timer = NULL,
#endif
        };
#undef RA_NOT_DEFINED

/* Lower level driver configuration */
const i2c_master_cfg_t g_rs_i2c_if_device_lower_level_cfg =
{ .slave = 0x44, .addr_mode = I2C_MASTER_ADDR_MODE_7BIT, .p_callback = rs_i2c_if_callback, };

const rs_i2c_if_cfg_t g_rs_i2c_if_device_cfg =
{ .semaphore_timeout = 0xFFFFFFFF, .p_lower_level_cfg = (void*) &g_rs_i2c_if_device_lower_level_cfg, .p_extend =
          (void*) &g_rs_i2c_if_bus_extended_cfg,
  .p_callback = rm_isl76682_callback,
#if defined(NULL)
    .p_context          = NULL,
#else
  .p_context = (void*) &NULL,
#endif
        };

const rs_i2c_if_instance_t g_i2c_if_device =
{ .p_ctrl = &g_rs_i2c_if_device_ctrl, .p_cfg = &g_rs_i2c_if_device_cfg, .p_api = &g_rs_i2c_if_on_i2c, };
