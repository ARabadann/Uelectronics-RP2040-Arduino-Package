#include "USBAudio.h"

#if !defined(USE_TINYUSB) && !defined(NO_USB)

#include "USB.h"
#include "CoreMutex.h"
#include <tusb.h>
#include <hardware/structs/usb_dpram.h>
#include <cstring>

// ============================================================
// UNIT Electronics USB Audio
// ============================================================

USBAudioClass USBAudio;

// ============================================================
// UAC2 Entity IDs
// ============================================================
//
// These IDs come from TinyUSB's
// TUD_AUDIO_MIC_ONE_CH_DESCRIPTOR.
//
// 0x01 = Input Terminal
// 0x02 = Feature Unit
// 0x03 = Output Terminal
// 0x04 = Clock Source
//

#define UAC2_ENTITY_INPUT_TERMINAL   0x01
#define UAC2_ENTITY_FEATURE_UNIT     0x02
#define UAC2_ENTITY_OUTPUT_TERMINAL  0x03
#define UAC2_ENTITY_CLOCK_SOURCE     0x04


// ============================================================
// USB Audio Runtime State
// ============================================================

// Single source of truth: tusb_config.h
static uint32_t usbAudioSampleRate = CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE;
static bool usbAudioMute = false;
static int16_t usbAudioVolume = 0;


// ============================================================
// Constructor
// ============================================================

USBAudioClass::USBAudioClass()
    : _interfaceID(0),
      _endpointIn(0),
      _stringID(0) {

    // --------------------------------------------------------
    // The constructor runs during static initialization,
    // before setup(), so the interface is registered before
    // the Arduino-Pico USB core builds its descriptors.
    // --------------------------------------------------------


    // --------------------------------------------------------
    // Allocate Audio IN endpoint
    // --------------------------------------------------------

    _endpointIn =
        USB.registerEndpointIn();


    // --------------------------------------------------------
    // Register Audio interface string
    // --------------------------------------------------------

    _stringID =
        USB.registerString(
            "UNIT Pulsar RP2350A Microphone"
        );


    // --------------------------------------------------------
    // Register USB Audio Function
    // --------------------------------------------------------
    //
    // UAC2 microphone:
    //
    // Interface N
    //     Audio Control
    //
    // Interface N + 1
    //     Audio Streaming
    //
    // Therefore this function consumes 2 interfaces.
    //

    _interfaceID =
        USB.registerInterface(
            2,
            descriptorCallback,
            this,
            TUD_AUDIO_MIC_ONE_CH_DESC_LEN,
            20,
            0
        );
}


// ============================================================
// Descriptor Builder
// ============================================================

void USBAudioClass::descriptorCallback(
    int interfaceNumber,
    uint8_t *dst,
    int len,
    void *param
) {

    USBAudioClass *audio =
        static_cast<USBAudioClass *>(param);


    // --------------------------------------------------------
    // Build TinyUSB UAC2 microphone descriptor
    // --------------------------------------------------------

    uint8_t descriptor[] = {

        TUD_AUDIO_MIC_ONE_CH_DESCRIPTOR(

            // Audio Control interface
            static_cast<uint8_t>(
                interfaceNumber
            ),

            // Interface string
            audio->_stringID,

            // Bytes per sample
            CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX,

            // Valid bits per sample
            CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX * 8,

            // Isochronous Audio IN endpoint
            audio->_endpointIn,

            // Maximum Audio IN packet size
            CFG_TUD_AUDIO_EP_SZ_IN
        )
    };


    // --------------------------------------------------------
    // Safety check
    // --------------------------------------------------------

    if (
        dst == nullptr ||
        len < static_cast<int>(sizeof(descriptor))
    ) {
        return;
    }


    // --------------------------------------------------------
    // Copy descriptor into Arduino-Pico configuration
    // descriptor buffer
    // --------------------------------------------------------

    memcpy(
        dst,
        descriptor,
        sizeof(descriptor)
    );
}


// ============================================================
// Begin
// ============================================================

void USBAudioClass::begin() {

    // Nothing is required here.
    //
    // USB Audio registration happens in the constructor,
    // during static initialization.
    //
    // Arduino-Pico starts the USB device itself.
}


// ============================================================
// USB State
// ============================================================

bool USBAudioClass::mounted() {

    return tud_mounted();
}


bool USBAudioClass::ready() {
    return tud_mounted();
}


// ============================================================
// Write PCM Samples
// ============================================================

