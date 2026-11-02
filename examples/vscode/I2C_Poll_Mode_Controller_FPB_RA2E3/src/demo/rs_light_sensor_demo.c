/************************************************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hal_data.h"
#include "rs_i2c_if_cfg_data.h"
#include "rs_i2c_if.h"
#include "rm_isl76682.h"
#include "rm_isl76682_api.h"
#include "ra_isl76682_profile.h"
#include "sensor_ctrl.h"


#define RESET_VALUE (0U)
#define SET_VALUE (1U)

#ifndef isl76682_callback
void isl76682_callback(rm_isl76682_callback_args_t *p_args);
#endif
/* Version: MAJOR.MINOR.PATCH */
#define RS_LIGHT_SENSOR_DEMO_VERSION_MAJOR    (1U)
#define RS_LIGHT_SENSOR_DEMO_VERSION_MINOR    (0U)
#define RS_LIGHT_SENSOR_DEMO_VERSION_PATCH    (0U)
#define RS_LIGHT_SENSOR_DEMO_VERSION_STRING   "1.0.0"

#define BANNER      "\r\n******************************************************************"\
                    "\r\n   I2C Master Controller in Polling Mode Demo with ISL76682 sensor"\
                    "\r\n   Demo Version "RS_LIGHT_SENSOR_DEMO_VERSION_STRING""\
                    "\r\n******************************************************************"\
                    "\r\n"

#define BANNER_1    "\r\n*   Flex Software Pack Version  %d.%d.%d                            *"\
                    "\r\n"

#define BANNER_2    "\r\n******************************************************************"

#define BANNER_3    "\r\nRefer to documentation file for more details on Demo Project and" \
                    "\r\nFSP User's Manual for more information about module drivers\r\n"

#define BANNER_4    "\r\nThis demonstration showcases I2C Master-to-Slave communication using"\
                    "\r\npolling mode. An external light sensor acts as the I2C slave device."\
                    "\r\nThe RA MCU periodically reads sensor measurement data and outputs the"\
                    "\r\nresults to the console. Successful retrieval and display of the sensor"\
                    "\r\nvalues verify reliable I2C communication between the MCU and the"\
                    "\r\nlight sensor."\
                    "\r\n******************************************************************\r\n"

#define BANNER_5    "\r\nUsers can view the informational and error messages on the console\r\n"\
                    "\r\n******************************************************************\r\n"\
/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
static volatile uint8_t timer_flag = 0;
static volatile uint8_t counter = 0;
static char dataBuff[30];

extern const  rs_i2c_if_instance_t g_i2c_if_device ;
rs_i2c_if_light_sensor_ctrl_t     g_light_sensor_sram;     /* Generated writable SRAM copy */


ra_isl76682_setting_t gs_isl76682_user_setting =
{ .external_scale_res_kohm = ISL76682_EXTERNAL_SCALE_RES_KOHM_USER,
  .cmd_power = ISL76682_CMD_POWER_USER,
  .cmd_mode = ISL76682_CMD_MODE_USER,
  .cmd_lightsense = ISL76682_CMD_LIGHTSENSE_USER,
  .cmd_timing_resolution = ISL76682_CMD_RESOLUTION_USER,
  .cmd_lux_range = ISL76682_CMD_LUX_RANGE_USER,
};

rm_isl76682_instance_ctrl_t g_isl76682_als_ctrl;

const rm_isl76682_cfg_t g_isl76682_als0_cfg =
{ .p_instance = &g_i2c_if_device, .p_callback = isl76682_callback,
#if defined(NULL)
    .p_context           = NULL,
#else
  .p_context = &NULL,
#endif
};

const rm_isl76682_instance_t g_isl76682_als0 =
{ .p_ctrl = &g_isl76682_als_ctrl, .p_cfg = &g_isl76682_als0_cfg, .p_api = &g_isl76682_on_isl76682, };


/**********************************************************************************************************************
 * Public Function Prototypes
 **********************************************************************************************************************/
fsp_err_t sensor_delay_timer_setup(void);
void timer_callback(timer_callback_args_t *p_args);
fsp_err_t rs_isl76682_Read_light_sensor_data(rm_isl76682_ctrl_t *p_api_ctrl, rm_isl76682_cfg_t  *p_cfg,
                                             ra_isl76682_setting_t *p_user_setting, float *data);

