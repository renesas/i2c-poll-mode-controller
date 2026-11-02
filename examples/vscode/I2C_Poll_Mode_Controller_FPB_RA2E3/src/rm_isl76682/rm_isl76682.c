/***********************************************************************************************************************
 * Copyright [2020-2023] Renesas Electronics Corporation and/or its affiliates.  All Rights Reserved.
 *
 * This software and documentation are supplied by Renesas Electronics America Inc. and may only be used with products
 * of Renesas Electronics Corp. and its affiliates ("Renesas").  No other uses are authorized.  Renesas products are
 * sold pursuant to Renesas terms and conditions of sale.  Purchasers are solely responsible for the selection and use
 * of Renesas products and Renesas assumes no liability.  No license, express or implied, to any intellectual property
 * right is granted by Renesas. This software is protected under all applicable laws, including copyright laws. Renesas
 * reserves the right to change or discontinue this software and/or this documentation. THE SOFTWARE AND DOCUMENTATION
 * IS DELIVERED TO YOU "AS IS," AND RENESAS MAKES NO REPRESENTATIONS OR WARRANTIES, AND TO THE FULLEST EXTENT
 * PERMISSIBLE UNDER APPLICABLE LAW, DISCLAIMS ALL WARRANTIES, WHETHER EXPLICITLY OR IMPLICITLY, INCLUDING WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT, WITH RESPECT TO THE SOFTWARE OR
 * DOCUMENTATION.  RENESAS SHALL HAVE NO LIABILITY ARISING OUT OF ANY SECURITY VULNERABILITY OR BREACH.  TO THE MAXIMUM
 * EXTENT PERMITTED BY LAW, IN NO EVENT WILL RENESAS BE LIABLE TO YOU IN CONNECTION WITH THE SOFTWARE OR DOCUMENTATION
 * (OR ANY PERSON OR ENTITY CLAIMING RIGHTS DERIVED FROM YOU) FOR ANY LOSS, DAMAGES, OR CLAIMS WHATSOEVER, INCLUDING,
 * WITHOUT LIMITATION, ANY DIRECT, CONSEQUENTIAL, SPECIAL, INDIRECT, PUNITIVE, OR INCIDENTAL DAMAGES; ANY LOST PROFITS,
 * OTHER ECONOMIC DAMAGE, PROPERTY DAMAGE, OR PERSONAL INJURY; AND EVEN IF RENESAS HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH LOSS, DAMAGES, CLAIMS OR COSTS.
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "rm_isl76682.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/* Definitions of Open flag */
#define RM_ISL76682_OPEN                               (0x433E4432UL) // Open state


/* Definitions of Timeout */
#define RM_ISL76682_TIMEOUT                            (1000)

/* Definitions of Wait Time */
#define RM_ISL76682_WAIT_TIME_100                      (100)
#define RM_ISL76682_WAIT_TIME_120                      (120)
#define RM_ISL76682_WAIT_TIME_500                      (500)
#define RM_ISL76682_WAIT_TIME_1000                     (1000)



/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Global function prototypes
 **********************************************************************************************************************/

void rm_isl76682_callback(rs_i2c_if_callback_args_t * p_args);
extern void isl76682_callback(rm_isl76682_callback_args_t *p_args);
/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
extern fsp_err_t rm_isl76682_delay_us(rm_isl76682_instance_ctrl_t * const p_ctrl, uint32_t const delay_us);

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Global variables
 **********************************************************************************************************************/
rm_isl76682_api_t const g_isl76682_on_isl76682 =
{
    .open                 = RM_ISL76682_Open,
    .close                = RM_ISL76682_Close,
    .readReg              = RM_ISL76682_ReadReg,
    .writeReg             = RM_ISL76682_WriteReg,
    .setCmdReg            = RM_ISL76682_SetCmdReg,
    .readLightLux         = RM_ISL76682_ReadLightLux,
};

