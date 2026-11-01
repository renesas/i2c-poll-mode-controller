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

/*******************************************************************************************************************//**
 * @ingroup RENESAS_INTERFACES
 * @defgroup RM_ISL76682_API ISL76682 Middleware Interface
 * @brief Interface for ISL76682 Middleware functions.
 *
 * @section RM_ISL76682_API_Summary Summary
 * The ISL76682 interface provides ISL76682 functionality.
 *
 * The ISL76682 interface can be implemented by:
 * - @ref RM_ISL76682
 *
 * @{
 **********************************************************************************************************************/

#ifndef RM_ISL76682_API_H_
#define RM_ISL76682_API_H_

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#if defined(__CCRX__) || defined(__ICCRX__) || defined(__RX__)
 #include <string.h>
 #include "platform.h"
#elif defined(__CCRL__) || defined(__ICCRL__) || defined(__RL78__)
 #include <string.h>
 #include "r_cg_macrodriver.h"
 #include "r_fsp_error.h"
#else
 #include "bsp_api.h"
#endif

#include <rs_i2c_if_api.h>

#if defined(__CCRX__) || defined(__ICCRX__) || defined(__RX__)
#elif defined(__CCRL__) || defined(__ICCRL__) || defined(__RL78__)
#else

/* Common macro for FSP header files. There is also a corresponding FSP_FOOTER macro at the end of this file. */
FSP_HEADER
#endif

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
//ISL76682 register
#define REG_COMMAND_ADDR            (0x00)
#define REG_DATA_ADDR               (0x01)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** Enable of ISL76682  */
typedef enum e_rm_isl76682_cmd_power
{
    RM_ISL76682_POWERDOWN = 0,
    RM_ISL76682_ENABLE    = 1,
} rm_isl76682_cmd_power_t;
/** Measurement Mode of ISL76682 */
typedef enum e_rm_isl76682_cmd_mode
{
    RM_ISL76682_MODE_ONETIME    = 0,
    RM_ISL76682_MODE_CONTINUOUS = 1,
} rm_isl76682_cmd_mode_t;
/** Light Sensing of ISL76682 */
typedef enum e_rm_isl76682_cmd_lightsense
{
    RM_ISL76682_LIGHT_AMBIENT    = 0,
    RM_ISL76682_LIGHT_INFRARED   = 1,
} rm_isl76682_cmd_lightsense_t;
/** Timing Mode and Resolution of ISL76682 */
typedef enum e_rm_isl76682_cmd_timing_resolution
{
    RM_ISL76682_INTERNAL_TIMING_16BIT_ADC       = 0x00,
    RM_ISL76682_INTERNAL_TIMING_12BIT_ADC       = 0x01,
    RM_ISL76682_INTERNAL_TIMING_8BIT_ADC        = 0x02,
    RM_ISL76682_INTERNAL_TIMING_4BIT_ADC        = 0x03,
    RM_ISL76682_EXTERNAL_TIMING_ADC_DATAOUT     = 0x04,
    RM_ISL76682_EXTERNAL_TIMING_TIMER_DATAOUT   = 0x05,
    RM_ISL76682_TIMING_RESOLUTION_RESERVED1     = 0x06,
    RM_ISL76682_TIMING_RESOLUTION_RESERVED2     = 0x07,
} rm_isl76682_cmd_timing_resolution_t;
/** Range/FSR Lux of ISL76682 */
typedef enum e_rm_isl76682_cmd_lux_range
{
    RM_ISL76682_LUX_FULLSCAL_RANGE1_1000       = 0x00,
    RM_ISL76682_LUX_FULLSCAL_RANGE2_4000       = 0x01,
    RM_ISL76682_LUX_FULLSCAL_RANGE3_16000      = 0x02,
    RM_ISL76682_LUX_FULLSCAL_RANGE4_64000      = 0x03,
} rm_isl76682_cmd_lux_range_t;


typedef struct st_ra_isl76682_setting
{
    float                                     external_scale_res_kohm;
    rm_isl76682_cmd_power_t                   cmd_power;
    rm_isl76682_cmd_mode_t                    cmd_mode;
    rm_isl76682_cmd_lightsense_t              cmd_lightsense;
    rm_isl76682_cmd_timing_resolution_t       cmd_timing_resolution;
    rm_isl76682_cmd_lux_range_t               cmd_lux_range;

} ra_isl76682_setting_t;

/** Event in the callback function */
typedef enum e_rm_isl76682_event
{
    RM_ISL76682_EVENT_SUCCESS = 0,
    RM_ISL76682_EVENT_ERROR,
} rm_isl76682_event_t;

