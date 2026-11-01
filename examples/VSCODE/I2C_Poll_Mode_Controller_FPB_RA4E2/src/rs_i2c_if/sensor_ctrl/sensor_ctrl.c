/***********************************************************************************************************************
 * File Name    : sensor_ctrl.c
 * Description  : Contains function definition.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2020 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/
#include "rs_i2c_if.h"
#include "sensor_ctrl.h"
#include "ra_isl76682_profile.h"
#include "rm_isl76682_api.h"
#include "rm_isl76682.h"

#define NOT_INITIALIZED (0U)
#define INITIALIZED     (1U)

extern rm_isl76682_instance_ctrl_t g_isl76682_als_ctrl;

#if BSP_CFG_RTOS == 2
/*******************************************************************************************************************//**
 * @brief Converts a timeout value in milliseconds to FreeRTOS ticks.
 *
 * The function converts @p timeout_ms to the corresponding FreeRTOS tick count. If a non-zero timeout converts to
 * zero ticks because it is shorter than one RTOS tick period, the timeout is rounded up to one tick.
 *
 * @param[in] timeout_ms  Timeout value in milliseconds.
 *
 * @return Timeout value converted to FreeRTOS ticks.
 **********************************************************************************************************************/
static TickType_t rs_i2c_if_timeout_to_ticks(uint32_t timeout_ms)
{
    TickType_t ticks = pdMS_TO_TICKS(timeout_ms);
    if ((timeout_ms > 0U) && (ticks == 0U))
    {
        ticks = (TickType_t) 1;
    }
    return ticks;
}

/*******************************************************************************************************************//**
 * @brief Removes any pending tokens from an ICM42670 interrupt semaphore.
 *
 * The function repeatedly attempts to take @p semaphore without blocking until no semaphore token remains. This
 * clears any previously signaled interrupt event before starting a new wait operation.
 *
 * @param[in] semaphore  FreeRTOS semaphore handle to drain.
 *
 * @note No action is performed if @p semaphore is NULL.
 **********************************************************************************************************************/
static void rs_i2c_if_semaphore_drain(SemaphoreHandle_t semaphore)
{
    if (NULL != semaphore)
    {
        while (pdTRUE == xSemaphoreTake(semaphore, (TickType_t) 0))
        {
        }
    }
}
#endif


/*******************************************************************************************************************//**
 * @brief Generates the writable SRAM object hierarchy required by rs_i2c_if_SensorDataRead().
 *
 * @param[in]  p_src_i2c_if_instance  Source RS I2C communications instance, normally FSP/generated const data.
 * @param[in]  p_src_sensor_instance Source ICM42670 instance, normally FSP/generated const data.
 * @param[in]  p_src_irq_instance    Source external IRQ instance, normally FSP/generated const data.
 * @param[out] p_dst                 Complete destination hierarchy in writable SRAM.
 *
 * @retval FSP_SUCCESS              The SRAM hierarchy was generated successfully.
 * @retval FSP_ERR_ASSERTION        A required pointer or nested source pointer is NULL.
 * @retval FSP_ERR_INVALID_ARGUMENT A source instance aliases storage inside the destination object.
 *
 * @note This function performs shallow copies, then redirects all mutable links needed by
 *       rs_i2c_if_SensorDataRead() to objects in @p p_dst. Read-only API tables remain in Flash.
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @brief Builds a writable (SRAM) copy of the ISL76682 configuration hierarchy and reconnects every internal
 *        link so the copy is self-contained. The generated hierarchy is then passed to
 *        rs_isl76682_Read_light_sensor_data().
 *
 * @param[in]  p_src_i2c_if_instance  Original RS I2C source instance (typically a const FSP-generated object).
 * @param[in]  p_src_light_instance  Original ISL76682 source instance.
 * @param[out] p_dst                 Destination hierarchy to be cleared, filled and reconnected.
 *
 * @retval FSP_SUCCESS               Hierarchy copied and reconnected.
 * @retval FSP_ERR_ASSERTION         A required pointer is NULL.
 * @retval FSP_ERR_INVALID_ARGUMENT  Source and destination objects overlap.
 **********************************************************************************************************************/