/*******************************************************************************************************************//**
 * @addtogroup RM_ISL76682
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief Opens and configures the ISL76682 Middle module. Implements @ref rm_isl76682_api_t::open.
 *
 * Example:
 * @snippet RM_ISL76682_Open
 *
 * @retval FSP_SUCCESS              ISL76682 successfully configured.
 * @retval FSP_ERR_ASSERTION        Null pointer, or one or more configuration options is invalid.
 * @retval FSP_ERR_ALREADY_OPEN     Module is already open.  This module can only be opened once.
 **********************************************************************************************************************/
fsp_err_t RM_ISL76682_Open (rm_isl76682_ctrl_t * const p_api_ctrl, rm_isl76682_cfg_t const * const p_cfg)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_isl76682_instance_ctrl_t * p_ctrl = (rm_isl76682_instance_ctrl_t *) p_api_ctrl;

#if RM_ISL76682_CFG_PARAM_CHECKING_ENABLE
    FSP_ASSERT(NULL != p_ctrl);
    FSP_ASSERT(NULL != p_cfg);
    FSP_ASSERT(NULL != p_cfg->p_instance);
    FSP_ERROR_RETURN(RM_ISL76682_OPEN != p_ctrl->open, FSP_ERR_ALREADY_OPEN);
#endif

    p_ctrl->p_cfg                  = p_cfg;
    p_ctrl->p_comms_i2c_instance   = p_cfg->p_instance;
    p_ctrl->p_context              = p_cfg->p_context;
    p_ctrl->p_callback             = p_cfg->p_callback;

    /* Open Communications middleware */
    err = p_ctrl->p_comms_i2c_instance->p_api->open(p_ctrl->p_comms_i2c_instance->p_ctrl,
                                                    p_ctrl->p_comms_i2c_instance->p_cfg);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    /* Set open flag */
    p_ctrl->open = RM_ISL76682_OPEN;

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Disables specified ISL76682 control block. Implements @ref rm_isl76682_api_t::close.
 *
 * @retval FSP_SUCCESS              Successfully closed.
 * @retval FSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval FSP_ERR_NOT_OPEN         Module is not open.
 **********************************************************************************************************************/
