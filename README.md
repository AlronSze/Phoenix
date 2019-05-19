# Phoenix
[![License](https://img.shields.io/badge/license-GPLv3-brightgreen.svg)](LICENSE) [![Build status](https://ci.appveyor.com/api/projects/status/hov7idgbivv9umsq?svg=true)](https://ci.appveyor.com/project/AlronSze/phoenix)

本四轴飞行器设计采用STM32F4开发板作为飞控板，在硬件上主要包括四个模块：9轴传感器模块、蓝牙模块、遥控器及接收器模块、电调控制模块等。传感器与飞控板之间采用IIC协议进行通信，并实现姿态解算和PID控制；蓝牙与飞控板之间采用USART协议进行通信；遥控器及接收器购买成品，通过飞控板对其PWM信号进行捕获来处理；四个电调与飞控板之间采用PWM进行控制，进而控制电机转速。为了能使四轴飞行器系统更具有管理性和控制性，我们移植了μC/OS II来进行任务运行和管理。

<div align="center">
<img src="./Images/overview.png" width="50%">
</div>

## 1. Hardware Requirement

|            器件             |                     型号及参数                     |
| :-------------------------: | :------------------------------------------------: |
|       Quadcopter Rack       |          F450 (Propeller pitch is 450mm)           |
| Electronic Speed Controller |              Hobbywing Skywalker-20A               |
|            Motor            |                 XXD 2212 (1000KV)                  |
|          Propeller          |  1045 (Diameter is 10 inches; Blade angle is 45°)  |
|           Battery           |      ACE 2200mAh 25C lithium-polymer battery       |
|     Main Control Board      |             STM32F401RE (Up to 84MHZ)              |
|  Accelerometer & Gyroscope  |           MPU6050 (Integrated in GY-86)            |
|        Magnetometer         |           HMC5883L (Integrated in GY-86)           |
|      Remote Controller      | RadioLink T4EU 2.4G four-channel remote controller |
|       Remote Receptor       | RadioLink R7EH 2.4G seven-channel remote receptor  |
|          Bluetooth          |           HM-10 (slave), HM-15 (master)            |
|       Expansion Board       |         Design and draw using Protel 99 SE         |

## 2. Software Requirement

Operation System: **Windows** or **Linux**

Recommend IDE: **Keil uVision** or **STM32CubeIDE**

Compiler: **ARM Compiler (using Keil uVision)** or **GNU ARM Embedded Toolchain (using makefile)**

Option: **Openocd** (if you want to burn program without IDE)

## 3. Build

Simply run `make` or use **Keil uVision 5** to compile.

## 4. Burn

Simply run `make burn` or use **Keil uVision 5** to burn.
