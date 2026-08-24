#ifndef SPWM_CORE_H
#define SPWM_CORE_H

#include <stdint.h>

/*
 * =======================================================================================
 * SINE WAVE LOOK-UP TABLE (LUT)
 * =======================================================================================
 * Contains one full electrical cycle (360 degrees) mapped across 256 points.
 * Amplitude is scaled to the maximum signed 16-bit integer range (-32767 to +32767).
 * Placed in Flash ROM using static const to eliminate RAM overhead.
 * =======================================================================================
 */
static const int16_t Sine_Table[256] = {
    0, 804, 1607, 2410, 3211, 4011, 4807, 5601, 6392, 7179, 7961, 8739, 9511, 10278, 11038, 
    11792, 12539, 13278, 14009, 14732, 15446, 16150, 16845, 17530, 18204, 18867, 19519, 
    20159, 20787, 21402, 22004, 22594, 23169, 23731, 24278, 24811, 25329, 25831, 26318, 
    26789, 27244, 27683, 28105, 28510, 28897, 29268, 29621, 29955, 30272, 30571, 30851, 
    31113, 31356, 31580, 31785, 31970, 32137, 32284, 32412, 32520, 32609, 32678, 32727, 
    32757, 32767, 32757, 32727, 32678, 32609, 32520, 32412, 32284, 32137, 31970, 31785, 
    31580, 31356, 31113, 30851, 30571, 30272, 29955, 29621, 29268, 28897, 28510, 28105, 
    27683, 27244, 26789, 26318, 25831, 25329, 24811, 24278, 23731, 23169, 22594, 22004, 
    21402, 20787, 20159, 19519, 18867, 18204, 17530, 16845, 16150, 15446, 14732, 14009, 
    13278, 12539, 11792, 11038, 10278, 9511, 8739, 7961, 7179, 6392, 5601, 4807, 4011, 
    3211, 2410, 1607, 804, 0, -804, -1607, -2410, -3211, -4011, -4807, -5601, -6392, -7179, 
    -7961, -8739, -9511, -10278, -11038, -11792, -12539, -13278, -14009, -14732, -15446, 
    -16150, -16845, -17530, -18204, -18867, -19519, -20159, -20787, -21402, -22004, -22594, 
    -23169, -23731, -24278, -24811, -25329, -25831, -26318, -26789, -27244, -27683, -28105, 
    -28510, -28897, -29268, -29621, -29955, -30272, -30571, -30851, -31113, -31356, -31580, 
    -31785, -31970, -32137, -32284, -32412, -32520, -32609, -32678, -32727, -32757, -32767, 
    -32757, -32727, -32678, -32609, -32520, -32412, -32284, -32137, -31970, -31785, -31580, 
    -31356, -31113, -30851, -30571, -30272, -29955, -29621, -29268, -28897, -28510, -28105, 
    -27683, -27244, -26789, -26318, -25831, -25329, -24811, -24278, -23731, -23169, -22594, 
    -22004, -21402, -20787, -20159, -19519, -18867, -18204, -17530, -16845, -16150, -15446, 
    -14732, -14009, -13278, -12539, -11792, -11038, -10278, -9511, -8739, -7961, -7179, 
    -6392, -5601, -4807, -4011, -3211, -2410, -1607, -804
};

/*
 * =======================================================================================
 * SPWM HANDLE STRUCTURE
 * =======================================================================================
 * Encapsulates all state variables, hardware pointers, and pre-calculated safety limits.
 * Guarantees zero global variable usage and complete interrupt-safe multi-instance support.
 * =======================================================================================
 */
typedef struct {
    // Hardware Abstraction Pointers
    // Direct pointers to Timer CCR registers bypass HAL latency in the high-frequency ISR
    volatile uint32_t *CCR1_Reg; 
    volatile uint32_t *CCR2_Reg; 
    volatile uint32_t *CCR3_Reg; 
    
    // Pointer to the Sine Look-Up Table
    const int16_t *sine_table;   
    
    // 32-bit Phase Accumulators for Direct Digital Synthesis (DDS)
    // Allows micro-hertz output frequency resolution without enlarging LUT memory
    uint32_t phase_acc_u;
    uint32_t phase_acc_v;
    uint32_t phase_acc_w;
    
    // DDS Tuning Word (determines output fundamental frequency)
    uint32_t phase_step;         
    
    // Pre-calculated Q16 fixed-point scaling factor to eliminate ISR divisions
    uint32_t duty_scale_q16;     
    
    // Timer Auto-Reload (ARR) value defining the PWM switching period
    uint32_t timer_arr;          
    
    // Pre-calculated virtual zero baseline (ARR / 2) to eliminate shifts in the ISR
    uint32_t timer_center;       
    
    // Minimum allowable CCR threshold to protect IGBT against narrow-pulse linear region
    uint32_t ccr_min_limit;      
    
    // Maximum allowable CCR threshold to guarantee low-side conduction for Bootstrap recharge
    uint32_t ccr_max_limit;      
    
    // Initialization guard flag
    uint8_t is_initialized;      
} SPWM_Handle_t;

// Function Prototypes
int8_t SPWM_Init(SPWM_Handle_t *hspwm, const int16_t *lut, uint32_t arr_val, 
                 uint32_t igbt_min_ticks, uint32_t dead_time_ticks);
void SPWM_Set_Frequency(SPWM_Handle_t *hspwm, float target_hz, float f_sw);
void SPWM_Set_Amplitude(SPWM_Handle_t *hspwm, uint16_t amplitude_percent);
void SPWM_Update_ISR(SPWM_Handle_t *hspwm);

#endif /* SPWM_CORE_H */