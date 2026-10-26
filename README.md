# OPPO R9st 原生 Alpine Linux 移植记录

截至 **2026-10-26**。面向希望研究旧款高通手机原生 Linux 的开发者，记录一台 OPPO R9st（Snapdragon 625 / MSM8953、Adreno 506）的实际适配过程。

当前阶段：**原生启动、持久化存储、显示触摸、CPU 调频/空闲、GPU 离屏渲染，以及短时深度休眠/电源键唤醒已跑通**。这是实验性移植记录，不是可以直接刷入任意 R9s/R9st 的成品发行版。

系统采用 Alpine 3.24.3 + 下游 Linux 4.19.325，不依赖 Android 用户空间运行。项目最初探索 Armada，后根据用途转为 Alpine；没有 Steam 适配目标。测试机使用后换的屏幕、电池，移动网络不在目标范围内。

## 当前状态

| 项目 | 实机证据与边界 |
|---|---|
| 启动与存储 | 单系统 Alpine，合并后的连续根分区约 57.74 GiB；重启后文件保留 |
| 恢复与调试 | 保留第三方 Recovery，Recovery 下 root ADB；Alpine 下 USB ECM + 密钥 SSH |
| 显示与触摸 | 1080×1920 竖屏；全屏终端自动启动；真实拖动选字和关屏亮屏已验证 |
| 触摸中断 | 修正 GPIO65/17 引脚配置错配，空闲时约 2,000 次/秒的无效中断消失 |
| CPU | 八核上线，schedutil 动态调频，652.8 MHz 空闲 / 2.016 GHz 短时负载验证，C1 空闲计数增长 |
| GPU | KGSL + 隔离安装的 Mesa Freedreno，FD506 GLES 3.1；30 次着色器绘制、五点像素读回通过 |
| GPU 限制 | 抢占暂关闭；桌面仍软件 fbdev，GPU 屏幕呈现/桌面加速尚未打通；没有 Vulkan 验收 |
| 休眠 | 1 次 s2idle、4 次 deep 短时循环成功，其中一次确认由电源键 IRQ 唤醒；恢复后 GPU 回归通过 |
| 电源键 | 默认短按仍只关屏；深度休眠通过单独命令调用，含 RTC 定时唤醒兜底 |
| 音量键 | 按下/释放事件已确认；无声卡，尚不能实际调节播放音量 |
| 电池 | 能读取电压、温度和 SOC；曾出现开机后 0%，后续恢复，准确性和替换电池校准未验收 |
| 待适配 | Wi-Fi、蓝牙、音频、USB OTG/外接键盘、传感器、长时待机与纯电池功耗 |

## 阅读顺序

1. [修改方案与原因](docs/changes.md)
2. [验证记录与失败案例](docs/validation.md)
3. [构建参考与依赖](docs/build-reference.md)
4. [存储、启动链及恢复边界](docs/storage-and-recovery.md)
5. [运行脚本说明](runtime/README.md)
6. [已验证产物哈希](evidence/artifacts.json)

仓库提供补丁、配置快照、辅助程序源码及脱敏记录。**不包含 boot 镜像、完整 rootfs、原厂固件、签名包、校准/NV 数据、设备备份、SSH 密钥或内存转储**。公开文件经过独立整理，完整重建整机镜像仍需要本机型匹配的启动材料、固件和根文件系统。

## 重要经验

- 优先对照原机设备树和正常 Recovery 的行为；同一 SoC 的参考设备树可能保留其他机型引脚。
- GPU 初始化成功不等于能渲染，离屏渲染成功也不等于桌面已经加速。
- 关屏不等于系统休眠；CPU idle 不等于深度睡眠；短时恢复通过不代表长期续航已经验证。
- 活动 USB 调试会阻止本机进入系统休眠，先让 USB 进入低功耗，再在恢复路径中重连。
- 不读取本移植内核的完整 GPIO debugfs 转储：曾触发 external abort。定点排查代替全量寄存器遍历。

## 来源与致谢

- [XuexGao/android_kernel_oppo_msm8937](https://github.com/XuexGao/android_kernel_oppo_msm8937)：本次下游 4.19 内核基础。
- [lfdevs/mesa-for-android-container](https://github.com/lfdevs/mesa-for-android-container)：KGSL Mesa 用户空间基础。
- [Alpine Linux](https://alpinelinux.org/)、Linux、Mesa/Freedreno、Xorg、Openbox、xterm 及原始驱动作者。

精确提交及派生补丁见构建文档。本项目与上述项目及 OPPO 无官方隶属关系。许可证范围见 [LICENSE.md](LICENSE.md)。
