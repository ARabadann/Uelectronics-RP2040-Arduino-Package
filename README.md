# UNIT Electronics Arduino Package for RP2040 and RP2350

The **UNIT Electronics Arduino RP2040/RP2350 Package** provides Arduino IDE support for UNIT Electronics development boards based on the Raspberry Pi **RP2040** and **RP2350** microcontrollers.

This package is based on the excellent **Arduino-Pico** project developed and maintained by **Earle F. Philhower III** and contributors.

UNIT Electronics extends and adapts the upstream project with board definitions, pin mappings, memory configurations, and hardware-specific support for UNIT Electronics development boards.

Version **3.0.0** introduces support for the **UNIT Pulsar RP2350A**, while maintaining support for the previous UNIT RP2040 boards, selected official Raspberry Pi Pico boards, and generic RP2040/RP2350 targets.

---

## Core Foundation

This package is based on the **Arduino-Pico 6.0.0** core developed and maintained by **Earle F. Philhower III** and contributors.

UNIT Electronics extends the upstream project with board definitions, pin mappings, memory configurations, and hardware-specific support for UNIT Electronics RP2040 and RP2350 boards.

**UNIT Electronics Package:** 3.0.0  
**Arduino-Pico Base:** 6.0.0

For complete information about the underlying Arduino core:

**Arduino-Pico:**  
https://github.com/earlephilhower/arduino-pico

**Arduino-Pico Documentation:**  
https://arduino-pico.readthedocs.io/en/latest/

---

## Key Features

- Native Arduino IDE support for UNIT Electronics RP2040 and RP2350 boards.
- Support for the **UNIT Pulsar RP2350A**.
- Support for the **UNIT DualMCU RP2040** and **UNIT Dual ONE RP2040**.
- Based on the modern **Arduino-Pico 6.0.0** core.
- Support for both **RP2040** and **RP2350** architectures.
- Board-specific Arduino pin mappings.
- Board-specific Flash and memory configurations.
- PSRAM support for compatible RP2350 boards.
- Compatibility with selected official Raspberry Pi Pico boards.
- Generic RP2040 and RP2350 targets for custom hardware.
- Arduino Boards Manager installation and version management.
- Access to the libraries and examples provided by the Arduino-Pico ecosystem.

---

## Supported Boards

### UNIT Electronics

- **UNIT DualMCU RP2040**
- **UNIT Dual ONE RP2040**
- **UNIT Pulsar RP2350A**

### Raspberry Pi

- **Raspberry Pi Pico**
- **Raspberry Pi Pico W**
- **Raspberry Pi Pico 2**
- **Raspberry Pi Pico 2W**

### Generic Targets

- **Generic RP2040**
- **Generic RP2350**

---

## UNIT Pulsar RP2350A

Version **3.0.0** adds native Arduino support for the **UNIT Pulsar RP2350A**.

The Pulsar RP2350A is based on the Raspberry Pi **RP2350A** microcontroller and includes:

- **16 MB external Flash**
- **8 MB APS6404L-3SQR-ZR PSRAM**
- Dedicated on-board LED
- **3 on-board NeoPixels**
- QWIIC I2C interface
- On-board **BMI270 IMU**
- On-board PDM microphone
- microSD interface
- HSTX-capable signals exposed through the FPC connector

The Pulsar board definition provides its corresponding Arduino pin mapping, memory configuration, and integrated hardware definitions directly through the board package.

---

# Installing via Arduino Boards Manager

## 1. Add the Boards Manager URL

Open the Arduino IDE and navigate to:

**File → Preferences**

Add the following URL to the **Additional Boards Manager URLs** field:

```text
https://raw.githubusercontent.com/UNIT-Electronics/Uelectronics-RP2040-Arduino-Package/main/package_Uelectronics_rp2040_index.json
```

If you already have other Boards Manager URLs configured, add this URL as an additional entry.

---

## 2. Install the Package

Open:

**Tools → Board → Boards Manager**

Search for:

```text
UNIT Electronics RP2040/RP2350
```

Select the package and click **Install**.

The Arduino IDE will download and install the required platform files and tools.

---

## 3. Select Your Board

After installation, navigate to:

**Tools → Board → UNIT Electronics RP2040/RP2350**

Select the appropriate development board.

For example:

```text
UNIT DualMCU RP2040
UNIT Dual ONE RP2040
UNIT Pulsar RP2350A
```

---

# Uploading Sketches

UNIT RP2040 and RP2350 boards support programming through the USB bootloader provided by the Raspberry Pi microcontroller architecture.

In normal operation, sketches can be uploaded directly from the Arduino IDE.

If the board is not automatically detected or manual bootloader entry is required, use the corresponding **BOOT** and **RESET** controls on the board.

---

## Uploading to the DualMCU RP2040

Connect the DualMCU to your computer using a USB-C cable.

Make sure the mechanical USB selector is routed to the RP2040 side.

To manually enter bootloader mode:

1. Press and hold the RP2040 **RESET** button.
2. Press and hold the RP2040 **BOOT** button.
3. Release the **RESET** button.
4. Release the **BOOT** button.
5. The RP2040 should enter USB bootloader mode.
6. Upload the sketch from the Arduino IDE.

