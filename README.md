# CUFE-EECE-FOTA-2023

Cairo University.
Faculty of Engineering.
Electronics and Electrical Communcation Engineering Department.

2022-2023

## Firmware Over The Air (FOTA)

by:
- Ahmed Shawky Hamed Abdulrahman
- Ahmed Mohamed Hassanin Ali
- Rana Ali Ibrahim Mohamed
- Roaa Ayman Fahmy Hasan
- Mostafa Hesham Mahmoud Shousha

## Introduction

Firmware-over-the-air (FOTA) services enable firmware downloads and updates for any of the specific electronic control units (ECUs) inside a car remotely. Project objective is to Update ECU’s software remotely and perform Live Diagnostics through a Wi-Fi connection to a server/cloud.

## Overview

The system consists of 2 ECUs, one of them is Application ECU which is: STM32F429ZIT6, and the other one is the Main ECU which is: Raspberry Pi 4 Model B, and a touch screen connected to it through HDMI in an HMI system. HMI system’s GUI is implemented using Python. The two ECUs communicate with each other over CAN Network. Raspberry Pi is connected to the CAN network by using SPI to CAN module (CAN Controller). Flashing process is implemented using custom CAN Bootloader, and the application is implemented by using Real Time Operating System (FreeRTOS).

## Block Diagram

![Block Diagram](https://github.com/AhmedShawkyelmihy/CUFE-EECE-FOTA-2023/blob/main/Block_Diagram.jpeg)

## System Phases

### GUI



### SPI to CAN Module

- Although there is no CAN module in Raspberry PI, a Serial communcation protocol module supported by Raspberry PI must be used, which is Serial Peripheral Interface protocol (__SPI__). 
- SPI to CAN module driver is written in Python.
- SPI to CAN module consists of 2 chips: MCP2515, TJA1050.
- TJA1050 is replaced by SN65HVD23x which is operating at 3.3V and compatible with Raspberry PI GPIO pins operating voltage.

### CAN Driver

- To read or write data to CAN bus, CAN Driver should be implemented to use CAN module (__bxCAN__) in STM32F429, which is implemented from scratch.
- The implemented CAN driver supports both modes: polling and interrupt.

### Bootloader

- The bootloader is a software (piece of code) which runs first after any power up or reset.
- The bootloader receives commands from the host (__Main ECU__) to do a specific functionality on the target (__Application ECU__).
- The bootloader supports different commands mainly:
	- Replace the existing application code with a new application (Flashing).
	- Start the existing application (By jumping to the application startup code).
- The bootloader communicates with the host via communication protocol, the implemented bootloader supports CAN and UART protocols.

### Application

- There are two applications: 
	- Radiator cooler consists of three main components: an LM35 temperature sensor, a DC brushless fan with a 2N2222A transistor as a driver for it.
	- Side mirror controlling consists of two main components: a servo motor and a push button.
- The application code is implemented using APIs supported by FreeRTOS to meet timing constrains (real time system).


### Note

The Source files __only__ are uploaded.
