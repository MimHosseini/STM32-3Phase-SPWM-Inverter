# STM32 Industrial SPWM Core

![Language](https://img.shields.io/badge/Language-C99%2FC11-blue.svg)
![Platform](https://img.shields.io/badge/Platform-STM32%20ARM%20Cortex--M-orange.svg)
![Architecture](https://img.shields.io/badge/Architecture-Object--Oriented-success.svg)
![License](https://img.shields.io/badge/License-MIT-green.svg)

A high-performance, production-ready library for generating Sinusoidal Pulse Width Modulation (SPWM) on STM32 microcontrollers. Designed specifically for power electronics and industrial motor drives, this core prioritizes hardware safety, zero-latency execution, and strict adherence to embedded coding standards.

## ✨ Key Features

* **Object-Oriented C Architecture:** Fully encapsulated state variables and hardware pointers (`SPWM_Handle_t`). Zero global variables, allowing infinite multi-instance support (e.g., controlling multiple inverters simultaneously).
* **Zero HAL Latency in ISR:** Hardware registers (CCRx) are updated via direct memory pointers, completely bypassing the STM32 HAL overhead within the high-frequency Timer Update Interrupt.
* **MISRA C & Fixed-Point DSP:** All mathematical operations inside the ISR utilize Q16 fixed-point arithmetic. Floating-point units (FPU) and heavy divisions are avoided. Negative integer bit-shifting (which is implementation-defined) is strictly avoided to ensure MISRA C compliance.
* **Hardware-Safe Pulse Dropping:** Built-in hysteresis clamping protects IGBTs from operating in the linear region by dropping pulses narrower than the specified hardware limits.
* **Bootstrap Capacitor Protection:** Intelligently clamps the maximum duty cycle to prevent 100% high-side conduction, guaranteeing periodic low-side conduction to recharge the IPM bootstrap capacitors.
* **Millihertz Frequency Resolution:** Employs a 32-bit Phase Accumulator and Direct Digital Synthesis (DDS) to provide continuous, high-resolution frequency control without requiring massive Look-Up Tables.

## 🧠 Software Architecture

The core relies on a pre-calculated 256-point Sine Look-Up Table (LUT) stored in Flash memory. During the 20kHz Timer Update Interrupt, the algorithm advances three 32-bit phase accumulators (separated by 120 degrees). The Most Significant Bits (MSB) of these accumulators index the LUT. 

To ensure absolute safety in **Center-Aligned Mode**, the library computes the limits by incorporating both the **IGBT minimum pulse width** and the **hardware dead-time**. 

## 🚀 How to Use

### 1. Include and Instantiate
```c
#include "spwm_core.h"

// Create an instance of the SPWM handle
SPWM_Handle_t hspwm_inverter;
```

### 2. Initialization
Configure the hardware parameters before enabling the timer outputs.
```c
// Example for 80MHz Clock, 20kHz Switching Frequency
uint32_t timer_arr = 1999;
uint32_t igbt_min_ticks = 160;  // 2.0us minimum pulse
uint32_t dead_time_ticks = 120; // 1.5us dead-time

// Link hardware registers to bypass HAL in ISR
hspwm_inverter.CCR1_Reg = &(TIM1->CCR1);
hspwm_inverter.CCR2_Reg = &(TIM1->CCR2);
hspwm_inverter.CCR3_Reg = &(TIM1->CCR3);

// Initialize the library
if (SPWM_Init(&hspwm_inverter, Sine_Table, timer_arr, igbt_min_ticks, dead_time_ticks) != 0) {
    // Handle initialization error (e.g., Null pointers)
    Error_Handler();
}
```

### 3. Dynamic Control (Main Loop)
Update frequency and amplitude safely outside the ISR.
```c
// Set fundamental frequency to 50.0 Hz (f_sw = 20000 Hz)
SPWM_Set_Frequency(&hspwm_inverter, 50.0f, 20000.0f);

// Set output amplitude to 80%
SPWM_Set_Amplitude(&hspwm_inverter, 80);
```

### 4. Zero-Latency Execution (Interrupt Routine)
Place the update function directly inside the Timer Update IRQ.
```c
void TIM1_UP_TIM16_IRQHandler(void)
{
    /* USER CODE BEGIN TIM1_UP_TIM16_IRQn 0 */
    SPWM_Update_ISR(&hspwm_inverter);
    /* USER CODE END TIM1_UP_TIM16_IRQn 0 */
    
    HAL_TIM_IRQHandler(&htim1);
}
```

## 👨‍💻 About the Developer

**Mohammad Hosseini**
Senior Embedded Systems Engineer & Instructor specializing in STM32 microcontrollers, Edge AI, and Power Electronics.

* 🌐 **Website:** [esfdrive.ir](https://esfdrive.ir)
* 💻 **GitHub:** [@MimHosseini](https://github.com/MimHosseini)
* ✉️ **Email:** about.mim.hosseini@gmail.com

---
*Built with passion for clean code and robust hardware.*