size_t USBAudioClass::write(
    const int16_t *samples,
    size_t sampleCount
) {

    if (
        samples == nullptr ||
        sampleCount == 0
    ) {
        return 0;
    }

    // --------------------------------------------------------
    // Hold the USB mutex, like SerialUSB does.
    //
    // Arduino-Pico runs tud_task() from an IRQ. Without this
    // lock, a SET_INTERFACE handled in that IRQ can call
    // tu_fifo_clear() while we hold the FIFO write mutex
    // here, and the IRQ blocks forever (whole chip freezes).
    // --------------------------------------------------------

    CoreMutex m(&USB.mutex, false);

    if (!m) {
        return 0;
    }

    if (!tud_mounted()) {
        return 0;
    }

    const uint16_t bytes =
        static_cast<uint16_t>(
            sampleCount *
            sizeof(int16_t)
        );

    const uint16_t written =
        tud_audio_write(
            reinterpret_cast<const uint8_t *>(samples),
            bytes
        );

    // While we hold USB.mutex the 1 ms USB IRQ skips tud_task().
    // write() is usually called on the same 1 ms cadence, so the
    // IRQ can keep landing here and the ISO IN endpoint is never
    // re-armed (audio only got through when Serial ran tud_task).
    // Service the stack ourselves, as SerialUSB does.
    tud_task();

    return written /
           sizeof(int16_t);
}

// ============================================================
// TinyUSB UAC2 GET Entity Request
// ============================================================

extern "C"
bool tud_audio_get_req_entity_cb(
    uint8_t rhport,
    tusb_control_request_t const *request
) {

    const uint8_t entityID =
        static_cast<uint8_t>(
            request->wIndex >> 8
        );

    const uint8_t controlSelector =
        static_cast<uint8_t>(
            request->wValue >> 8
        );

    // ========================================================
    // CLOCK SOURCE
    // ========================================================

    if (
        entityID ==
        UAC2_ENTITY_CLOCK_SOURCE
    ) {

        // ----------------------------------------------------
        // Sampling Frequency Control
        // ----------------------------------------------------

        if (
            controlSelector ==
            AUDIO_CS_CTRL_SAM_FREQ
        ) {

            // ------------------------------------------------
            // GET CUR
            // ------------------------------------------------

            if (
                request->bRequest ==
                AUDIO_CS_REQ_CUR
            ) {

                audio_control_cur_4_t cur = {
                    .bCur = static_cast<int32_t>(usbAudioSampleRate)
                };

                return
                    tud_audio_buffer_and_schedule_control_xfer(
                        rhport,
                        request,
                        &cur,
                        sizeof(cur)
                    );
            }


            // ------------------------------------------------
            // GET RANGE
            // ------------------------------------------------

            if (
                request->bRequest ==
                AUDIO_CS_REQ_RANGE
            ) {

                audio_control_range_4_n_t(1) range = {
                    .wNumSubRanges = 1,
                    .subrange = {
                        {
                            .bMin = CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE,
                            .bMax = CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE,
                            .bRes = 0
                        }
                    }
                };

                return
                    tud_audio_buffer_and_schedule_control_xfer(
                        rhport,
                        request,
                        &range,
                        sizeof(range)
                    );
            }
        }


        // ----------------------------------------------------
        // Clock Valid Control
        // ----------------------------------------------------

        if (
            controlSelector ==
            AUDIO_CS_CTRL_CLK_VALID
        ) {

            if (
                request->bRequest ==
                AUDIO_CS_REQ_CUR
            ) {

                audio_control_cur_1_t cur = {
                    .bCur = 1
                };

                return
                    tud_audio_buffer_and_schedule_control_xfer(
                        rhport,
                        request,
                        &cur,
                        sizeof(cur)
                    );
            }
        }


        return false;
    }

    // ========================================================
    // INPUT TERMINAL
    // ========================================================

    if (
        entityID ==
        UAC2_ENTITY_INPUT_TERMINAL
    ) {

        // ----------------------------------------------------
        // Terminal Connector
        // ----------------------------------------------------

        if (
            controlSelector ==
            AUDIO_TE_CTRL_CONNECTOR
        ) {

            audio_desc_channel_cluster_t ret;

            ret.bNrChannels = 1;
            ret.bmChannelConfig =
                static_cast<audio_channel_config_t>(0);
            ret.iChannelNames = 0;

            return
                tud_audio_buffer_and_schedule_control_xfer(
                    rhport,
                    request,
                    &ret,
                    sizeof(ret)
                );
        }

        return false;
    }

    // ========================================================
    // FEATURE UNIT
    // ========================================================

    if (
        entityID ==
        UAC2_ENTITY_FEATURE_UNIT
    ) {

        // ----------------------------------------------------
        // Mute
        // ----------------------------------------------------

        if (
            controlSelector ==
            AUDIO_FU_CTRL_MUTE
        ) {

            if (
                request->bRequest ==
                AUDIO_CS_REQ_CUR
            ) {

                audio_control_cur_1_t cur = {
                    .bCur = static_cast<int8_t>(usbAudioMute ? 1 : 0)
                };

                return
                    tud_audio_buffer_and_schedule_control_xfer(
                        rhport,
                        request,
                        &cur,
                        sizeof(cur)
                    );
            }
        }


        // ----------------------------------------------------
        // Volume
        // ----------------------------------------------------

        if (
            controlSelector ==
            AUDIO_FU_CTRL_VOLUME
        ) {

            // GET CUR
            if (
                request->bRequest ==
                AUDIO_CS_REQ_CUR
            ) {

                audio_control_cur_2_t cur = {
                    .bCur = usbAudioVolume
                };

                return
                    tud_audio_buffer_and_schedule_control_xfer(
                        rhport,
                        request,
                        &cur,
                        sizeof(cur)
                    );
            }


            // GET RANGE
            if (
                request->bRequest ==
                AUDIO_CS_REQ_RANGE
            ) {

                audio_control_range_2_n_t(1) range = {
                    .wNumSubRanges = 1,
                    .subrange = {
                        {
                            // -90 dB
                            .bMin = -90 * 256,

                            // 0 dB
                            .bMax = 0,

                            // 1 dB steps
                            .bRes = 1 * 256
                        }
                    }
                };

                return
                    tud_audio_buffer_and_schedule_control_xfer(
                        rhport,
                        request,
                        &range,
                        sizeof(range)
                    );
            }
        }


        return false;
    }


    return false;
}


