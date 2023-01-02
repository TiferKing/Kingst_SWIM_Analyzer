# 金思特(Kingst)逻辑分析仪SWIM协议分析插件

[中文](README.md)
[English](README.en.md)

## 简介

KingstVIS是青岛金思特电子有限公司推出的多款逻辑分析仪配套的数据采集软件，该软件包含很多常用的协议分析模组。然而在近期使用的时候发现其并不包含意法半导体(STMicroelectronics)公司的SWIM(Single wire interface module)调试总线的时序分析，于是便参考官方提供的协议文档自己开发了这个协议分析的模块。SWIM总线是用于对STM8系列的8-bit单片机进行调试的调试总线，在调试和深度开发STM8系列的单片机时会用到，希望该模块可以帮助有相关需要的人。

## 内容列表

- [简介](#简介)
- [使用方法](#使用方法)
- [数据解析和样例](#数据解析和样例)
- [附录](#附录)
- [使用许可](#使用许可)

## 使用方法

将编译好的SWIMAnalyzer.dll复制到KingstVIS程序根目录下的Analyzer目录中即可，通常这个目录的地址为:`C:\Program Files\KingstVIS\Analyzer`。这时打开KingstVIS软件便可以在协议解析器列表中找到`SWIM`模块。

![SWIMList][img1]

## 数据解析和样例

打开SWIM解析器模块之后，会弹出一个配置页面其中包含四个配置选项，分别是选择SWIM和RST通道和HSI,LSI时钟频率的配置。其中SWIM是必选通道，对应的是SWIM总线的数据通道，RST则是单片机的复位线，复位通道不是必选的，但当选择了复位线之后可以辅助对协议进行更精确的分析。而HSI和LSI始终频率则需要根据使用的单片机型号做出相应的调整，这里默认是16MHz和128KHz，具体需要参阅所使用的单片机型号手册。

![SWIMInterface][img2]

在捕捉到SWIM数据之后，可以在左侧数据面板或者右侧解析结果面板进行对解析到的数据进行观察。以下展示实际捕捉到的三组SWIM数据。

初始化部分：

![SWIMScreenShot1][img3]

写数据包：

![SWIMScreenShot2][img4]

读数据包：

![SWIMScreenShot3][img5]

相关数据类型解释：

- Entry Pulses: SWIM总线协议中唤醒从机的特殊序列，通常为4个1KHz脉冲和4个2KHz脉冲。
- HSI Freq: SWIM总线协议中从机向主机发送的SWIM时钟训练脉宽，通常是128个SWIM时钟。
- ROTF: (Read on-the-fly) 读取指令
- WOTF: (Write on-the-fly) 写入指令
- SRST: (System Reset) 复位指令
- N: 数据包长度
- Addr: 操作起始地址
- RD: 读数据包，通常为从机发往主机
- WD: 写数据包，通常为主机发往从机
- Error Frame: 错误的数据包，通常是奇偶校验结果不符
- Unknown Frame: 未知的数据头，通常是检测到了未知的数据类型
- \[A\]: (ACK) 表明该帧已被确认
- \[N\]: (NACK) 表明该帧未被确认

## 附录

- KingstVIS下载地址&青岛金思特公司官网: <http://www.qdkingst.com>
- 意法半导体公司官网: <https://www.st.com>
- SWIM时序介绍文档(UM0470): <https://www.st.com/resource/en/user_manual/cd00173911-stm8-swim-communication-protocol-and-debug-module-stmicroelectronics.pdf>

## 使用许可

[MIT](LICENSE) © Tifer King

[img1]: doc/SWIM-1.png "SWIM在全部协议解析器列表中"
[img2]: doc/SWIM-2.png "SWIM解析器配置界面"
[img3]: doc/SWIM-3.png "SWIM使用界面1"
[img4]: doc/SWIM-4.png "SWIM使用界面2"
[img5]: doc/SWIM-5.png "SWIM使用界面3"