/*******************************************************************************************************************//**
 * @brief   Initializes and starts the sensor delay timer.
 *
 * This function opens the AGT (Asynchronous General Purpose Timer) module
 * using the predefined configuration and starts the timer operation. The
 * timer is typically used to generate periodic delays for ambient light
 * sensor sampling or other time-based application tasks.
 *
 * @retval  FSP_SUCCESS        Timer initialized and started successfully.
 * @retval  Otherwise          Error code returned by the AGT driver.
 *
 * @note    Any initialization or startup errors are returned to the caller
 *          for application-level handling.
 **********************************************************************************************************************/
fsp_err_t sensor_delay_timer_setup(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Initializes the module. */
    err = R_AGT_Open(&sensor_delay_ctrl, &sensor_delay_cfg);
    /* Handle any errors. in the Application level by the user. */
    if(err)
    {
        return err;
    }
    /* Start the timer. */
    err = R_AGT_Start(&sensor_delay_ctrl);
    /* Handle any errors. in the Application level by the user. */
    return err;
}

/*******************************************************************************************************************//**
 * @brief   AGT timer callback function.
 *
 * This callback function is invoked automatically when the AGT timer
 * period expires. It sets a software flag that can be monitored by the
 * application to indicate that the configured timer interval has elapsed.
 *
 * @param[in] p_args    Pointer to callback arguments provided by the timer
 *                      driver (not used in this implementation).
 *
 * @note    The callback execution should be kept short and efficient since
 *          it runs in an interrupt context.
 *
 * @return  None
 **********************************************************************************************************************/
void timer_callback(timer_callback_args_t *p_args)
{

    if (NULL != p_args)
    {
        if (TIMER_EVENT_CYCLE_END == p_args->event)
        {
            if(0xFF == counter)
            {
            /* Set event flag on Periodic timer interrupt  */
                timer_flag = SET_VALUE;
            }
            counter++;
        }
    }
}


/***********************************************************************************************************************
 * Functions : Start_demo()
 **********************************************************************************************************************/

void rs_isl76682_SensorMeasurementEntry(void)
{
    fsp_err_t err = FSP_SUCCESS;
    float light_lux = 0.0;
    fsp_pack_version_t version = {RESET_VALUE};

    /* version get API for FLEX pack information */
    R_FSP_VersionGet(&version);

    printf(BANNER);
    printf(BANNER_1,version.version_id_b.major, version.version_id_b.minor, version.version_id_b.patch);
    printf(BANNER_2);
    printf(BANNER_3);
    printf(BANNER_4);
    printf(BANNER_5);

    memset(dataBuff, RESET_VALUE, sizeof(dataBuff));

    /* Generate configuration for the light sensor in SRAM */
    err = rs_i2c_if_LightSensorConfigGen(&g_i2c_if_device, &g_isl76682_als0, &g_light_sensor_sram);
    if (err != FSP_SUCCESS)
    {
        __BKPT(0);
    }

    /* Verify the configuration */
    err = rs_i2c_if_LightSensorConfigCheck(&g_i2c_if_device, &g_isl76682_als0, &g_light_sensor_sram);
    if (err != FSP_SUCCESS)
    {
        __BKPT(1);
    }

    /* Setup Periodic Sensor Processing Timer */
    err = sensor_delay_timer_setup();
    if (FSP_SUCCESS != err)
    {
        printf("\r\nPeriodic Sensor Processing Timer Setup failed!\r\n");
        __asm("BKPT #0\n");
    }
    printf("\r\nPeriodic Sensor Processing Timer Setup is successfully completed !!! \r\n");

    while(1)
    {
        if(timer_flag){

            err = rs_isl76682_Read_light_sensor_data(g_isl76682_als0.p_ctrl, (rm_isl76682_cfg_t *)g_isl76682_als0.p_cfg, &gs_isl76682_user_setting, &light_lux);
            if (FSP_SUCCESS == err)
            {
                printf("*****************ISL76682 Light Measurement Result***************\r\n");
                printf("Light Lux: %.1f\r\n",  light_lux);
            }
            else
            {
                 printf("Error Reading the Light Sensor !!!\r\n");
            }
            timer_flag = RESET_VALUE;
        }
    }
}