For detailed hardware information, see the UNIT DualMCU repository:

https://github.com/UNIT-Electronics/DualMCU

---

## Uploading to the Dual ONE RP2040

Connect the **Dual ONE RP2040** through USB-C.

If manual bootloader entry is required:

1. Press and hold the RP2040 **BOOT** button.
2. Press and release **RESET**.
3. Release the **BOOT** button.
4. Upload the sketch from the Arduino IDE.

For additional hardware information:

https://github.com/UNIT-Electronics/DualMCU-ONE

---

## Uploading to the Pulsar RP2350A

Connect the **Pulsar RP2350A** to your computer through USB-C.

If manual bootloader entry is required:

1. Press and hold the **BOOT** button.
2. Press and release the **RESET** button.
3. Release the **BOOT** button.
4. The RP2350A should enter USB bootloader mode.
5. Select **UNIT Pulsar RP2350A** in the Arduino IDE.
6. Upload the sketch.

The Pulsar board definition automatically provides the board-specific configuration for the RP2350A, including its **16 MB Flash**, **8 MB PSRAM**, and Arduino pin mapping.

---

# Examples and Libraries

The package includes the libraries and examples provided by the Arduino-Pico core.

Most examples can be accessed directly from the Arduino IDE:

**File → Examples**

Additional board-specific examples, documentation, and hardware resources may be available in the corresponding UNIT Electronics repositories.

---

# Documentation

For information about the Arduino-Pico core, APIs, supported features, RP2040/RP2350 peripherals, networking, filesystem support, and other core functionality, refer to the official Arduino-Pico documentation:

https://arduino-pico.readthedocs.io/en/latest/

Arduino-Pico source repository:

https://github.com/earlephilhower/arduino-pico

UNIT Electronics repositories:

**DualMCU RP2040:**  
https://github.com/UNIT-Electronics/DualMCU

**Dual ONE RP2040:**  
https://github.com/UNIT-Electronics/DualMCU-ONE

---

# Upstream Arduino-Pico Project

This package is based on the **Arduino-Pico** project created and maintained by **Earle F. Philhower III** and contributors.

Arduino-Pico provides the underlying Arduino core, Raspberry Pi Pico SDK integration, compiler/toolchain support, libraries, USB support, filesystem functionality, networking components, and RP2040/RP2350 architecture support used by this package.

UNIT Electronics does not claim authorship of the Arduino-Pico core.

This repository provides a **board-package adaptation and distribution for UNIT Electronics hardware**, including board definitions, pin mappings, memory configurations, and hardware-specific support, while retaining the original licenses and credits of the upstream project.

Arduino-Pico:

https://github.com/earlephilhower/arduino-pico

---

# Contributing

Contributions, bug reports, board-support improvements, and fixes are welcome.

Please open an **Issue** or **Pull Request** in this repository.

When reporting an issue, please include when possible:

- Board model
- UNIT package version
- Arduino IDE version
- Operating system
- Compilation or upload error
- Minimal example required to reproduce the issue

---

# Licensing and Credits

This package contains and builds upon open-source projects from multiple authors and organizations.

The original licenses, copyright notices, and attribution requirements of the upstream projects are retained.

Major components include:

- **Arduino IDE and ArduinoCore-API** — developed and maintained by the Arduino team.
- **Arduino-Pico** — developed and maintained by Earle F. Philhower III and contributors. Core files are distributed under the LGPL.
- **Raspberry Pi Pico SDK** — developed by Raspberry Pi Ltd. and distributed under the BSD 3-Clause license.
- **Pico Quick Toolchain** — provided by the Arduino-Pico project.
- **LittleFS** — developed by ARM Limited and distributed under the BSD 3-Clause license.
- **UF2** — developed by Microsoft Corporation and distributed under the MIT license.
- **FreeRTOS** — Copyright Amazon.com, Inc. or its affiliates and distributed under the MIT license.
- **lwIP** — originally developed by the Swedish Institute of Computer Science and distributed under a BSD-style license.
- **BearSSL** — developed by Thomas Pornin and distributed under the MIT license.
- **UZLib** — Copyright Joergen Ibsen and distributed under the zlib license.
- Networking and filesystem components derived from the **ESP8266 Arduino Core**.
- Additional open-source components distributed as part of the upstream Arduino-Pico project.

For complete and authoritative licensing information, refer to the license and copyright files included in this repository and in the respective upstream projects.

### Arduino-Pico

https://github.com/earlephilhower/arduino-pico

### Raspberry Pi Pico SDK

https://github.com/raspberrypi/pico-sdk

### Arduino

https://www.arduino.cc/

---

# Acknowledgments

Special recognition goes to **Earle F. Philhower III** and all contributors to the **Arduino-Pico** project for developing and maintaining the RP2040/RP2350 Arduino core on which this package is based.

We also acknowledge the work of the **Raspberry Pi**, **Arduino**, and broader open-source communities whose projects make this package possible.

---

**UNIT Electronics — RP2040 / RP2350 Arduino Board Support Package**

**Package Version 3.0.0 — Based on Arduino-Pico 6.0.0**