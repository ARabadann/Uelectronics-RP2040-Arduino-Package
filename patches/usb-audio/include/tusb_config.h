/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Modified 2026 by Adrián Rabadán Ortiz (UNIT Electronics): enables the
 * USB Audio class (CFG_TUD_AUDIO) and sizes its endpoint for 48 kHz.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
 extern "C" {
#endif

//--------------------------------------------------------------------
// COMMON CONFIGURATION
//--------------------------------------------------------------------

#ifndef CFG_TUSB_MCU
 #define CFG_TUSB_MCU             OPT_MCU_RP2040
#endif

#define CFG_TUSB_RHPORT0_MODE     OPT_MODE_DEVICE
#define CFG_TUSB_OS               OPT_OS_PICO

// CFG_TUSB_DEBUG is defined by compiler in DEBUG build
#ifndef CFG_TUSB_DEBUG
#define CFG_TUSB_DEBUG           0
#endif

/* USB DMA on some MCUs can only access a specific SRAM region with restriction on alignment.
 * Tinyusb use follows macros to declare transferring memory so that they can be put
 * into those specific section.
 * e.g
 * - CFG_TUSB_MEM SECTION : __attribute__ (( section(".usb_ram") ))
 * - CFG_TUSB_MEM_ALIGN   : __attribute__ ((aligned(4)))
 */
#ifndef CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_SECTION
#endif

#ifndef CFG_TUSB_MEM_ALIGN
#define CFG_TUSB_MEM_ALIGN          __attribute__ ((aligned(4)))
#endif

//--------------------------------------------------------------------
// DEVICE CONFIGURATION
//--------------------------------------------------------------------

#ifndef CFG_TUD_ENDPOINT0_SIZE
#define CFG_TUD_ENDPOINT0_SIZE    64
#endif

//------------- CLASS -------------//
#define CFG_TUD_HID              (2)
#define CFG_TUD_CDC              (1)
#define CFG_TUD_MSC              (1)
#define CFG_TUD_MIDI             (1)
#define CFG_TUD_VENDOR           (0)
#define CFG_TUD_NCM              (1)

// USB Audio Class
#define CFG_TUD_AUDIO            (1)

#define CFG_TUD_CDC_RX_BUFSIZE  (256)
#define CFG_TUD_CDC_TX_BUFSIZE  (256)

#define CFG_TUD_MSC_EP_BUFSIZE  (64) // Max for RP2040, not OPT_MODE_HS

// HID buffer size Should be sufficient to hold ID (if any) + Data
#define CFG_TUD_HID_EP_BUFSIZE  (64)

// MIDI
#define CFG_TUD_MIDI_RX_BUFSIZE (64)
#define CFG_TUD_MIDI_TX_BUFSIZE (64)

//--------------------------------------------------------------------
// AUDIO CLASS DRIVER CONFIGURATION
//--------------------------------------------------------------------
//
// UNIT Pulsar RP2350A USB Microphone
//
// Format:
//   24 kHz or 48 kHz, selected by the host (UAC2 clock source)
//   16-bit PCM
//   Mono
//   Isochronous IN endpoint
//
// The supported rates live in cores/rp2040/USBAudio.cpp. Only the
// highest one matters here: it sizes the endpoint and the FIFO,
// which are compiled into lib/<cpu>/libpico.a.
//

#define CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE          (48000)

#define CFG_TUD_AUDIO_ENABLE_EP_IN                    (1)

#define CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX    (2)
#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX            (1)


// Complete Audio Function descriptor length
#define CFG_TUD_AUDIO_FUNC_1_DESC_LEN \
    TUD_AUDIO_MIC_ONE_CH_DESC_LEN


// Number of Audio Streaming interfaces
#define CFG_TUD_AUDIO_FUNC_1_N_AS_INT                 (1)


// Audio Class control request buffer
#define CFG_TUD_AUDIO_FUNC_1_CTRL_BUF_SZ              (64)


// 48 kHz x 2 bytes x 1 channel (+1 sample of margin) = 98 bytes
//
// TinyUSB version included in this core uses:
//
// TUD_AUDIO_EP_SIZE(
//     maxFrequency,
//     bytesPerSample,
//     channels
// )

#define CFG_TUD_AUDIO_EP_SZ_IN \
    TUD_AUDIO_EP_SIZE( \
        CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE, \
        CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX, \
        CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX \
    )


// Maximum endpoint packet size
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ_MAX \
    CFG_TUD_AUDIO_EP_SZ_IN


// Software FIFO
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SW_BUF_SZ \
    (4 * CFG_TUD_AUDIO_EP_SZ_IN)

//--------------------------------------------------------------------
// NCM CLASS CONFIGURATION, SEE "ncm.h" FOR PERFORMANCE TUNING
//--------------------------------------------------------------------
#include "lwipopts.h"
// Must be >> MTU
// Can be set to 2048 without impact
#define CFG_TUD_NCM_IN_NTB_MAX_SIZE (2 * TCP_MSS + 100)

// Must be >> MTU
// Can be set to smaller values if wNtbOutMaxDatagrams==1
#define CFG_TUD_NCM_OUT_NTB_MAX_SIZE (2 * TCP_MSS + 100)

// Number of NCM transfer blocks for reception side
#ifndef CFG_TUD_NCM_OUT_NTB_N
#define CFG_TUD_NCM_OUT_NTB_N 1
#endif

// Number of NCM transfer blocks for transmission side
#ifndef CFG_TUD_NCM_IN_NTB_N
#define CFG_TUD_NCM_IN_NTB_N 1
#endif

#ifdef __cplusplus
 }
#endif

#endif /* _TUSB_CONFIG_H_ */
