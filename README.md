# I2C Master Polling Mode - Sensor Communication Solution

## Table of Contents

* [1. Introduction](#1-introduction)
* [2. Hardware and Tools Support](#2-hardware-and-tools-support)
* [3. Importing and Building Application Project](#3-importing-and-building-application-project)
* [4. Project Execution and Expected Results](#4-project-execution-and-expected-results)
* [5. MCU and FSP Resource Use](#5-mcu-and-fsp-resource-use)
* [6. System Level Architecture](#6-system-level-architecture)
* [7. Periodic Light Sensor Data Read Sequence of events](#7-periodic-light-sensor-data-read-sequence-of-events)
* [8. Module Layer Code and Demonstration Code](#8-module-layer-code-and-demonstration-code)
* [9. Module and Sensor Driver Integration](#9-module-and-sensor-driver-integration)
* [10. Public Functions - Where to Start](#10-public-functions---where-to-start)
* [11. Adapting to Your Board](#11-adapting-to-your-board)

# 1. Introduction

The **I2C Master Polling Mode Sensor Communication Solution**  project demonstrates a polling-based I2C interface between a Renesas RA MCU and the QCIOT-ISL76682POCZ light sensor module. The MCU acts as the I2C master (controller) and periodically polls the sensor at a fixed interval to read its data. This polling approach provides a simple, deterministic, and easy-to-debug sensing solution with minimal software complexity. It is well suited to low-to-moderate sampling rates in Industrial IoT, Smart Building, Healthcare, and Consumer Electronics applications.


### Project Structure

```
.
├── index.htm / docs/README.md
│   └── Documentation for the I2C controller module with Polling control   
│        
├── Examples
│   └── VSCode
│       │   
│       │── I2C_Poll_Mode_Controller_FPB_RA4E2
│       │    └── Polling Based I2C Light sensor data acquisition for FPB-RA4E2
│       │── I2C_Poll_Mode_Controller_FPB_RA6E2
│       │    └── Polling Based I2C Light sensor data acquisition for FPB-RA6E2
│       │── I2C_Poll_Mode_Controller_FPB_RA2E3
│       │    └── Polling Based I2C Light sensor data acquisition for FPB-RA2E3
│       │
│       └── executable
│            └── Executables for the demo projects
├── src
│   │── demo
│   │    └── Demonstration projects
│   │ 
│   │── rm_isl76682
│   │    └── Driver for QCIOT-ISL76682POCZ light sensor
│   │ 
│   │── config   
│   │    └── Module configuration settings
│   │       
│   │── sensor_ctrl
│   │    └── Sensor control functions - Single entry function
│   │ 
│   │── rs_i2c_if.c
│   │        └── I2C controller module main source code
│   │── rs_i2c_if_ra.c
│   │        └── I2C controller module private function
│   └── rs_i2c_if.h
│           └── I2C controller module header file
│   
```
## Reference Design

The accompanying example project demonstrates polling-based I2C communication between an RA MCU and a slave device. It is available for the **FPB-RA4E2** , **FPB-RA6E2** and **FPB_RA2E3** boards, with the Renesas RA4E2 or RA6E2 or RA2E3 MCU acting as the I2C controller. The QCIOT-ISL76682POCZ light sensor serves as the reference target.

In poll mode, the application configures and starts the sensor, when the periodic timer expires, the I2C master, initiates a read from the sensor, a transfer and then polls a status flag until the transfer completes,  The application:

- Opens the FSP I2C master driver
- Sets the target device address
- Writes and reads bytes on the bus
- Reports each result on a serial console

Polling mode keeps the code small and predictable, which makes it a good fit for bare-metal projects. After starting an I2C transfer, the application simply checks a completion flag in a loop until the transfer finishes. This means it needs no RTOS objects such as semaphores, queues, or event groups, and no extra tasks or context switching. Because the program flow is sequential and easy to follow, the timing of each sensor read is easy to reason about, and debugging is simpler. The footprint is also small in both flash and RAM, so the approach suits resource-constrained MCUs.

This makes polling a good fit for a light sensor like the ISL76682, where the conversion time is known (about 100 ms at 16-bit resolution) and readings are needed only at a fixed interval. The application can start a measurement, wait for the transfer to complete, convert the result to lux, and report it on the serial console. The trade-off is that the CPU is busy while it waits on the flag, so polling is less power-efficient than an interrupt-driven or RTOS-based design. For battery-powered products, you can offset this by using the sensor’s one-time measurement mode and a longer reading interval.

## Key Features

- I2C controller (master) on the FPB-RA4E2, FPB-RA6E2 and FPB-RA2E3 with 7-bit addressing
- Poll mode: the application waits on a completion flag set by the driver callback
- Write, read, and write-then-read (repeated start) transfers
- Console output for results and errors
- Bus speed configured in the FSP Stack Configurator (standard 100 kHz or fast 400 kHz)

# 2. Hardware and Tools Support

| Hardware Item | Description |
| --- | --- |
| Reference Boards | FPB-RA4E2, FPB-RA6E2, FPB-RA2E3 |
| Sensor | QCIOT-ISL76682POCZ light sensor (I2C slave device) |
| Connecting Wires | Jumper wires for I2C connections (SDA, SCL, VCC, GND) between the board and the sensor |
| USB Cable | Micro USB cable for programming, debugging, and observing project operation on the serial console |

**Note:** Please ensure the I2C pull-up resistors are present on the SDA and SCL lines (on the sensor module or externally) and that the sensor supply voltage matches the board I/O voltage before running the project. QCIOT-ISL76682POCZ PMOD has option to use the I2C Pullup using the Jumpers.

### Software Requirements

- **Renesas Flexible Software Package (FSP):** Version 6.5.0 or later
- **VS Code/Renesas extensions for VS Code with Renesas 365 explorer studio** 
- **LLVM Embedded Toolchain for ARM:** Version 21.1.1
- **Terminal Console Application:** Tera Term or a similar application

## Hardware Connections for Light Sensor Module

Connections from the Light Sensor Module (QCIOT-021) to the FPB-RA4E2, FPB-RA6E2 and FPB-RA2E3 boards are as follows.

### FPB-RA4E2: Arduino Uno J2, J4 Connector

| Signal | Arduino Uno Pin # | Light Sensor Module J1 Pin # | Note |
| --- | --- | --- | --- |
| VCC | J2-Pin 2 | J1-Pin 6 |  |
| GND | J2-Pin 6 | J1-Pin 5 |  |
| SCL (I2C signal) | J4-Pin 10 | J1-Pin 3 | P100/SCL |
| SDA (I2C signal) | J4-Pin 9 | J1-Pin 4 | P101/SDA |

### FPB-RA6E2: Arduino Uno J2, J4 Connector

| Signal | Arduino Uno Pin # | Light Sensor Module J1 Pin # | Note |
| --- | --- | --- | --- |
| VCC | J2-Pin 2 | J1-Pin 6 |  |
| GND | J2-Pin 6 | J1-Pin 5 |  |
| SCL (I2C signal) | J4-Pin 10 | J1-Pin 3 | P100/SCL |
| SDA (I2C signal) | J4-Pin 9 | J1-Pin 4 | P101/SDA  |

### FPB-RA2E3 - Arduino Uno J2, J4 Connector

|Signal           | FPB-RA2E3 PMOD2 Pin# | Light Sensor Module J1 Pin |Note   | 
|:----------------|------------------|----------------------------|--------------|
|VCC              | #6               | J1-Pin 6                         |              |
|GND              | #5               | J1-Pin 5                         |              |
|I2C-SCL          | #3               | J1-Pin 3                         |P400 (SCL)    |
|I2C-SDA          | #4               | J1-Pin 4                         |P401 (SDA)    |

**Note:** Note: Ensure E26, E28, E46, and E47 are connected to enable MCU UART communication through the J-Link Virtual COM Port.

# 3. Importing and Building Application Project

For detailed instructions on importing, configuring, building, and running the demonstration project in Visual Studio Code, refer to the Renesas RA VS Code Documentation:

[creating-and-building-project.html#using-renesas-ra](https://tool-support.renesas.com/e2studio/vscode/docs/creating-and-building-project.html#using-renesas-ra)

#### Configure User Configurations (if default config needs change)

1. Open the `rs_light_sensor_demo.c` file located in the `src/demo` folder.
2. Update the `gs_isl76682_user_setting` to change the Lux range, mode, light sensing source (if the user wants to change the default setting).
3. Update/Change the Periodic Timer interval using r_agt --> Properties-> General -> Period	 to desired seconds

# 4. Project Execution and Expected Results

```
### Expected Console Output
******************************************************************
   I2C Master Controller in Polling Mode Demo with ISL76682 sensor
   Demo Version 1.0.0
******************************************************************

*   Flex Software Pack Version  6.5.0                            *

******************************************************************
Refer to documentation file for more details on Demo Project and
FSP User's Manual for more information about module drivers

This demonstration showcases I2C Master-to-Slave communication using
polling mode. An external light sensor acts as the I2C slave device.
The RA MCU periodically reads sensor measurement data and outputs the
results to the console. Successful retrieval and display of the sensor
values verify reliable I2C communication between the MCU and the
light sensor.
******************************************************************

Users can view the informational and error messages on the console

******************************************************************

Periodic Sensor Processing Timer Setup is successfully completed !!!
*****************ISL76682 Light Measurement Result***************
Light Lux: 0.0
*****************ISL76682 Light Measurement Result***************
Light Lux: 20.8
*****************ISL76682 Light Measurement Result***************
Light Lux: 20.8

```


# 5. MCU and FSP Resource Use

| Item | Description |
|--------|------------|
| I2C Master | r\_iic\_master - (I2C Master driver, poll mode with callback completion flag) |
| UART | r\_sci\_uart - (User interface and console communication) |
| Timer | \[r\_agt - (AGT periodic timer for the sampling interval) |


### Module Summary

| Module | Key Configuration | Purpose |
| --- | --- | --- |
| I2C Master (r\_iic\_master) | Channel \[0\], 7-bit addressing, speed \[100 kHz standard / 400 kHz fast\], callback \[rs_i2c_if_callback\] | I2C communication with the light sensor interface|
| SCI UART (r\_sci\_uart) | Channel \[0\], \[115200\] baud, 8N1 | Serial console for results and errors |
| AGT Timer (r\_agt) |  Mode: Periodic, Period units: Seconds | Periodic sensor sampling |


# 6. System Level Architecture

### Functional Overview

The **rs_isl76682_SensorMeasurementEntry()** function implements a periodic, polling-mode I2C workflow for reading the light sensor. On startup, the application prints a welcome banner with the project details to the console and configures the AGT timer to generate periodic timer interrupts. It then opens the FSP I2C master driver using the configured channel, bus speed, and addressing mode, and sets the sensor’s 7-bit slave address.

At runtime, the application starts the I2C transaction with slave device and polls a completion flag. The I2C master callback sets this flag when the driver reports that the transfer has completed or that an error has occurred. Once a transfer completes successfully, the application converts the raw sensor bytes into a Lux value and prints the result to the serial console. This cycle repeats at the configured interval of 1 seconds, giving a lightweight and deterministic framework for periodic sensor reads.

![](images/RS_I2C_Polling_Periodic.jpg "Periodic Read of I2C Light Sensor Block Diagram")

# 7. Periodic Light Sensor Data Read Sequence of events
### Sequence of Events Overview

The **rs_isl76682_SensorMeasurementEntry()** function demonstrates the complete workflow for periodically reading light-level data from the ISL76682 sensor using the I2C master driver in polling mode. The application configures the AGT timer for periodic interrupts, opens the I2C master driver, and initializes the sensor. It then repeatedly reads the sensor, converts the raw data to lux, and prints the result to the serial console until the application is stopped. Finally, it performs a graceful shutdown by stopping the timer, powering down the sensor, and closing the I2C driver.

### 1. Initialization

At startup, the application sends a message containing the project details to the serial console, then verifies the user-supplied configuration, namely the I2C channel, bus speed, and addressing mode.

### 2. Timer and I2C Driver Initialization

After the configuration is complete, the application opens and starts the AGT timer that controls the measurement interval. It then opens the FSP I2C master driver with the configured channel, bus speed, and addressing mode, and sets the 7-bit slave address of the sensor. Once the I2C driver and timer have started successfully, the application is ready to communicate with the sensor.

### 3. Sensor Configuration

The application writes the sensor's COMMAND register (00h) to select the operating settings: enable bit, measurement mode ( continuous), light type (ambient or IR), ADC resolution, and lux range. For example, writing 0x80 selects ambient light sensing, one-time mode, 16-bit resolution, and Range 1 (1,000 lux). The application polls the transfer-complete flag until the I2C master callback reports that the write has finished, or that an error has occurred.

### 4. Periodic Measurement Loop

The application then enters its main processing loop, which executes continuously while the application is running.

During each timer interval, the following operations are performed:

1. **Wait for Timer Event**
   The application blocks until the periodic AGT timer event is received.

2. **Start Measurement**
   The sensor converts continuously. The application then waits for the integration time to elapse before reading the result.

3. **Read Sensor Data**
   The application starts a write-then-read transfer with a repeated start to read the DATA LSB (01h) and DATA MSB (02h) registers. It then polls the transfer-complete flag in a loop. The I2C master callback sets this flag when the driver reports that the transfer has completed or that an error has occurred.

4. **Convert Raw Data to Lux**
   The two data bytes are combined into a 16-bit value (`DATA = (MSB << 8) | LSB`) and converted to lux using `Lux = DATA × Range / 65536`.

5. **Report Result**
   The calculated light level is printed to the serial console.

This sequence repeats every time the AGT timer expires, enabling periodic sensor reads at the configured interval.

### 5. Application Shutdown (Optional)

When periodic measurement is no longer required, the application initiates an orderly shutdown. Closes the I2C master driver, and releases all allocated resources before the application exits gracefully.


![](images/isl76682_periodic_polling_flow.jpg "Periodic Polling Sequence Diagram")

# 8. Module Layer Code and Demonstration Code

The following table identifies which files are reusable `module-layer` code and which belong only to the reference or demo application layer.

| Layer | Files | Purpose | Reuse Expectation |
|---|---|---|---|
| `module-layer` | `src/rs_i2c_if/rs_i2c_if.c` | Lifecycle state machine; all public function entry points|
| `module-layer` | `src/rs_i2c_if/rs_i2c_if_ra.c` | private functions | Include directly; do not modify |
| `module-layer` | `src/rs_i2c_if/rs_i2c_if.h` | Public interface header all public function prototypes | Include directly; do not modify |
| `module-layer` | `src/rs_i2c_if/sensor_ctrl/sensor_ctrl.h` |Standard module configurations | Include directly; modify as needed to support the lower-level driver|
| `module-layer` | `src/rs_i2c_if/sensor_ctrl/sensor_ctrl.c` | Public/private functions for sensor-specific driver support|Modify to support a specific sensor driver|
| `module-layer` | `src/rs_i2c_if/config/rs_i2c_if_cfg_data.h` |Public interface header for module configurations| Include directly; do not modify |
| `module-layer` | `src/rs_i2c_if/config/rs_i2c_if_cfg_data.c` |Standard configurations for the module|Include directly; modify to adapt to lower-level driver when needed|
| `Preset Config` | `src/rs_i2c_if/config/rs_i2c_if_cfg_data_fpb_ra4e2_fpb_ra6e2.c/.h` | Pre-built HAL instance definitions for FPB-RA4E2/FPB-RA6E2; aliased to FSP-generated instances where appropriate | Reference only, do not edit; for custom boards, use the FSP Stack Configurator instead
| `Preset Config` | `src/rs_i2c_if/config/rs_i2c_if_cfg_data_fpb_ra2e3.c/.h` | Pre-built HAL instance definitions for FPB-RA2E3; aliased to FSP-generated instances where appropriate | Reference only  do not edit; for custom boards, use the FSP Stack Configurator instead
| `Reference App` | `src/demo/rs_light_sensor_demo.c` | Full demonstration; CLI input/output|

Note: The CLI code for printing and receiving user input via the serial terminal is entirely reference application code and is not part of the core module. When adapting this reference code, CLI input handling stays in your application layer.



### 8.1 Reference Application
These files demonstrate the module but are not intended for direct reuse. They contain board-specific and demo-specific scaffolding. Use them as a starting point and replace what you don't need.

| File | Adaptation Guidance |
|---|---|
| `src/rs_i2c_if/sensor_ctrl/sensor_ctrl.c/.h` |sensor-specific control functions; replace or heavily adapt|
| `src/demo/rs_light_sensor_demo.c` | Starting point for your Application; replace or heavily adapt|
| `src/hal_entry.c` | FSP application entry point |

### 8.2. FSP-Generated Files, Do Not Edit Manually

| Directory | Notes |
|---|---|
| `ra/` | FSP-generated drivers, middleware, CMSIS, and board support; regenerate using the FSP tooling when configuration.xml changes. |
| `ra_gen/` | Auto-generated by the FSP Stack Configurator; overwritten when regenerated. |

### 8.3. Adopting a Newer Release

This module is delivered as reference source code. Once you incorporate and modify it, you are responsible for maintaining your modified copy and merging future upstream changes.

**What typically changes between releases:**
- Core source files (`src/rs_i2c_if/rs_i2c_if.c`, `src/rs_i2c_if/rs_i2c_if_ra.c`, `src/rs_i2c_if/sensor_ctrl/sensor_ctrl.c`),  may receive bug fixes, new features, or FSP version compatibility updates.
- Public headers (`src/rs_i2c_if/rs_i2c_if.h`, `src/rs_i2c_if/sensor_ctrl/sensor_ctrl.h`) may add new public functions or extend existing structures. Additions are generally backward-compatible; removals or signature changes are not.
- `src/rs_i2c_if/rs_i2c_if.h` constants may gain new `#ifndef`-guarded defaults. Your existing overrides continue to take precedence without modification.
- Preset config files may be updated for new FSP releases or corrected for hardware errata.

**Recommended merge process:**

1. Diff the updated core files against your local copies.
2. Apply upstream changes selectively, preserving any application-specific modifications.
3. Review the updated `src/rs_i2c_if/rs_i2c_if.h`, `src/rs_i2c_if/sensor_ctrl/sensor_ctrl.h` for interface changes; update call sites in your application accordingly.
4. Rebuild and retest on target hardware.

**Note:** Releases do not need to track every minor FSP release automatically. Confirm the supported FSP version in the release notes before upgrading. If the FSP version changes between your baseline and the update, rerun the FSP Stack Configurator to regenerate `ra_gen` content for the new FSP version.



## 9. Module and Sensor Driver Integration

### 9.1. Overview

These modules form a layered sensor-control architecture:

> **`sensor_ctrl` provides the generic sensor interface, a `specific sensor driver` implements sensor-specific behavior, and `rs_i2c_if` provides I2C Interface communication and shared-bus management.**

The application should normally access the sensor through `sensor_ctrl`. A sensor implementation will perform register transfers through `g_rs_i2c_if_device`.

### 9.2. Module Relationship

```text
Application
    |
    v
sensor_ctrl's single entry function
    |
    v
sensor-specific driver
    |
    v
g_rs_i2c_if_device
    +-- p_ctrl --> g_rs_i2c_if_device_ctrl
    +-- p_cfg  --> g_rs_i2c_if_device_cfg
    |               +-- p_lower_level_cfg --> address and FSP callback
    |               +-- p_extend ---------> shared-bus configuration
    +-- p_api  --> g_rs_i2c_if_on_i2c
                         |
                         v
                    FSP HAL I2C driver
```

### 9.3. Module Responsibilities

#### 9.3.1. `sensor_ctrl`

`sensor_ctrl` is an application-facing sensor abstraction layer. It stores or references the selected sensor driver’s control, configuration, and API objects, allowing application code to initialize, configure, and read the sensor without directly depending on a sensor-specific driver implementation. It also provides a single-entry interface for sensor operations, isolating sensor-specific implementation details from the application. The `sensor_ctrl` dispatches generic sensor operations to the selected sensor-specific driver.

#### `rs_i2c_if_sensor_ctrl_t`

#### Overview

`rs_i2c_if_sensor_ctrl_t` is the SRAM-resident runtime container used by `rs_i2c_if_SensorDataRead()`. It groups the writable I2C, sensor-driver, external-interrupt, read-context, and sensor-data objects required for one sensor instance.

Read-only API tables remain in **Flash** and are referenced by the corresponding instance objects.

```c
typedef struct st_rs_i2c_if_sensor_ctrl
{
    rs_i2c_if_instance_t                   i2c_instance;
    rs_i2c_if_instance_ctrl_t              i2c_ctrl;
    rs_i2c_if_cfg_t                        i2c_cfg;
    i2c_master_cfg_t                       i2c_hal_cfg;
    rs_i2c_if_bus_extended_cfg_t           i2c_bus_cfg;

    sensor_specific_driver_instance_t      sensor_instance;
    sensor_specific_driver_instance_ctrl_t sensor_ctrl;
    sensor_specific_driver_cfg_t           sensor_cfg;

    external_irq_instance_t                irq_instance;
    icu_instance_ctrl_t                    irq_ctrl;
    external_irq_cfg_t                     irq_cfg;

    rs_i2c_if_sensor_read_context_t        sensor_context;
    sensor_specific_driver_data_t          sensor_data;
} rs_i2c_if_sensor_ctrl_t;
```

#### Object Groups

| Group | Members | Purpose |
|---|---|---|
| I2C controller | `i2c_instance`, `i2c_ctrl`, `i2c_cfg`, `i2c_hal_cfg`, `i2c_bus_cfg` | Maintains the I2C interface, runtime control, and configuration |
| Sensor-specific driver | `sensor_instance`, `sensor_ctrl`, `sensor_cfg` | Maintains the sensor-specific driver instance, runtime state, and sensor configuration |
| External IRQ | `irq_instance`, `irq_ctrl`, `irq_cfg` | Provides the external interrupt instance, control block, and configuration |
| Sensor read | `sensor_context`, `sensor_data` | Maintains the generic read context and destination for acquired sensor data |

#### Memory Organization

- **SRAM:** Runtime control blocks, mutable configurations, read context, and sensor data.
- **Flash:** Read-only I2C, sensor, and external-IRQ API tables.

This organization keeps writable runtime state in SRAM without unnecessarily copying constant API tables from Flash.

#### Architecture Role

```text
Application
  -> Sensor Control
    -> Sensor-Specific Driver
      -> rs_i2c_if
        -> I2C HAL
          -> Sensor
```

`rs_i2c_if_sensor_ctrl_t` consolidates the runtime state required by this path into one object, supporting an application-facing, single-entry sensor interface while keeping sensor-specific and lower-level I2C implementation details below the application layer.

#### 9.3.2. `Sensor-specific driver`

The `Sensor-specific driver` implements a sensor-specific operations, including:

- Device initialization and reset;
- Register configuration;
- Interrupt configuration;
- Raw-data conversion; and
- Sensor-specific communication-event handling.

Its configuration should reference `g_rs_i2c_if_device` as the communication interface. It should not directly access the FSP I2C instance, mutex, semaphore, or shared-bus state.

#### 9.3.3. `rs_i2c_if`

`rs_i2c_if` provides the generic transport layer. It owns:

- The FSP I2C driver connection;
- Device addressing;
- Interrupt-driven transfer completion;
- Shared-bus locking;
- Bus timeout handling; and
- Bus reconfiguration when multiple devices share one peripheral.


#### Shared-bus configuration

```c
rs_i2c_if_bus_extended_cfg_t g_rs_i2c_if_bus_extended_cfg =
{
#if RS_I2C_IF_CFG_PRESET_USE
    .p_driver_instance       = (void *) &g_i2c_preset,
#else
    .p_driver_instance       = (void *) &g_i2c,
#endif
    .p_current_ctrl          = NULL,
    .bus_timeout             = RS_I2C_IF_BUS_TIMEOUT_TICKS,
#if BSP_CFG_RTOS == 2
    .p_blocking_semaphore    = &g_rs_i2c_if_bus_blocking_semaphore,
    .p_bus_recursive_mutex   = &g_rs_i2c_if_bus_recursive_mutex,
#endif
    .p_elc                   = NULL,
    .p_timer                 = NULL,
};
```

This structure describes resources shared by all logical devices on the physical I2C bus. Device configurations reference it through `p_extend`.

#### Device runtime control

```c
rs_i2c_if_instance_ctrl_t g_rs_i2c_if_device_ctrl;
```

This object stores mutable runtime state for the sensor communication instance. It is managed by `rs_i2c_if` and is not modified directly by `sensor_ctrl` or `sensor-specific driver`.

#### Lower-level device configuration

```c
const i2c_master_cfg_t g_rs_i2c_if_device_lower_level_cfg =
{
    .slave      = SENSOR_ADDRESS,
    .addr_mode  = I2C_MASTER_ADDR_MODE_7BIT,
    .p_callback = rs_i2c_if_callback,
};
```

This FSP-level configuration defines the sensor I2C address and routes lower-level transfer events to `rs_i2c_if_callback()`.

### Device configuration

```c
const rs_i2c_if_cfg_t g_rs_i2c_if_device_cfg =
{
    .semaphore_timeout = 0xFFFFFFFF,
    .p_lower_level_cfg = (void *) &g_rs_i2c_if_device_lower_level_cfg,
    .p_extend          = (void *) &g_rs_i2c_if_bus_extended_cfg,
    .p_callback        = rm_isl76682_callback,
    .p_context         = NULL,
};
```

This object connects the sensor-specific I2C settings to the shared bus. It also defines the blocking timeout and the callback used by the sensor driver. Use `.p_context = NULL` when no callback context is required.

#### Complete communication instance

```c
const rs_i2c_if_instance_t g_rs_i2c_if_device =
{
    .p_ctrl = &g_rs_i2c_if_device_ctrl,
    .p_cfg  = &g_rs_i2c_if_device_cfg,
    .p_api  = &g_rs_i2c_if_on_i2c,
};
```

This is the transport instance consumed by the `sensor-specific driver`. A sensor-driver configuration can reference it as follows:

```c

.p_instance = &g_rs_i2c_if_device,

```
The `sensor_ctrl` reaches this instance indirectly through the `sensor-specific driver`.

### 9.4. Callback Roles

- `rs_i2c_if_callback()` handles lower-level FSP I2C events, records transfer status, and signals the completion semaphore.
- `sensor_driver_callback() --> rm_isl76682_callback()` receives communication events at the sensor-specific driver layer.
- A `sensor_ctrl` callback, when supported, reports completed sensor operations or sensor events to the application without exposing the I2C callback details.

```text
FSP I2C ISR
     |
     v
rs_i2c_if_callback()
     |
     v
sensor_specific_driver_callback()
     |
     v
sensor_ctrl callback (when supported)
     |
     v
Application
```

### 9.5. Ownership Summary

| Object or layer | Owner and purpose |
|---|---|
| `sensor_ctrl` instance | Generic application-facing sensor interface. |
| `sensor-specific driver` control/configuration | sensor runtime state and sensor-specific settings. |
| `g_rs_i2c_if_bus_recursive_mutex` | `rs_i2c_if`; protects shared-bus access. |
| `g_rs_i2c_if_bus_blocking_semaphore` | `rs_i2c_if`; signals transfer completion. |
| `g_rs_i2c_if_bus_extended_cfg` | `rs_i2c_if`; describes shared physical-bus resources. |
| `g_rs_i2c_if_device_ctrl` | `rs_i2c_if`; stores mutable communication state. |
| `g_rs_i2c_if_device_lower_level_cfg` | FSP boundary; defines device address and callback. |
| `g_rs_i2c_if_device_cfg` | `rs_i2c_if`; connects device and bus configurations. |
| `g_rs_i2c_if_device` | Transport interface exposed to `sensor-specific driver`. |

### 9.6. Runtime Flow

1. The application calls a generic `sensor_ctrl` operation.
2. `sensor_ctrl` dispatches the request to the `sensor-specific driver` implementation.
3. `sensor-specific driver` prepares the required sensor-register transaction.
4. The driver calls the API exposed by `g_rs_i2c_if_device`.
5. `rs_i2c_if` locks the shared bus and applies the device configuration.
6. The FSP I2C driver starts the transfer.
7. `rs_i2c_if_callback()` records completion and signals the semaphore.
8. `rs_i2c_if` returns the transfer result and releases the bus mutex.
9. `sensor-specific driver` interprets the sensor data and returns it through `sensor_ctrl`.
10. The application receives sensor data through the generic interface.

This separation allows another sensor driver to replace `sensor-specific driver` while preserving the application-facing `sensor_ctrl` interface and reusing the same `rs_i2c_if` transport layer.

## 10. Public Functions - Where to Start

All public functions use the naming pattern `rs_i2c_if_OperationName()`  a lowercase `rs_i2c_if_` prefix followed by an UpperCamelCase operation name. 

All functions return `fsp_err_t` and take a pointer to the control block as their first argument.

Build-time configuration macros and enumeration values use the all-uppercase `RS_I2C_IF_` form.

### 10.1. Single Entry Function

### Overview

The `rs_isl76682_Read_light_sensor_data()` function provides a polling-mode method for acquiring light data from a sensor instance. It manages one-time initialization of the I2C communication path, the sensor-specific driver, and the measurement operation. On each call, it then reads the sensor over I2C and converts the sample to a light value, with no interrupt or external IRQ involved.

### Function Prototype

```c
fsp_err_t rs_isl76682_Read_light_sensor_data(rm_isl76682_ctrl_t    *p_api_ctrl,
                                             rm_isl76682_cfg_t     *p_cfg,
                                             ra_isl76682_setting_t *p_user_setting,
                                             float                 *data);
```

### Parameters

| Parameter | Direction | Description |
|---|---|---|
| `p_api_ctrl` | In/Out | Sensor control block used to retain the driver state across calls |
| `p_cfg` | In | Sensor configuration, including the I2C communication settings and the sensor API |
| `p_user_setting` | In | User-defined sensor settings applied during initialization |
| `data` | Out | Pointer to the variable that receives the converted light data |

### Functional Behavior

#### First Call

On the first call, the function performs the initialization sequence required for polling-mode acquisition:

1. Validates the control block, configuration, user settings, and output pointer.
2. Opens the lower-level I2C driver. `FSP_ERR_ALREADY_OPEN` is accepted because the I2C bus may already be shared and initialized.
3. Opens the sensor driver.
4. Applies the user settings and starts sensor measurement.

If initialization fails after resources have been opened, the function uses the module cleanup path before returning the originating error.

#### Subsequent Calls

After initialization, the function reuses the existing sensor state instead of reopening the drivers and goes directly to reading and converting data.

#### Reading and Converting Data

On each call, the function:

1. Reads the raw sensor data over I2C.
2. Converts the raw data to a light value and stores it in `data`.

Errors from either operation are returned directly to the caller.

### Return Values

| Return value | Meaning |
|---|---|
| `FSP_SUCCESS` | Light sensor data was read and converted successfully |
| `FSP_ERR_ASSERTION` | A required pointer or required API function pointer is `NULL` |
| Other `fsp_err_t` values | Errors returned by the lower-level I2C driver, sensor driver, data read, or data conversion functions |

### Usage Considerations

- The object referenced by `p_api_ctrl` is persistent state and must remain valid across calls.
- The lower-level I2C bus may be shared. Therefore, `FSP_ERR_ALREADY_OPEN` from its `open()` API is treated as an acceptable initialization result.
- The function is polling-based. The application calls it periodically at the desired sampling interval, and each call blocks only for the duration of the I2C transaction.
- No external IRQ or data-ready interrupt is required.

### Typical Usage

```c
fsp_err_t err;
float light_lux = 0;

err = rs_isl76682_Read_light_sensor_data(g_isl76682_als0.p_ctrl,
                                         (rm_isl76682_cfg_t *) g_isl76682_als0.p_cfg,
                                         &gs_isl76682_user_setting,
                                         &light_lux);
```

### 10.2. Public Function Summary

| Function | Purpose |
|---|---|
| `rs_i2c_if_ctrl_Read` | Reads one or more bytes from the configured I2C device. |
| `rs_i2c_if_ctrl_Write` | Writes one or more bytes to the configured I2C device. |
| `rs_i2c_if_ctrl_WriteRead` | Performs a combined write followed by a read operation. |
| `rs_i2c_if_ctrl_Open` | Validates the configuration, initializes the instance and RTOS resources when applicable, and verifies the lower-level bus. |
| `rs_i2c_if_ctrl_Close` | Closes the instance, releases its shared-bus selection, and clears runtime state. |

### Common Preconditions

- `p_api_ctrl` must point to a valid `rs_i2c_if_instance_ctrl_t` control block.
- `rs_i2c_if_ctrl_Open()` must complete successfully before calling `Read`, `Write`, `WriteRead`, or `Close`.
- Transfer buffers must remain valid for the duration of the blocking transaction.
- A transfer size of zero is rejected with `FSP_ERR_INVALID_SIZE`.

### Error Propagation

In addition to the explicitly documented validation errors, the transaction functions return the status produced by `rs_i2c_if_bus_read()`, `rs_i2c_if_bus_write()`, or `rs_i2c_if_bus_write_read()`. `rs_i2c_if_ctrl_Open()` also propagates failures from RTOS-object initialization and lower-level bus-status checking.

### 10.3. `rs_i2c_if_ctrl_Read`

**Implements:** `rs_i2c_if_api_t::read`

Reads the requested number of bytes from the configured I2C device into the caller-provided destination buffer.

### Prototype
```c
fsp_err_t rs_i2c_if_ctrl_Read(rs_i2c_if_ctrl_t * const p_api_ctrl,
                              uint8_t * const           p_dest,
                              uint32_t const            bytes);
```
### Parameters

| Parameter | Description |
|---|---|
| `p_api_ctrl` | Pointer to the `rs_i2c_if` control block. |
| `p_dest` | Destination buffer that receives the data. |
| `bytes` | Number of bytes to read. Must be greater than zero. |

### Operation

1. Validates the control pointer, destination pointer, and transfer size.
2. Casts the public control pointer to `rs_i2c_if_instance_ctrl_t`.
3. Verifies that the instance open marker equals `RS_I2C_IF_OPEN`.
4. Calls `rs_i2c_if_bus_read()` and returns its status.

### Return Values

| Return value | Condition |
|---|---|
| `FSP_SUCCESS` | The read completed successfully. |
| `FSP_ERR_ASSERTION` | `p_api_ctrl` or `p_dest` is `NULL`. |
| `FSP_ERR_INVALID_SIZE` | `bytes` is zero. |
| `FSP_ERR_NOT_OPEN` | The module is not open. |
| Other `fsp_err_t` | An error returned by the lower-level bus-read helper. |

### 10.4. `rs_i2c_if_ctrl_Write`

**Implements:** `rs_i2c_if_api_t::write`

Writes the requested number of bytes from the caller-provided source buffer to the configured I2C device.

### Prototype

```c
fsp_err_t rs_i2c_if_ctrl_Write(rs_i2c_if_ctrl_t * const p_api_ctrl,
                               uint8_t * const           p_src,
                               uint32_t const            bytes);
```
### Parameters

| Parameter | Description |
|---|---|
| `p_api_ctrl` | Pointer to the `rs_i2c_if` control block. |
| `p_src` | Source buffer containing the bytes to transmit. |
| `bytes` | Number of bytes to write. Must be greater than zero. |

### Operation

1. Validates the control pointer, source pointer, and transfer size.
2. Casts the public control pointer to `rs_i2c_if_instance_ctrl_t`.
3. Verifies that the module is open.
4. Calls `rs_i2c_if_bus_write()` and returns its status.

### Return Values

| Return value | Condition |
|---|---|
| `FSP_SUCCESS` | The write completed successfully. |
| `FSP_ERR_ASSERTION` | `p_api_ctrl` or `p_src` is `NULL`. |
| `FSP_ERR_INVALID_SIZE` | `bytes` is zero. |
| `FSP_ERR_NOT_OPEN` | The module is not open. |
| Other `fsp_err_t` | An error returned by the lower-level bus-write helper. |

### 10.5. `rs_i2c_if_ctrl_WriteRead`

**Implements:** `rs_i2c_if_api_t::writeRead`

Performs a combined I2C transaction that writes a command or register address and then reads response data.

### Prototype
```c
fsp_err_t rs_i2c_if_ctrl_WriteRead( rs_i2c_if_ctrl_t * const            p_api_ctrl,
                                    rs_i2c_if_write_read_params_t const write_read_params);
```
### Parameters

| Parameter | Description |
|---|---|
| `p_api_ctrl` | Pointer to the `rs_i2c_if` control block. |
| `write_read_params.p_src` | Source buffer for the write phase. |
| `write_read_params.src_bytes` | Number of bytes in the write phase. Must be greater than zero. |
| `write_read_params.p_dest` | Destination buffer for the read phase. |
| `write_read_params.dest_bytes` | Number of bytes in the read phase. Must be greater than zero. |

### Operation

1. Validates the control pointer, both transfer buffers, and both transfer sizes.
2. Casts the public control pointer to `rs_i2c_if_instance_ctrl_t`.
3. Verifies that the module is open.
4. Passes the parameter structure to `rs_i2c_if_bus_write_read()`.
5. Returns the lower-level helper status without translation.

### Return Values

| Return value | Condition |
|---|---|
| `FSP_SUCCESS` | The combined transaction completed successfully. |
| `FSP_ERR_ASSERTION` | `p_api_ctrl`, `p_src`, or `p_dest` is `NULL`. |
| `FSP_ERR_INVALID_SIZE` | `src_bytes` or `dest_bytes` is zero. |
| `FSP_ERR_NOT_OPEN` | The module is not open. |
| Other `fsp_err_t` | An error returned by the lower-level write-read helper. |

### 10.6. `rs_i2c_if_ctrl_Open`

**Implements:** `rs_i2c_if_api_t::open`

Opens and configures an `rs_i2c_if` instance, associates it with the shared I2C bus, initializes optional RTOS resources, and verifies that the lower-level driver is available.

### Prototype

```c
fsp_err_t rs_i2c_if_ctrl_Open(rs_i2c_if_ctrl_t * const      p_api_ctrl,
                              rs_i2c_if_cfg_t const * const p_cfg);
```
### Parameters

| Parameter | Description |
|---|---|
| `p_api_ctrl` | Pointer to the instance control block to initialize. |
| `p_cfg` | Pointer to the immutable instance configuration. `p_extend` and `p_lower_level_cfg` must be valid. |
| `p_cfg->p_extend` | Points to `rs_i2c_if_bus_extended_cfg_t`, which contains the shared-bus configuration. |
| `p_cfg->p_lower_level_cfg` | Points to the lower-level I2C driver configuration. |

### Operation

1. Validates `p_api_ctrl`, `p_cfg`, `p_cfg->p_extend`, and `p_cfg->p_lower_level_cfg`.
2. Casts `p_api_ctrl` to `rs_i2c_if_instance_ctrl_t`.
3. Casts `p_cfg->p_extend` to `rs_i2c_if_bus_extended_cfg_t`.
4. Rejects an instance whose `open` marker already equals `RS_I2C_IF_OPEN`.
5. Validates `p_bus->p_driver_instance`.
6. For RTOS builds, verifies that the blocking semaphore and recursive mutex are either both configured or both absent.
7. Initializes the control block with the configuration, bus, lower-level configuration, callback, and context.
8. Resets the transfer-data fields and SMBus operation flag.
9. When `BSP_CFG_RTOS == 2`, calls `rs_i2c_if_bus_os_init()`.
10. Calls `rs_i2c_if_bus_status_check()` to verify the lower-level bus.
11. Clears `p_bus->p_current_ctrl` if it already points to this control block, forcing address and callback configuration on the first transfer.
12. Sets `p_ctrl->open` to `RS_I2C_IF_OPEN`.



### Return Values

| Return value | Condition |
|---|---|
| `FSP_SUCCESS` | The module was configured and opened successfully. |
| `FSP_ERR_ASSERTION` | A required pointer is `NULL`, the driver instance is missing, or the RTOS synchronization-object combination is invalid. |
| `FSP_ERR_ALREADY_OPEN` | The control block is already open. |
| `FSP_ERR_COMMS_BUS_NOT_OPEN` | The lower-level I2C driver is not open, when reported by the bus-status check. |
| Other `fsp_err_t` | An error returned by RTOS-object initialization or bus-status checking. |


---
### 10.7. `rs_i2c_if_ctrl_Close`

**Implements:** `rs_i2c_if_api_t::close`

Closes an `rs_i2c_if` instance, removes its shared-bus selection when applicable, and clears its runtime control state.

### Prototype

```c
fsp_err_t rs_i2c_if_ctrl_Close(rs_i2c_if_ctrl_t * const p_api_ctrl);
```
### Parameters

| Parameter | Description |
|---|---|
| `p_api_ctrl` | Pointer to the open `rs_i2c_if` control block. |

### Operation

1. Validates `p_api_ctrl`.
2. Casts the public control pointer to `rs_i2c_if_instance_ctrl_t`.
3. Verifies that the instance is open.
4. Verifies that the stored shared-bus pointer is valid.
5. Clears `p_current_ctrl` only if the closing instance currently owns the shared-bus selection.
6. Clears the open marker, configuration pointers, callback, context, transfer state, and SMBus state.
7. Returns `FSP_SUCCESS`.

### Cleared Runtime State

```c
p_ctrl->open                = 0U;
p_ctrl->p_cfg               = NULL;
p_ctrl->p_bus               = NULL;
p_ctrl->p_lower_level_cfg   = NULL;
p_ctrl->p_callback          = NULL;
p_ctrl->p_context           = NULL;
p_ctrl->p_transfer_data     = NULL;
p_ctrl->transfer_data_bytes = 0U;
p_ctrl->smbus_operation     = false;
```
### Return Values

| Return value | Condition |
|---|---|
| `FSP_SUCCESS` | The instance was closed and its control state was cleared. |
| `FSP_ERR_ASSERTION` | `p_api_ctrl` is `NULL` or the stored bus pointer is `NULL`. |
| `FSP_ERR_NOT_OPEN` | The module is not open. |

### Implementation Notes

- The function does not close the shared lower-level I2C driver. This supports a bus shared by multiple logical device instances.



# 11. Adapting to Your Board

### Configuration Path Options

The module supports two ways to supply hardware configuration, selected by RS_I2C_IF_CFG_PRESET_FPB_RA4E2_FPB_RA6E2 in the demonstration project build options.

#### Preset Configuration Path (RS_I2C_IF_CFG_PRESET_FPB_RA4E2_FPB_RA6E2 = 1)

Pre-built HAL instance definitions for the FPB-RA4E2 and FPB-RA6E2 are provided in:

- &#91;src/rs\_i2c\_if\/config/rs\_i2c\_if\_cfg\_data\_fpb\_ra4e2\_fpb_\_ra6e2.c/.h\]
- &#91;src/rs\_i2c\_if\/config/rs\_i2c\_if\_cfg\_data\_fpb\_ra4e2\_fpb_\_ra4e2.c/.h\]
- &#91;src/rs\_i2c\_if\/config/rs\_i2c\_if\_cfg\_data\_fpb\_ra2e3\_fpb_\_ra2e3.c/.h\]

Using these pre-configured HAL instances, the project can be built and executed without manually adding or configuring the I2C master and UART modules in the FSP Stack Configurator.

However, the following FSP components must still be enabled in the FSP Component Configuration:

- r\_iic\_master
- r\_agt 

#### Manual FSP Configuration Path (RS_I2C_IF_CFG_PRESET_FPB_RA4E2_FPB_RA6E2 = 0)

For custom boards, configure the stacks in the FSP Stack Configurator:

| Step | Configuration Menu | Action | Setting |
| --- | --- | --- | --- |
| 1 | Pins | Select the I2C channel pins | SCL and SDA pins for the board \[RA4E2: P100 / P101; RA6E2: P100 / P101] |
| 2 | Stacks | Add I2C Master (r\_iic\_master) | Channel \[0\], 7-bit addressing |
| 3 | I2C Master properties | Set bus speed | Standard (100 kHz) or Fast (400 kHz) | 
| 4 | I2C Master properties | Slave Address	| 0x44 | 
| 5 | I2C Master properties | Set callback | \[rs\_i2c\_callback\] |
| 6 | Stacks | Add Timers (r\_agt\) for the Periodic Timer | Channel \[0\], Mode , Periodic |
| 7 | Timer properties | Name  | \[sensor\_delay\] |
| 8 | Timer properties | Period  | \[1] |
| 9 | Timer properties | Period Units  | seconds|
| 10 | Timer properties | Callback  | \[timer\_callback\]|
| 11 | Stacks | Add UART (r\_sci\_uart) for the console | Channel \[0\], \[115200\] baud, 8N1 |

Generate the project to apply the configuration.

### Recommended Usage Example

```c
FUNCTION read_light_sensor(p_ctrl, p_cfg, p_user_setting, data)

if (!isInitialized)
{
    /* First call: initialize the I2C driver and the sensor */

    /* Open the I2C master driver */
    err = I2C_Open(p_driver_instance);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    /* Open the ISL76682 sensor instance (required before any other sensor API) */
    err = RM_ISL76682_Open(p_ctrl, p_cfg);
    if (FSP_SUCCESS != err)
    {
        I2C_Close(p_driver_instance);        /* Roll back */
        return err;
    }

    /* Configure the COMMAND register (enable, mode, light type, resolution, range) */
    err = RM_ISL76682_SetCmdReg(p_ctrl, p_user_setting);
    if (FSP_SUCCESS != err)
    {
        RM_ISL76682_Close(p_ctrl);           /* Roll back */
        I2C_Close(p_driver_instance);
        return err;
    }

    /* Initialization complete */
    isInitialized = INITIALIZED;
}
else
{
    /* Already initialized: read the light level in lux */
    return RM_ISL76682_ReadLightLux(p_ctrl, p_user_setting, data);
}
END FUNCTION
```



### Integration Notes

- The I2C master driver must be opened before any transfer is started.
- Set the slave address before the first transfer.
- Do not start a new transfer until the completion flag for the previous one is set.
- Clear the completion flag before starting each transfer.
- Use a repeated start (restart set to true) for write-then-read register access.
- Always close the driver before application shutdown.