fsp_err_t RM_ISL76682_Close (rm_isl76682_ctrl_t * const p_api_ctrl)
{
    rm_isl76682_instance_ctrl_t * p_ctrl = (rm_isl76682_instance_ctrl_t *) p_api_ctrl;

#if RM_ISL76682_CFG_PARAM_CHECKING_ENABLE
    FSP_ASSERT(NULL != p_ctrl);
    FSP_ERROR_RETURN(RM_ISL76682_OPEN == p_ctrl->open, FSP_ERR_NOT_OPEN);
#endif

    /* Close Communications Middleware */
    p_ctrl->p_comms_i2c_instance->p_api->close(p_ctrl->p_comms_i2c_instance->p_ctrl);

    /* Clear Open flag */
    p_ctrl->open = 0;

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Write data to the Register.
 * Implements @ref rm_isl76682_api_t::writeReg.
 *
 * @retval FSP_SUCCESS              Successfully started.
 * @retval FSP_ERR_ASSERTION        Null pointer passed as a parameter.
 * @retval FSP_ERR_NOT_OPEN         Module is not open.
 **********************************************************************************************************************/
fsp_err_t RM_ISL76682_WriteReg (rm_isl76682_ctrl_t * const p_api_ctrl,uint8_t * const reg_addr,uint8_t * const p_reg_data)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_isl76682_instance_ctrl_t * p_ctrl = (rm_isl76682_instance_ctrl_t *) p_api_ctrl;
    uint16_t counter = 0;

#if RM_ISL76682_CFG_PARAM_CHECKING_ENABLE
    FSP_ASSERT(NULL != p_ctrl);
    FSP_ERROR_RETURN(RM_ISL76682_OPEN == p_ctrl->open, FSP_ERR_NOT_OPEN);
#endif
    /* Write */
    p_ctrl->buf[0] = *reg_addr;
    p_ctrl->buf[1] = *p_reg_data;

    p_ctrl->communication_finished = false;
    err            =
        p_ctrl->p_comms_i2c_instance->p_api->write(p_ctrl->p_comms_i2c_instance->p_ctrl, &p_ctrl->buf[0], 1);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    /* Wait the callback */
    while (false == p_ctrl->communication_finished)
    {
#if BSP_CFG_RTOS == 2
        vTaskDelay(1);
#else
        R_BSP_SoftwareDelay(RM_ISL76682_WAIT_TIME_100, BSP_DELAY_UNITS_MICROSECONDS);
#endif
        counter++;
        FSP_ERROR_RETURN(RM_ISL76682_TIMEOUT >= counter, FSP_ERR_TIMEOUT);
    }

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Reads the register data from ISL76682.
 * Implements @ref rm_isl76682_api_t::readReg.
 *
 * @retval FSP_SUCCESS              Successfully data decoded.
 * @retval FSP_ERR_ASSERTION        Null pointer, or one or more configuration options is invalid.
 * @retval FSP_ERR_NOT_OPEN         Module is not open.
 **********************************************************************************************************************/
fsp_err_t RM_ISL76682_ReadReg (rm_isl76682_ctrl_t * const p_api_ctrl, uint8_t * const reg_addr,uint8_t * const p_reg_data)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_isl76682_instance_ctrl_t * p_ctrl = (rm_isl76682_instance_ctrl_t *) p_api_ctrl;
    uint16_t counter = 0;

#if RM_ISL76682_CFG_PARAM_CHECKING_ENABLE
    FSP_ASSERT(NULL != p_ctrl);
    FSP_ASSERT(NULL != p_raw_data);
    FSP_ERROR_RETURN(RM_ISL76682_OPEN == p_ctrl->open, FSP_ERR_NOT_OPEN);
#endif

    /* Write */
    p_ctrl->buf[0] = *reg_addr;
    p_ctrl->communication_finished = false;
    err            =
        p_ctrl->p_comms_i2c_instance->p_api->write(p_ctrl->p_comms_i2c_instance->p_ctrl, &p_ctrl->buf[0], 1);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    /* Wait the callback */
    while (false == p_ctrl->communication_finished)
    {
#if BSP_CFG_RTOS == 2
        vTaskDelay(1);
#else
        R_BSP_SoftwareDelay(RM_ISL76682_WAIT_TIME_100, BSP_DELAY_UNITS_MICROSECONDS);
#endif
        counter++;
        FSP_ERROR_RETURN(RM_ISL76682_TIMEOUT >= counter, FSP_ERR_TIMEOUT);
    }
#if BSP_CFG_RTOS == 2
        vTaskDelay(1);
#else
    /* Delay 120us */
    R_BSP_SoftwareDelay(RM_ISL76682_WAIT_TIME_120, BSP_DELAY_UNITS_MICROSECONDS);
#endif
    /* Read ISL76682 data */
    p_ctrl->communication_finished = false;
    err =
         p_ctrl->p_comms_i2c_instance->p_api->read(p_ctrl->p_comms_i2c_instance->p_ctrl,
                                                     p_reg_data,
                                                      2);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    counter = 0;
    while (false == p_ctrl->communication_finished)
    {
#if BSP_CFG_RTOS == 2
        vTaskDelay(1);
#else
        R_BSP_SoftwareDelay(RM_ISL76682_WAIT_TIME_100, BSP_DELAY_UNITS_MICROSECONDS);
#endif
        counter++;
        FSP_ERROR_RETURN(RM_ISL76682_TIMEOUT >= counter, FSP_ERR_TIMEOUT);
    }

    return FSP_SUCCESS;
}


/*******************************************************************************************************************//**
 * @brief Write COMMAND register of ISL76682
 *  BIT  D7    D6    D5     D4    D3    D2    D1      D0
 *  NAME EN    MODE  LIGHT  RES2  RES2  RES2  RANGE1  RANGE1
 * Implements @ref rm_isl76682_api_t::setCmdReg.
 *
 * @retval FSP_SUCCESS              Successfully data decoded.
 * @retval FSP_ERR_ASSERTION        Null pointer, or one or more configuration options is invalid.
 * @retval FSP_ERR_NOT_OPEN         Module is not open.
 **********************************************************************************************************************/
fsp_err_t RM_ISL76682_SetCmdReg (rm_isl76682_ctrl_t * const p_api_ctrl, ra_isl76682_setting_t * const setting)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_isl76682_instance_ctrl_t * p_ctrl = (rm_isl76682_instance_ctrl_t *) p_api_ctrl;
    uint16_t counter = 0;