// ============================================================
// TinyUSB UAC2 SET Entity Request
// ============================================================

extern "C"
bool tud_audio_set_req_entity_cb(
    uint8_t rhport,
    tusb_control_request_t const *request,
    uint8_t *buffer
) {

    (void)rhport;

    const uint8_t entityID =
        static_cast<uint8_t>(
            request->wIndex >> 8
        );

    const uint8_t controlSelector =
        static_cast<uint8_t>(
            request->wValue >> 8
        );

    // ========================================================
    // CLOCK SOURCE
    // ========================================================

    if (
        entityID ==
        UAC2_ENTITY_CLOCK_SOURCE
    ) {

        if (
            controlSelector ==
            AUDIO_CS_CTRL_SAM_FREQ
        ) {

            if (
                request->bRequest ==
                AUDIO_CS_REQ_CUR
            ) {

                const audio_control_cur_4_t *cur =
                    reinterpret_cast<
                        audio_control_cur_4_t *
                    >(buffer);

                // Only the configured rate is supported.
                if (
                    cur->bCur ==
                    CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE
                ) {
                    usbAudioSampleRate =
                        cur->bCur;

                    return true;
                }

                return false;
            }
        }

        return false;
    }


    // ========================================================
    // FEATURE UNIT
    // ========================================================

    if (
        entityID ==
        UAC2_ENTITY_FEATURE_UNIT
    ) {

        // ----------------------------------------------------
        // Mute
        // ----------------------------------------------------

        if (
            controlSelector ==
            AUDIO_FU_CTRL_MUTE
        ) {

            if (
                request->bRequest ==
                AUDIO_CS_REQ_CUR
            ) {

                const audio_control_cur_1_t *cur =
                    reinterpret_cast<
                        audio_control_cur_1_t *
                    >(buffer);

                usbAudioMute =
                    cur->bCur != 0;

                return true;
            }
        }


        // ----------------------------------------------------
        // Volume
        // ----------------------------------------------------

        if (
            controlSelector ==
            AUDIO_FU_CTRL_VOLUME
        ) {

            if (
                request->bRequest ==
                AUDIO_CS_REQ_CUR
            ) {

                const audio_control_cur_2_t *cur =
                    reinterpret_cast<
                        audio_control_cur_2_t *
                    >(buffer);

                usbAudioVolume =
                    cur->bCur;

                return true;
            }
        }


        return false;
    }


    return false;
}


// ============================================================
// TinyUSB: Audio Streaming endpoint closed (alt -> 0)
// ============================================================
//
// With TUP_DCD_EDPT_ISO_ALLOC the RP2040/RP2350 DCD never
// closes the ISO endpoint on alt 0. The IN buffer armed while
// alt was 1 stays AVAILABLE in hardware because the host stops
// polling. On the next alt 1, TinyUSB arms it again and the DCD
// calls panic("ep 81 was already available"), halting the chip.
//
// Windows does alt 1 -> alt 0 -> alt 1 as soon as the sound
// settings page is opened, which is exactly when it froze.
//
// Disarm the hardware buffer here so the next transfer starts
// from a clean state.
//

extern "C"
bool tud_audio_set_itf_close_EP_cb(
    uint8_t rhport,
    tusb_control_request_t const *request
) {
    (void)rhport;
    (void)request;

    const uint8_t epNum =
        USBAudio.endpointIn() & 0x0F;

    if (epNum != 0) {
        usb_dpram->ep_buf_ctrl[epNum].in = 0;
    }

    return true;
}

#endif