fsp_err_t rs_i2c_if_LightSensorConfigGen (
    rs_i2c_if_instance_t const         * const p_src_i2c_if_instance,
    rm_isl76682_instance_t const       * const p_src_light_instance,
    rs_i2c_if_light_sensor_ctrl_t      * const p_dst)
{
    rs_i2c_if_cfg_t const              * p_src_i2c_if_cfg;
    i2c_master_cfg_t const             * p_src_lower_cfg;
    rs_i2c_if_bus_extended_cfg_t const * p_src_bus_cfg;
    rm_isl76682_cfg_t const            * p_src_light_cfg;

    /* Validate the top-level arguments. */
    FSP_ERROR_RETURN(NULL != p_src_i2c_if_instance, FSP_ERR_ASSERTION);
    FSP_ERROR_RETURN(NULL != p_src_light_instance, FSP_ERR_ASSERTION);
    FSP_ERROR_RETURN(NULL != p_dst,                FSP_ERR_ASSERTION);

    /* Reject a source that already is the destination copy (memset below would destroy it). */
    FSP_ERROR_RETURN((void const *) p_src_i2c_if_instance != (void const *) &p_dst->i2c_instance,   FSP_ERR_INVALID_ARGUMENT);
    FSP_ERROR_RETURN((void const *) p_src_light_instance != (void const *) &p_dst->light_instance, FSP_ERR_INVALID_ARGUMENT);

    /* Validate the source instance members. */
    FSP_ERROR_RETURN(NULL != p_src_i2c_if_instance->p_cfg, FSP_ERR_ASSERTION);
    FSP_ERROR_RETURN(NULL != p_src_i2c_if_instance->p_api, FSP_ERR_ASSERTION);
    FSP_ERROR_RETURN(NULL != p_src_light_instance->p_cfg, FSP_ERR_ASSERTION);
    FSP_ERROR_RETURN(NULL != p_src_light_instance->p_api, FSP_ERR_ASSERTION);

    p_src_i2c_if_cfg = p_src_i2c_if_instance->p_cfg;
    p_src_lower_cfg = (i2c_master_cfg_t const *) p_src_i2c_if_cfg->p_lower_level_cfg;
    p_src_bus_cfg   = (rs_i2c_if_bus_extended_cfg_t const *) p_src_i2c_if_cfg->p_extend;
    p_src_light_cfg = p_src_light_instance->p_cfg;

    /* Validate the nested configuration pointers that will be copied. */
    FSP_ERROR_RETURN(NULL != p_src_lower_cfg, FSP_ERR_ASSERTION);
    FSP_ERROR_RETURN(NULL != p_src_bus_cfg,   FSP_ERR_ASSERTION);

    /* The I2C driver instance is dereferenced later by rs_isl76682_Read_light_sensor_data(). */
    FSP_ERROR_RETURN(NULL != p_src_bus_cfg->p_driver_instance, FSP_ERR_ASSERTION);

    /* Clear every runtime object before reconnecting the copied configuration hierarchy. */
    memset(p_dst, 0, sizeof(*p_dst));

    memcpy(&p_dst->i2c_instance,   p_src_i2c_if_instance, sizeof(p_dst->i2c_instance));
    memcpy(&p_dst->i2c_cfg,        p_src_i2c_if_cfg,      sizeof(p_dst->i2c_cfg));
    memcpy(&p_dst->i2c_hal_cfg,    p_src_lower_cfg,      sizeof(p_dst->i2c_hal_cfg));
    memcpy(&p_dst->i2c_bus_cfg,    p_src_bus_cfg,        sizeof(p_dst->i2c_bus_cfg));
    memcpy(&p_dst->light_instance, p_src_light_instance, sizeof(p_dst->light_instance));
    memcpy(&p_dst->light_cfg,      p_src_light_cfg,      sizeof(p_dst->light_cfg));

    /* Reconnect the writable RS I2C communications hierarchy. */
    p_dst->i2c_bus_cfg.p_current_ctrl = NULL;
    p_dst->i2c_cfg.p_lower_level_cfg  = (void *) &p_dst->i2c_hal_cfg;
    p_dst->i2c_cfg.p_extend           = (void *) &p_dst->i2c_bus_cfg;
    p_dst->i2c_instance.p_ctrl        = (rs_i2c_if_ctrl_t *) &p_dst->i2c_ctrl;
    p_dst->i2c_instance.p_cfg         = &p_dst->i2c_cfg;

    /* Route the I2C completion callback to the writable ISL76682 control block, not the original one.
     * rm_isl76682_callback() reads its instance control block from this context. Only redirect it if the
     * source configuration actually used a context. */
    if (NULL != p_src_i2c_if_cfg->p_context)
    {
        p_dst->i2c_cfg.p_context = (void *) &p_dst->light_ctrl;
    }

    /* Reconnect the writable ISL76682 hierarchy. */
    p_dst->light_cfg.p_instance   = &p_dst->i2c_instance;
    p_dst->light_instance.p_ctrl  = (rm_isl76682_ctrl_t *) &p_dst->light_ctrl;
    p_dst->light_instance.p_cfg   = &p_dst->light_cfg;

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Validates the SRAM arguments and pointer topology required by rs_isl76682_Read_light_sensor_data().
 *
 * @param[in] p_src_i2c_if_instance  Original RS I2C source instance.
 * @param[in] p_src_light_instance  Original ISL76682 source instance.
 * @param[in] p_dst                 Generated SRAM hierarchy to validate.
 *
 * @retval FSP_SUCCESS              Every required argument and link is valid.
 * @retval FSP_ERR_INVALID_POINTER  A required pointer is NULL.
 * @retval FSP_ERR_INVALID_ARGUMENT A pointer link, API table, or copied key configuration value is invalid.
 **********************************************************************************************************************/
fsp_err_t rs_i2c_if_LightSensorConfigCheck (
    rs_i2c_if_instance_t const              * const p_src_i2c_if_instance,
    rm_isl76682_instance_t const            * const p_src_light_instance,
    rs_i2c_if_light_sensor_ctrl_t const     * const p_dst)
{
    i2c_master_cfg_t const * p_src_lower_cfg;

    FSP_ERROR_RETURN(NULL != p_src_i2c_if_instance, FSP_ERR_INVALID_POINTER);
    FSP_ERROR_RETURN(NULL != p_src_light_instance, FSP_ERR_INVALID_POINTER);
    FSP_ERROR_RETURN(NULL != p_dst,                FSP_ERR_INVALID_POINTER);

    FSP_ERROR_RETURN(NULL != p_src_i2c_if_instance->p_cfg, FSP_ERR_INVALID_POINTER);
    FSP_ERROR_RETURN(NULL != p_src_light_instance->p_cfg, FSP_ERR_INVALID_POINTER);

    p_src_lower_cfg = (i2c_master_cfg_t const *) p_src_i2c_if_instance->p_cfg->p_lower_level_cfg;
    FSP_ERROR_RETURN(NULL != p_src_lower_cfg, FSP_ERR_INVALID_POINTER);

    /* Validate objects passed directly to rs_isl76682_Read_light_sensor_data(). */
    FSP_ERROR_RETURN(NULL != p_dst->light_instance.p_ctrl, FSP_ERR_INVALID_POINTER);
    FSP_ERROR_RETURN(NULL != p_dst->light_instance.p_cfg,  FSP_ERR_INVALID_POINTER);
    FSP_ERROR_RETURN(NULL != p_dst->light_instance.p_api,  FSP_ERR_INVALID_POINTER);

    /* Validate ISL76682 instance links. */
    FSP_ERROR_RETURN(p_dst->light_instance.p_ctrl == (rm_isl76682_ctrl_t *) &p_dst->light_ctrl, FSP_ERR_INVALID_ARGUMENT);
    FSP_ERROR_RETURN(p_dst->light_instance.p_cfg  == &p_dst->light_cfg,                         FSP_ERR_INVALID_ARGUMENT);
    FSP_ERROR_RETURN(p_dst->light_instance.p_api  == p_src_light_instance->p_api,               FSP_ERR_INVALID_ARGUMENT);

    /* Validate ISL76682 -> communications link. */
    FSP_ERROR_RETURN(p_dst->light_cfg.p_instance == &p_dst->i2c_instance, FSP_ERR_INVALID_ARGUMENT);

    /* Validate communications instance and nested configuration links. */
    FSP_ERROR_RETURN(p_dst->i2c_instance.p_ctrl == (rs_i2c_if_ctrl_t *) &p_dst->i2c_ctrl, FSP_ERR_INVALID_ARGUMENT);
    FSP_ERROR_RETURN(p_dst->i2c_instance.p_cfg  == &p_dst->i2c_cfg,                       FSP_ERR_INVALID_ARGUMENT);
    FSP_ERROR_RETURN(p_dst->i2c_instance.p_api  == p_src_i2c_if_instance->p_api,           FSP_ERR_INVALID_ARGUMENT);
    FSP_ERROR_RETURN(p_dst->i2c_cfg.p_lower_level_cfg == (void const *) &p_dst->i2c_hal_cfg, FSP_ERR_INVALID_ARGUMENT);
    FSP_ERROR_RETURN(p_dst->i2c_cfg.p_extend          == (void const *) &p_dst->i2c_bus_cfg, FSP_ERR_INVALID_ARGUMENT);
    FSP_ERROR_RETURN(NULL == p_dst->i2c_bus_cfg.p_current_ctrl,                              FSP_ERR_INVALID_ARGUMENT);

    /* Validate the callback context, if one was used by the source configuration. */
    if (NULL != p_src_i2c_if_instance->p_cfg->p_context)
    {
        FSP_ERROR_RETURN(p_dst->i2c_cfg.p_context == (void const *) &p_dst->light_ctrl, FSP_ERR_INVALID_ARGUMENT);
    }

    /* Validate the I2C driver instance used to open the bus. */
    FSP_ERROR_RETURN(NULL != p_dst->i2c_bus_cfg.p_driver_instance, FSP_ERR_INVALID_POINTER);

    /* Validate key copied I2C values. */
    FSP_ERROR_RETURN(p_dst->i2c_hal_cfg.channel   == p_src_lower_cfg->channel,   FSP_ERR_INVALID_ARGUMENT);
    FSP_ERROR_RETURN(p_dst->i2c_hal_cfg.rate      == p_src_lower_cfg->rate,      FSP_ERR_INVALID_ARGUMENT);
    FSP_ERROR_RETURN(p_dst->i2c_hal_cfg.addr_mode == p_src_lower_cfg->addr_mode, FSP_ERR_INVALID_ARGUMENT);

    /* Validate key copied ISL76682 values. */
    FSP_ERROR_RETURN(p_dst->light_cfg.p_context  == p_src_light_instance->p_cfg->p_context,  FSP_ERR_INVALID_ARGUMENT);
    FSP_ERROR_RETURN(p_dst->light_cfg.p_callback == p_src_light_instance->p_cfg->p_callback, FSP_ERR_INVALID_ARGUMENT);

    /* Validate API entries dereferenced by the ISL76682 driver. */
    FSP_ERROR_RETURN(NULL != p_dst->light_instance.p_api->open,          FSP_ERR_INVALID_POINTER);
    FSP_ERROR_RETURN(NULL != p_dst->light_instance.p_api->close,         FSP_ERR_INVALID_POINTER);
    FSP_ERROR_RETURN(NULL != p_dst->light_instance.p_api->setCmdReg,     FSP_ERR_INVALID_POINTER);
    FSP_ERROR_RETURN(NULL != p_dst->light_instance.p_api->readLightLux,  FSP_ERR_INVALID_POINTER);
    FSP_ERROR_RETURN(NULL != p_dst->i2c_instance.p_api->open,            FSP_ERR_INVALID_POINTER);
    FSP_ERROR_RETURN(NULL != p_dst->i2c_instance.p_api->close,           FSP_ERR_INVALID_POINTER);
    FSP_ERROR_RETURN(NULL != p_dst->i2c_instance.p_api->write,           FSP_ERR_INVALID_POINTER);
    FSP_ERROR_RETURN(NULL != p_dst->i2c_instance.p_api->read,            FSP_ERR_INVALID_POINTER);

    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*****************************************************************************************************************
 ** @brief       User callback for the ISL76682 middleware. It is called from rm_isl76682_callback(), which is
 **              itself invoked from the I2C communications middleware callback when a transfer finishes. The
 **              function translates the reported event into the communication_finished flag that the
 **              ISL76682 driver functions poll while waiting for an I2C transfer to complete.
 **
 **              On RM_ISL76682_EVENT_SUCCESS the flag is set to true, which releases the waiting driver
 **              function. On any other event (an I2C error) the flag is set to false, so the waiting
 **              function does not see the transfer as finished and ends with a timeout.
 **
 **              Note: this function typically runs in interrupt context, so it must stay short and must not
 **              block.
 **
 ** @param[in]   p_args          Pointer to the callback arguments, which carry the event reported for the
 **                              completed I2C transfer.
 **
 ** @retval      NONE
 ******************************************************************************************************************/
void isl76682_callback(rm_isl76682_callback_args_t * p_args)
{
    /* Ignore the callback if no arguments were supplied. */
     if (NULL == p_args)
     {
         return;
     }

    /* Transfer completed without error: release the driver function waiting on the flag */
    if (RM_ISL76682_EVENT_SUCCESS == p_args->event)
    {
        g_isl76682_als_ctrl.communication_finished = true;
    }
    else
    {
        /* Transfer failed: leave the flag false so the waiting function times out */
        g_isl76682_als_ctrl.communication_finished = false;
    }
}

/*****************************************************************************************************************
 ** @brief       Reads ambient light data from the ISL76682 light sensor. On the first call, this function
 **              validates its parameters, then performs the one-time initialization: it opens the I2C bus
 **              driver, opens the ISL76682 middleware instance, and writes the user settings to the COMMAND
 **              register. On every later call, it reads the current light level in lux.
 **
 **              Note: the first call only initializes the sensor and does not return a lux value. The caller
 **              must call this function again to obtain a reading.
 **
 ** @param[in]   p_ctrl          Pointer to the ISL76682 middleware control block.
 ** @param[in]   p_cfg           Pointer to the ISL76682 middleware configuration structure.
 ** @param[in]   p_user_setting  Pointer to the sensor settings.
 ** @param[out]  data            Pointer to the variable that receives the light level in lux.
 **
 ** @retval      FSP_SUCCESS                 Initialization completed, or the lux value was read successfully.
 ** @retval      FSP_ERR_ASSERTION           A required pointer or API function pointer is NULL.
 ** @retval      FSP_ERR_INVALID_ARGUMENT    The external scale resistor value is not greater than zero.
 ** @retval      Other                       Error code from the I2C driver open, RM_ISL76682_Open,
 **                                          RM_ISL76682_SetCmdReg, or RM_ISL76682_ReadLightLux.
 ******************************************************************************************************************/
fsp_err_t rs_isl76682_Read_light_sensor_data(rm_isl76682_ctrl_t *p_ctrl, rm_isl76682_cfg_t *p_cfg,
                                             ra_isl76682_setting_t *p_user_setting, float *data)
{
    /* Persistent flag: keeps its value between calls so initialization runs only once */
    volatile static int isInitialized = NOT_INITIALIZED;
    fsp_err_t err = FSP_SUCCESS;
    fsp_err_t close_err = FSP_SUCCESS;
    rs_i2c_if_bus_extended_cfg_t const * p_extend = NULL;
    i2c_master_instance_t const * p_driver_instance = NULL;

    /* Validate parameters required on every call. */
    FSP_ERROR_RETURN(NULL != p_ctrl,         FSP_ERR_ASSERTION);
    FSP_ERROR_RETURN(NULL != p_cfg,          FSP_ERR_ASSERTION);
    FSP_ERROR_RETURN(NULL != p_user_setting, FSP_ERR_ASSERTION);
    FSP_ERROR_RETURN(NULL != data,           FSP_ERR_ASSERTION);

    /* Validate communications middleware instance members. */
    FSP_ERROR_RETURN(NULL != p_cfg->p_instance,         FSP_ERR_ASSERTION);
    FSP_ERROR_RETURN(NULL != p_cfg->p_instance->p_cfg,  FSP_ERR_ASSERTION);
    FSP_ERROR_RETURN(NULL != p_cfg->p_instance->p_api,  FSP_ERR_ASSERTION);
    FSP_ERROR_RETURN(NULL != p_cfg->p_instance->p_ctrl, FSP_ERR_ASSERTION);


    /* Get the I2C bus extended configuration from the communications instance. */
    p_extend = (rs_i2c_if_bus_extended_cfg_t const *) p_cfg->p_instance->p_cfg->p_extend;
    FSP_ERROR_RETURN(NULL != p_extend, FSP_ERR_ASSERTION);

    /* Get the underlying I2C master driver instance used by the communications middleware. */
    p_driver_instance = (i2c_master_instance_t const *) p_extend->p_driver_instance;
    FSP_ERROR_RETURN(NULL != p_driver_instance, FSP_ERR_ASSERTION);


    if (!isInitialized)
    {
        /* First call: initialize the I2C driver and the sensor */
        /* Open the ISL76682 instance. This must be done before calling any ISL76682 API. */
        err = p_driver_instance->p_api->open(p_driver_instance->p_ctrl, p_driver_instance->p_cfg);
        if (FSP_SUCCESS != err)
        {
            return err;
        }

        err = RM_ISL76682_Open(p_ctrl, p_cfg);
        if (FSP_SUCCESS != err)
        {
            /* Roll back: close the I2C driver so a retry starts from a clean state. */
            (void) p_driver_instance->p_api->close(p_driver_instance->p_ctrl);
            return err;
        }

        /* Configure the COMMAND register (enable, mode, light sense, resolution and lux range). */
        err = RM_ISL76682_SetCmdReg(p_ctrl, p_user_setting);
        if (FSP_SUCCESS != err)
        {
            /* Roll back: close the middleware and the I2C driver so a retry starts from a clean state. */
            close_err = RM_ISL76682_Close(p_ctrl);
            (void) close_err;
            (void) p_driver_instance->p_api->close(p_driver_instance->p_ctrl);
            return err;
        }

        /* Mark initialization as complete so later calls go straight to reading. */
        isInitialized = INITIALIZED;
    }
    else
    {
        /* Already initialized: read the light level in lux and return the result directly */
        return (RM_ISL76682_ReadLightLux(p_ctrl, (ra_isl76682_setting_t * const)p_user_setting,
                                        (float * const)data));
    }

    return err;
}

