# Pheonix
本四轴飞行器设计采用STM32F4开发板作为飞控板，在硬件上主要包括四个模块：9轴传感器模块、蓝牙模块、遥控器及接收器模块、电调控制模块等。传感器与飞控板之间采用IIC协议进行通信，并实现姿态解算和PID控制；蓝牙与飞控板之间采用USART协议进行通信；遥控器及接收器购买成品，通过飞控板对其PWM信号进行捕获来处理；四个电调与飞控板之间采用PWM进行控制，进而控制电机转速。为了能使四轴飞行器系统更具有管理性和控制性，我们移植了μC/OS II来进行任务运行和管理。

<img src="./Images/overview.png" width = "50%" align = "center" />

## 1. Hardware Requirement

|       器件       |              型号及参数              |
| :--------------: | :----------------------------------: |
|       机架       | F450（浆距为450mm，采用碳纤维材料）  |
|       电调       |            好盈天行者20A             |
|       电机       |         新西达2212（1000KV）         |
|      螺旋桨      |  1045（直径为10英寸，桨叶角为45°）   |
|       电池       |     格氏 2200mAh 25C锂聚合物电池     |
|      主控板      |    STM32F401RE（主频最高为84MHZ）    |
| 加速度计、陀螺仪 |       MPU6050（集成于GY-86中）       |
|      磁力计      |      HMC5883L（集成于GY-86中）       |
|      遥控器      |      乐迪T4EU 2.4G四通道遥控器       |
|      接收器      |      乐迪R7EH 2.4G七通道接收器       |
|       蓝牙       | HM-10（下位机端）、HM-15（上位机端） |
|      扩展板      |    使用Protel 99 SE自行设计并绘制    |

## 2. Software Requirement

Operation System: **Windows** or **Linux**

Compiler: **ARM Compiler (using Keil uVision)** or **GNU ARM Embedded Toolchain (using makefile)**

## 3. Build

Simply run `make` or use Keil uVision 5 to compile.