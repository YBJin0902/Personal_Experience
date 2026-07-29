# LCM 114 Mini LED 國科會計畫 APP

開發版載 NPU 搭配 USB Port。

將擷取到的 CA310 USB 訊號直接傳入模型中做推論。

螢幕訊號為：LVDS & eDP/DP

---

</br>

## Application version note

</br>

### Ver 1.0.0

基本功能皆有，串接代測面板與色彩分析儀，色彩分析儀所收到的 CIE 1931 直接送到 NPU 最推論，推論輸出面板顏色信心值。

</br>

- USB：CA310

- Display port：LVDS

- NPU：
	- driver version：2.3.2
	- model：resnet

</br>

### Ver 2.0.0

更新 runloop 功能：存檔時清理 buffer

更新 NPU runtime 輸出 log files

更新 Panel detect :
- 指定 LVDS Panel 輸出顏色並同時檢測與輸出 log

</br>

### Ver 3.0.0

更新 NPU model
- 新增 "black" 辨識
- 增加網路深度
- 新增輸入維度，完整 X Y Lv x y

</br>