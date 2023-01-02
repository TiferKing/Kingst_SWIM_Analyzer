# KingstVIS Logic Analyzer SWIM Plugin

[中文](README.md)
[English](README.en.md)

## Introduction

KingstVIS, developed by Qingdao Kingst Electronics Co., Ltd., is a logic analyzer support software that includes many useful analyzers. However, when I attempted to use it to analyze SWIM (Single wire interface module) recently, I found that it did not have this particular analyzer. STMicroelectronics developed SWIM, which is a powerful debugging bus used for debugging STM8 8-bit microcontrollers. I created this analyzer plugin by referring to the official documentation. If you are working with STM8 series MCUs, you may find this plugin helpful during the development or debugging process.

## Table of Contents

- [Introduction](#Introduction)
- [Instructions](#Instructions)
- [Samples](#Samples)
- [Appendix](#Appendix)
- [License](#License)

## Instructions

To add the SWIM analyzer to KingstVIS, first copy `SWIMAnalyzer.dll` to the `Analyzer` directory within the KingstVIS root directory (usually located at `C:\Program Files\KingstVIS\Analyzer`). Then, launch KingstVIS and find the `SWIM` option under `More Analyzers`.

![SWIMList][img1]

## Samples

When you open the SWIM analyzer, a configuration interface with four options will appear: SWIM and RST channel selection, as well as HSI and LSI clock frequency configuration. The SWIM channel is required, while the RST channel is optional. These options correspond to the SWIM data channel and the MCU reset channel, respectively. While the RST channel is optional, selecting it can provide more accurate analysis. The HSI and LSI frequencies should be configured according to the specific MCU you are using. The default frequencies are 16 MHz and 128 KHz, but please refer to the MCU user manual for more information.

![SWIMInterface][img2]

After capturing SWIM data, you can view it in the left waveform panel or the right data analysis panel. The following are three examples of SWIM data captured:

SWIM entry sequence:

![SWIMScreenShot1][img3]

Write on-the-fly:

![SWIMScreenShot2][img4]

Read on-the-fly:

![SWIMScreenShot3][img5]

Some explanations:

- Entry Pulses: SWIM wake-up sequence typically consists of four 1 KHz pulses followed by four 2 KHz pulses.
- HSI Freq: This is a synchronization pulse typically with a width of 128xSWIM clocks.
- ROTF: Read on-the-fly
- WOTF: Write on-the-fly
- SRST: System Reset
- N: The length of the data in bytes
- Addr: Address
- RD: Read data (data that is read from the MCU to the host)
- WD: Write data (data that is written from the host to the MCU)
- Error Frame: Parity error
- Unknown Frame: Reserved frame
- \[A\]: ACK (acknowledgement)
- \[N\]: NACK (negative acknowledgement)

## Appendix

- KingstVIS & Qingdao Kingst Electronics Website: <http://www.qdkingst.com>
- STMicroelectronics Website: <https://www.st.com>
- SWIM Document(UM0470): <https://www.st.com/resource/en/user_manual/cd00173911-stm8-swim-communication-protocol-and-debug-module-stmicroelectronics.pdf>

## License

[MIT](LICENSE) © Tifer King

[img1]: doc/SWIM-1.png "SWIM in Analyzer List"
[img2]: doc/SWIM-2.png "SWIM Configure Interface"
[img3]: doc/SWIM-3.png "SWIM Interface-1"
[img4]: doc/SWIM-4.png "SWIM Interface-2"
[img5]: doc/SWIM-5.png "SWIM Interface-3"