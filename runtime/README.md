# 运行组件参考

本目录提供已经实机验证过的实现的公开副本，不是一键安装器。保留已有 USB/Recovery 维护通路后再逐项集成。

- `src/fb-refresh.c` → `/usr/local/bin/r9st-fb-refresh`：面板提交 helper。
- `src/r9st-keys.c` → `/usr/local/bin/r9st-keys`：实体键、触摸抓取及显示电源控制。
- `bin/*` → `/usr/local/bin/`：Xorg会话启动、全屏终端、隔离Mesa包装。
- `openrc/*` → `/etc/init.d/`，需要执行权限。依赖的 `r9st` 基础服务负责 USB ECM/SSH，因含设备专属配置未公开安装实现。
- `sbin/r9st-suspend` → `/usr/local/sbin/r9st-suspend`：root 调用，例如 `r9st-suspend 120`，单位秒；这是RTC兜底时限，不是进入休眠前延时。
- `config/xorg-r9st.conf` → `/etc/X11/xorg-r9st.conf`。当前触摸节点event3必须在目标设备重新核对。
- `/etc/r9st-session-mode` 写入 `console` 可启用全屏终端；其他值使用窗口式会话。

公开的 CPU/GPU OpenRC服务与休眠命令需设置 `R9ST_EXPECTED_ROOT_ID`，内容必须等于本机 `/etc/r9st-root-id`。为服务配置可在对应 `/etc/conf.d/<service>` 中设置；命令可通过调用环境传入。不得为了省事删除身份检查。

Xorg使用本地MIT cookie并关闭TCP监听；普通用户为alpine，示例固定UID1000。未提供屏幕键盘，触摸可以选择文字，输入需SSH或后续适配的键盘。

默认电源短按仍是关屏/亮屏，不调用深度休眠。休眠命令要求RTC未被其他任务占用、当前选中deep、USB角色为peripheral或none；临时断开USB数据连接，唤醒后恢复。不是已验收的无限时长待机方案。

现有限制：helper异常被SIGKILL时无法执行清理；锁残留需确认没有任务运行后处理。按键服务未实现音频混音器控制。不要把本目录当成跨设备通用电源管理框架。
