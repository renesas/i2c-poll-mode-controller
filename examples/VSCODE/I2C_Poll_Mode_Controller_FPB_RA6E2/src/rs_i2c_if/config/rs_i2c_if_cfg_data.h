/* generated common header file - do not edit */
#ifndef RS_I2C_IF_CFG_DATA_H_
#define RS_I2C_IF_CFG_DATA_H_

#include "r_iic_b_master.h"
#include "rs_i2c_if.h"

FSP_HEADER
/* I2C Master on IIC Instance. */
/** Access the I2C Master instance using these structures when calling API functions directly (::p_api is not used). */
extern const i2c_master_instance_t g_i2c_master;
extern iic_b_master_instance_ctrl_t g_i2c_master_ctrl;
extern const i2c_master_cfg_t g_i2c_master_cfg;

#ifndef rs_i2c_if_callback
void rs_i2c_if_callback(i2c_master_callback_args_t *p_args);
#endif
/* I2C Shared Bus */
extern rs_i2c_if_bus_extended_cfg_t g_rs_i2c_if_bus_extended_cfg;
/* I2C Interface Bus */
extern const  rs_i2c_if_instance_t g_i2c_if_device ;

FSP_FOOTER
#endif /* RS_I2C_IF_CFG_DATA_H_ */
