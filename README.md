缓解 OnePlus 9 和 9 Pro 在低亮度下的低频 PWM 调光频闪问题。

Mitigation for OnePlus 9 (Pro) low frequency and low duty PWM when in low brightness.

本软件需要 root 权限。

Root required.

仅推荐 OxygenOS 系统使用，ColorOS 系统设置已自带低亮度下降低频闪功能。

Recommended for OxygenOS users only. ColorOS system already has this feature.

推荐使用通知栏快速设置。

The notification bar quick setting tile is recommended.

原理 / Implementation: `/sys/kernel/oplus_display/dimlayer_hbm`

The `dimlayer_hbm` mode is set to 1 when in Finger On Display(FOD) mode,
where the display is set to maximum brightness and an alpha layer is shown on top of the display,
which is required for the under screen fingerprint camera to work properly (PWM is bad for sampling).
