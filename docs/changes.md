# 修改方案

## 显示链路

原生系统启动但面板黑屏，曾出现 DSI 刷新等待超时。删除本机 EA8064 面板节点中的 `qcom,mdss-dsi-force-clock-lane-hs` 后，三色块及触摸轨迹通过实机确认。该结果是本机有效修正，不推广为所有面板的通用设置。

Xorg 无法识别 framebuffer 对应平台设备。曾尝试给 `framebuffer_alloc()` 传入 parent，但改变了旧驱动 runtime PM 关系，导致 framebuffer 打开失败。最终保留原分配关系，仅增加/移除 `device` sysfs 链接，见 kernel 补丁 0001。

当前 Xorg fbdev + Openbox + xterm 使用软件绘制。命令模式面板需额外 `FBIOPAN_DISPLAY` 提交；辅助程序采用绝对时间约 16.67 ms 调度。该提交周期不等于实测桌面帧率。关闭 ShadowFB 的试验使拖动更差，已撤回。

## 触摸

驱动上报 `ABS_MT_WIDTH_MAJOR`，但缺少有效范围，libinput 拒绝设备。补充 0..256 范围，与原驱动上报的宽度及重复事件 +1 行为对应。

更关键的问题是：触摸节点实际 IRQ 为 GPIO17，但 `pmx_ts_int_active/ts_int_active` 的 mux/config 都指向 GPIO65。它没有配置实际中断线的上拉，空闲发生约每秒 2,000 次中断，读取控制器中断状态却为 0。

仅将该节点下两处 `pins = "gpio65"` 改为 `"gpio17"`，其余上拉、驱动强度保持原样。Recovery 对照空闲计数不增长；修正后 Linux 也不增长，真实触摸及文字选择仍正常。见 `reference/device-tree-fixes.dtsi`，这是改动片段而非可直接编译的完整设备树。

## CPU

启动封装继承了 `lpm_levels.sleep_disabled=1`。启用 schedutil，并将对应运行时参数设为 N 后，空闲/短负载调频和 C1 计数通过检查。没有提高频率、修改电压或放宽热保护。

## GPU

固件必须来自匹配机型。A506 在该驱动中请求 A530 PM4/PFP 微码以及 A506 ZAP，文件名不能仅凭 GPU 数字推断错误。

CP 初始化遇到写保护故障；诊断证明故障地址恰好等于 scratch buffer 地址。旧原机驱动未将该缓冲区标为 privileged，新驱动则统一设置。补丁仅对 A506 scratch 去掉 `KGSL_MEMDESC_PRIVILEGED`，保留随机化及其他缓冲区保护。

另有抢占握手超时，暂以 `preemption=0` 兼容。不能将 KGSL open 返回成功当作整个初始化流程成功。

Mesa fork 的 KGSL surfaceless 回退路径缺少创建 screen/device，发生 NULL 解引用。补齐初始化和失败清理后，实际 shader/readback 测试通过。Mesa 独立安装到 `/opt/r9st-mesa`，不替换系统库；只通过包装命令选择它。

## 关屏和休眠

按键守护程序读取 `qpnp_pon` 与 `gpio-keys`。关屏：抓取触摸输入、暂停刷新 helper、FBIOBLANK POWERDOWN、亮度设为 0；亮屏做对应恢复。默认行为仍是系统继续运行。

深度休眠首先拒绝活动 USB：`7000000.ssusb` 返回 EBUSY。驱动 `mode=none` 可在软件侧断开 USB，使 runtime 状态进入 suspended；恢复时还原原角色。已验证 RTC 及电源键唤醒，用户确认显示/触摸正常。持久命令包含设备身份校验、独占锁、RTC 占用检查和异常退出恢复；默认 RTC 120 秒，长时间/纯电池使用仍待验证。
