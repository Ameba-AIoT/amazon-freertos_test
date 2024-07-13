# FreeRTOS AWS Reference Integrations
## How to use
This is the library sdk which provides Amazon freertos examples. To make sure example working correctly, please download the working chip sdk from related page with this submodule updated.

|Chip         |          Link       |     Supported branch link      |
|:----------- |:---------------------:| :---------------------:|
|Ameba Z2     | https://github.com/Ameba-AIoT/ambz2_sdk| https://github.com/Ameba-AIoT/amazon-freertos/tree/amebaZ2-7.1d-202107.00-LTS |
|The rest chip is under developing   |comming soon | comming soon|


## Cloning
This repo uses [Git Submodules](https://git-scm.com/book/en/v2/Git-Tools-Submodules) to bring in dependent components.

Note: If you download the ZIP file provided by GitHub UI, you will not get the contents of the submodules. (The ZIP file is also not a valid git repository)

To clone using HTTPS:
```
git clone https://github.com/ambiot/amazon-freertos.git --recurse-submodules
```
Using SSH:
```
git clone git@github.com:ambiot/amazon-freertos.git --recurse-submodules
```

If you have downloaded the repo without using the `--recurse-submodules` argument, you need to run:
```
git submodule update --init --recursive
```

## Getting Started

For more information on FreeRTOS, refer to the [Getting Started section of FreeRTOS webpage](https://aws.amazon.com/freertos).

To directly access the **Getting Started Guide** for supported hardware platforms, click the corresponding link in the Supported Hardware section below.

For detailed documentation on FreeRTOS, refer to the [FreeRTOS User Guide](https://aws.amazon.com/documentation/freertos).

## Supported Hardware

For additional boards that are supported for FreeRTOS, please visit the [AWS Device Catalog](https://devices.amazonaws.com/search?kw=freertos)

The following MCU boards are supported for FreeRTOS:
1. **Realtek** - [AamebaD](https://www.amebaiot.com/en/amebad).
    * [Getting Started Guide](https://github.com/ambiot/amazon-freertos/blob/master/AmebaD_Amazon_FreeRTOS_Getting_Started_Guide_v1.6.pdf)
    * IDEs: [IAR Embedded Workbench](https://www.iar.com/iar-embedded-workbench/partners/texas-instruments)
2. **Realtek** - [AmebaZ2](https://www.amebaiot.com/en/amebaz2).
    * [Getting Started Guide](https://github.com/ambiot/amazon-freertos/blob/master/AmebaZ2_Amazon_FreeRTOS_Getting_Started_Guide_v1.0.pdf)
    * IDEs: [IAR Embedded Workbench](https://www.iar.com/iar-embedded-workbench/partners/texas-instruments)
3. **Windows Simulator** - To evaluate FreeRTOS without using MCU-based hardware, you can use the Windows Simulator.
    * Requirements: Microsoft Windows 7 or newer, with at least a dual core and a hard-wired Ethernet connection
    * [Getting Started Guide](https://docs.aws.amazon.com/freertos/latest/userguide/getting_started_windows.html)
    * IDE: [Visual Studio Community Edition](https://www.visualstudio.com/downloads/)


## amazon-freeRTOS/projects
The ```./projects``` folder contains the IDE test and demo projects for each vendor and their boards. The majority of boards can be built with both IDE and cmake (there are some exceptions!). Please refer to the Getting Started Guides above for board specific instructions.

## Mbed TLS License
This repository uses Mbed TLS under Apache 2.0

# Support list

## AmebaD
Work with 

sdk-amebad_v6.2C-RC.tar.gz + 

6.2_patch_integrated_231211_25b3bc38.zip +

6.2c_patch_Support_Amazon_v202210_LTS_w231211_240124_(v01).zip


## AmebaDplus
Work with

sdk-RTL8722F_v11.1b_beta +

11.1b_bata_patch_amazon_v202210-LTS_20240118_(v01).zip


## AmebaSmart
Work with

sdk-RTL8730E_v8.5b_beta + 

8.5b_bata_patch_amazon_v202210-LTS_20240219_(v01).zip

# History link

This repository is carry from and continue develop from old repository link

https://github.com/ambiot/amazon-freertos