/** ISL76682 callback parameter definition */
typedef struct st_rm_isl76682_callback_args
{
    void const      * p_context;
    rm_isl76682_event_t event;
} rm_isl76682_callback_args_t;


/** ISL76682 Configuration */
typedef struct st_rm_isl76682_cfg
{
    rs_i2c_if_instance_t const * p_instance;                  ///< Pointer to Communications Middleware instance.
    void const                * p_context;                   ///< Pointer to the user-provided context.
    void const                * p_extend;                    ///< Pointer to extended configuration by instance of interface.
    void (* p_callback)(rm_isl76682_callback_args_t * p_args); ///< Pointer to callback function.
} rm_isl76682_cfg_t;

/** ISL76682 control block.  Allocate an instance specific control block to pass into the ISL76682 API calls.
 * @par Implemented as
 * - rm_isl76682_instance_ctrl_t
 */
typedef void rm_isl76682_ctrl_t;

/** ISL76682 APIs */
typedef struct st_rm_isl76682_api
{
    /** Open sensor.
     * @par Implemented as
     * - @ref RM_ISL76682_Open()
     *
     * @param[in]  p_ctrl       Pointer to control structure.
     * @param[in]  p_cfg        Pointer to configuration structure.
     */
    fsp_err_t (* open)(rm_isl76682_ctrl_t * const p_ctrl, rm_isl76682_cfg_t const * const p_cfg);

    /** Close ISL76682.
     * @par Implemented as
     * - @ref RM_ISL76682_Close()
     *
     * @param[in]  p_ctrl       Pointer to control structure.
     */
    fsp_err_t (* close)(rm_isl76682_ctrl_t * const p_ctrl);

    /** Read the Register data from ISL76682.
     * @par Implemented as
     * - @ref RM_ISL76682_ReadReg()
     *
     * @param[in]  p_ctrl           Pointer to control structure.
     * @param[in]  p_raw_data       Pointer to raw data structure.
     */
    fsp_err_t (* readReg)(rm_isl76682_ctrl_t * const p_ctrl, uint8_t * const reg_addr,uint8_t * const reg_data);

    /** Write data to the Register of ISL76682.
     * @par Implemented as
     * - @ref RM_ISL76682_WriteReg()
     *
     * @param[in]  p_ctrl           Pointer to control structure.
     * @param[in]  p_raw_data       Pointer to raw data structure.
     */
    fsp_err_t (* writeReg)(rm_isl76682_ctrl_t * const p_ctrl, uint8_t * const reg_addr,uint8_t * const reg_data);

    /** Set the CMD register of ISL76682.
        * @par Implemented as
        * - @ref RM_ISL76682_SetCmdReg()
        *
        * @param[in]  p_ctrl           Pointer to control structure.
        * @param[in]  p_raw_data       Pointer to raw data structure.
    */
    fsp_err_t (* setCmdReg)(rm_isl76682_ctrl_t * const p_api_ctrl, ra_isl76682_setting_t * const setting);

    /** Read Lux reading from ISL76682.
     * @par Implemented as
     * - @ref RM_ISL76682_ReadLightLux()
     *
     * @param[in]  p_ctrl           Pointer to control structure.
     * @param[in]  p_raw_data       Pointer to raw data structure.
     */
    fsp_err_t (* readLightLux)(rm_isl76682_ctrl_t * const p_ctrl, ra_isl76682_setting_t * const setting, float * const LightLux);

} rm_isl76682_api_t;

/** ISL76682 instance */
typedef struct st_rm_isl76682_instance
{
    rm_isl76682_ctrl_t      * p_ctrl;    /**< Pointer to the control structure for this instance */
    rm_isl76682_cfg_t const * p_cfg;     /**< Pointer to the configuration structure for this instance */
    rm_isl76682_api_t const * p_api;     /**< Pointer to the API structure for this instance */
} rm_isl76682_instance_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/

/**********************************************************************************************************************
 * Public Function Prototypes
 **********************************************************************************************************************/
#ifndef rm_isl76682_callback
void rm_isl76682_callback(rs_i2c_if_callback_args_t *p_args);
#endif

#if defined(__CCRX__) || defined(__ICCRX__) || defined(__RX__)
#elif defined(__CCRL__) || defined(__ICCRL__) || defined(__RL78__)
#else

/* Common macro for FSP header files. There is also a corresponding FSP_FOOTER macro at the end of this file. */
FSP_FOOTER
#endif

#endif                                 /* RM_ISL76682_API_H_*/

/*******************************************************************************************************************//**
 * @} (end addtogroup RM_ISL76682_API)
 **********************************************************************************************************************/
