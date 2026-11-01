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
 * @addtogroup RM_ISL76682
 * @{
 **********************************************************************************************************************/

#ifndef RM_ISL76682_H
#define RM_ISL76682_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "rm_isl76682_api.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
/* Common macro for FSP header files. There is also a corresponding FSP_FOOTER macro at the end of this file. */
FSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define REG_CMD_RANGE_MASK       (0x0003)
#define REG_CMD_RANGE_SHIFT      (0)
#define REG_CMD_RESOLUTION_MASK  (0x0007)
#define REG_CMD_RESOLUTION_SHIFT (3)
#define REG_CMD_LIGHT_MASK       (0x0001)
#define REG_CMD_LIGHT_SHIFT      (5)
#define REG_CMD_MODE_MASK        (0x0001)
#define REG_CMD_MODE_SHIFT       (6)
#define REG_CMD_EN_MASK          (0x0001)
#define REG_CMD_EN_SHIFT         (7)


/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** ISL76682 Control Block */
typedef struct rm_isl76682_instance_ctrl
{
    uint32_t                            open;                 ///< Open flag
    rm_isl76682_cfg_t const           * p_cfg;                ///< Pointer to ISL76682 Configuration
    rs_i2c_if_instance_t const        * p_comms_i2c_instance; ///< Pointer of I2C Communications Middleware instance structure
    void const                        * p_context;            ///< Pointer to the user-provided context
    uint8_t buf[3];                                           ///< Buffer for I2C communications
    volatile bool              communication_finished; ///< Communication flag for blocking.
    /* Pointer to callback and optional working memory */
    void (* p_callback)(rm_isl76682_callback_args_t * p_args);
} rm_isl76682_instance_ctrl_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/

/** @cond INC_HEADER_DEFS_SEC */
/** Filled in Interface API structure for this Instance. */
extern rm_isl76682_api_t const g_isl76682_on_isl76682;

/** @endcond */

/**********************************************************************************************************************
 * Public Function Prototypes
 **********************************************************************************************************************/
fsp_err_t RM_ISL76682_Open(rm_isl76682_ctrl_t * const p_api_ctrl, rm_isl76682_cfg_t const * const p_cfg);
fsp_err_t RM_ISL76682_Close(rm_isl76682_ctrl_t * const p_api_ctrl);

fsp_err_t RM_ISL76682_ReadReg  (rm_isl76682_ctrl_t * const p_api_ctrl, uint8_t * const reg_addr,uint8_t * const p_reg_data);
fsp_err_t RM_ISL76682_WriteReg (rm_isl76682_ctrl_t * const p_api_ctrl, uint8_t * const reg_addr,uint8_t * const p_reg_data);
fsp_err_t RM_ISL76682_SetCmdReg (rm_isl76682_ctrl_t * const p_api_ctrl, ra_isl76682_setting_t * const setting);
fsp_err_t RM_ISL76682_ReadLightLux (rm_isl76682_ctrl_t * const p_api_ctrl, ra_isl76682_setting_t * const setting, float * const LightLux);

#if defined(__CCRX__) || defined(__ICCRX__) || defined(__RX__)
#elif defined(__CCRL__) || defined(__ICCRL__) || defined(__RL78__)
#else
/* Common macro for FSP header files. There is also a corresponding FSP_FOOTER macro at the end of this file. */
FSP_FOOTER
#endif

#endif                                 /* RM_ISL76682_H_*/

/*******************************************************************************************************************//**
 * @} (end addtogroup RM_ISL76682)
 **********************************************************************************************************************/