#if RM_ISL76682_CFG_PARAM_CHECKING_ENABLE
    FSP_ASSERT(NULL != p_ctrl);
    FSP_ASSERT(NULL != p_raw_data);
    FSP_ERROR_RETURN(RM_ISL76682_OPEN == p_ctrl->open, FSP_ERR_NOT_OPEN);
#endif

    uint8_t  reg_data_val;

    reg_data_val = (uint8_t)(((setting->cmd_power) << REG_CMD_EN_SHIFT)|
                             (setting->cmd_mode << REG_CMD_MODE_SHIFT)|
                             (setting->cmd_lightsense << REG_CMD_LIGHT_SHIFT)|
                             (setting->cmd_timing_resolution << REG_CMD_RESOLUTION_SHIFT)|
                             (setting->cmd_lux_range << REG_CMD_RANGE_SHIFT));

    /* Write */
    p_ctrl->buf[0] = REG_COMMAND_ADDR;
    p_ctrl->buf[1] = reg_data_val;
    p_ctrl->communication_finished = false;
    err            =
        p_ctrl->p_comms_i2c_instance->p_api->write(p_ctrl->p_comms_i2c_instance->p_ctrl, &p_ctrl->buf[0], 2);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    while (false == p_ctrl->communication_finished)
    {
#if BSP_CFG_RTOS == 2
        vTaskDelay(1);
#else
        R_BSP_SoftwareDelay(RM_ISL76682_WAIT_TIME_100, BSP_DELAY_UNITS_MICROSECONDS);
#endif
        counter++;
        FSP_ERROR_RETURN(RM_ISL76682_TIMEOUT >= counter, FSP_ERR_TIMEOUT);
    }
    return FSP_SUCCESS;
}



/*******************************************************************************************************************//**
 * @brief Reads the LUX reading from ISL76682.
 * Implements @ref rm_isl76682_api_t::readLightLux.
 *
 * @retval FSP_SUCCESS              Successfully data decoded.
 * @retval FSP_ERR_ASSERTION        Null pointer, or one or more configuration options is invalid.
 * @retval FSP_ERR_NOT_OPEN         Module is not open.
 **********************************************************************************************************************/
