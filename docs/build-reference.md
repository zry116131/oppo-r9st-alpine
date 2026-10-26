# 构建参考

这是已使用配置的记录，不是自包含整机固件构建器。

## 固定来源

- 内核：`XuexGao/android_kernel_oppo_msm8937`，提交 `36b11cbda205f0a76d8f548554b56d5601d0dc02`，当时分支 `lineage-22.1-k4.19`。
- Mesa：`lfdevs/mesa-for-android-container`，提交 `401198fbcf61f74bf7214e1be57f811fc74c8358`，当时分支 `adreno-main`，报告版本26.1.0-devel。
- Alpine：3.24.3 aarch64。构建应使用大小写敏感 Linux 文件系统；macOS 默认文件系统曾造成源码大小写碰撞。

## 内核

使用 Ubuntu22.04 ARM64 构建环境、Clang13及 GNU AArch64/ARM32 交叉工具。公开的 `config/kernel-4.19.325.config` 是已测试配置快照。先核对固定提交，应用 `patches/kernel/` 的三个补丁，再使用配置构建；构建兼容补丁见 `patches/build/`。需要将该补丁引用的 `size-append.py` 放在内核 `scripts/` 下。

显示/触摸 DT 修改见 `reference/device-tree-fixes.dtsi`。必须对本机面板和对应 pinctrl 节点应用；保留电源、存储、触摸及面板描述，不照搬别的机型整棵树。完整 rootfs/initramfs、OEM启动封装和镜像签名材料不在仓库内，因此仅有这些公开文件不能重现最终 boot 哈希。

## Mesa

在 Alpine aarch64 环境应用 Mesa 补丁，使用 Meson/Ninja。已验证选项：

```sh
meson setup build --prefix=/opt/r9st-mesa --libdir=lib --buildtype=release   --wrap-mode=nofallback -Dgallium-drivers=freedreno -Dfreedreno-kmds=kgsl   -Dvulkan-drivers= -Dplatforms=x11 -Dglx=disabled -Degl=enabled   -Dgbm=enabled -Dgles1=disabled -Dgles2=enabled -Dllvm=disabled   -Dgallium-rusticl=false -Dvideo-codecs= -Dvalgrind=disabled -Dbuild-tests=false
ninja -C build
```

需要匹配机型的 `a530_pm4.fw`、`a530_pfp.fw`、`a506_zap.b00/b01/b02/mdt`，自行从合法取得的原厂材料提取；本仓库不分发固件。

`tests/egl-probe.c` 使用 EGL/GLES2 编译链接，经 `r9st-gpu-run` 运行。验证的是 FD506 硬件离屏渲染及像素读回，不是 GLX、Vulkan、桌面合成或应用兼容性。

## 运行脚本

C helper 用目标 aarch64 libc/toolchain 构建；原实验采用静态编译。Shell/OpenRC 文件的路径需按 `runtime/README.md` 安装和设置。发布副本移除了设备序列号校验常量，改为必填 `R9ST_EXPECTED_ROOT_ID`，**这些参数化副本未作为整套包重新部署到手机**；原机运行版与公开源码之间的区别已在此说明。