fsp_err_t RM_ISL76682_ReadLightLux (rm_isl76682_ctrl_t * const p_api_ctrl, ra_isl76682_setting_t * const setting, float * const LightLux)
{
    fsp_err_t err = FSP_SUCCESS;
    rm_isl76682_instance_ctrl_t * p_ctrl = (rm_isl76682_instance_ctrl_t *) p_api_ctrl;
    uint8_t   reg_data[2] = {0,0};
    uint16_t  lux_temp;
    uint16_t counter = 0;
    float   lux_range_val;
    float   adc_count_max;
#if RM_ISL76682_CFG_PARAM_CHECKING_ENABLE
    FSP_ASSERT(NULL != p_ctrl);
    FSP_ASSERT(NULL != p_raw_data);
    FSP_ERROR_RETURN(RM_ISL76682_OPEN == p_ctrl->open, FSP_ERR_NOT_OPEN);
#endif

    /* Write */
    p_ctrl->buf[0] = REG_DATA_ADDR;
    p_ctrl->communication_finished = false;
    err            =
        p_ctrl->p_comms_i2c_instance->p_api->write(p_ctrl->p_comms_i2c_instance->p_ctrl, &p_ctrl->buf[0], 1);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    /* Wait the callback */
    while (false == p_ctrl->communication_finished)
    {
#if BSP_CFG_RTOS == 2
        vTaskDelay(1);
#else
        R_BSP_SoftwareDelay(RM_ISL76682_WAIT_TIME_100, BSP_DELAY_UNITS_MICROSECONDS);
#endif
        counter++;
        FSP_ERROR_RETURN(RM_ISL76682_TIMEOUT >= counter, FSP_ERR_TIMEOUT);
    }
#if BSP_CFG_RTOS == 2
        vTaskDelay(1);
#else
    /* Delay 120us */
    R_BSP_SoftwareDelay(RM_ISL76682_WAIT_TIME_120, BSP_DELAY_UNITS_MICROSECONDS);;
#endif
    /* Read ISL76682 data */
    p_ctrl->communication_finished = false;
    err =
         p_ctrl->p_comms_i2c_instance->p_api->read(p_ctrl->p_comms_i2c_instance->p_ctrl,
                                                   reg_data,
                                                      2);
    FSP_ERROR_RETURN(FSP_SUCCESS == err, err);

    counter = 0;
    while (false == p_ctrl->communication_finished)
    {
#if BSP_CFG_RTOS == 2
        vTaskDelay(1);
#else
        R_BSP_SoftwareDelay(RM_ISL76682_WAIT_TIME_100, BSP_DELAY_UNITS_MICROSECONDS);
#endif
        counter++;
        FSP_ERROR_RETURN(RM_ISL76682_TIMEOUT >= counter, FSP_ERR_TIMEOUT);
    }
    //LSD first, then MSB!!
    lux_temp = (uint16_t)((((uint16_t)reg_data[1]) << 8) | (uint16_t)reg_data[0]);


    switch (setting->cmd_lux_range)
    {
        case RM_ISL76682_LUX_FULLSCAL_RANGE1_1000:
        {
            lux_range_val = 1000;
        }
        break;
        case RM_ISL76682_LUX_FULLSCAL_RANGE2_4000:
        {
            lux_range_val = 4000;
        }
        break;
        case RM_ISL76682_LUX_FULLSCAL_RANGE3_16000:
        {
            lux_range_val = 16000;
        }
        break;
        case RM_ISL76682_LUX_FULLSCAL_RANGE4_64000:
        {
            lux_range_val = 64000;
        }
        break;
        default:
        {
            lux_range_val = 64000;
        }
        break;
    }

    switch (setting->cmd_timing_resolution)
    {
        case RM_ISL76682_INTERNAL_TIMING_16BIT_ADC:
        {
            adc_count_max = 65535;
        }
        break;
        case RM_ISL76682_INTERNAL_TIMING_12BIT_ADC:
        {
            adc_count_max = 4096;
        }
        break;
        case RM_ISL76682_INTERNAL_TIMING_8BIT_ADC:
        {
            adc_count_max = 256;
        }
        break;
        case RM_ISL76682_INTERNAL_TIMING_4BIT_ADC:
        {
            adc_count_max = 16;
        }
        break;
        default:
        {
            //in this code, External Timing Mode isn't handled.
            return FSP_ERR_ABORTED;
        }
        break;
    }

    *LightLux = ((float)((float)500/setting->external_scale_res_kohm))*((float)lux_temp)*lux_range_val/adc_count_max;

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief ISL76682 callback function called in the I2C Communications Middleware callback function.
 **********************************************************************************************************************/
void rm_isl76682_callback (rs_i2c_if_callback_args_t * p_args)
{
    rm_isl76682_instance_ctrl_t * p_ctrl = (rm_isl76682_instance_ctrl_t *) p_args->p_context;
    rm_isl76682_callback_args_t   isl76682_callback_args;

    /* Set context */
    isl76682_callback_args.p_context = p_ctrl->p_context;

    /* Set event */
    switch (p_args->event)
    {
        case RS_I2C_IF_EVENT_OPERATION_COMPLETE:
        {
            isl76682_callback_args.event = RM_ISL76682_EVENT_SUCCESS;
        }
        break;
        case RS_I2C_IF_EVENT_ERROR:
        default:
        {
            isl76682_callback_args.event = RM_ISL76682_EVENT_ERROR;
        }
        break;
    }

    if (NULL != p_ctrl->p_callback)
    {
        /* Call callback function */
#if 1
        /* p_ctrl->p_context is NULL because this is code isn't generated by FSP Stack Configuration tool
           So, call isl76682_callback directly */
        isl76682_callback(&isl76682_callback_args);
#else
        p_ctrl->p_callback(&isl76682_callback_args);
#endif
    }
}
/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * Internal isl76682 private function.
 **********************************************************************************************************************/

