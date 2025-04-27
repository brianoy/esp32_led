# esp32_led
用ESP32製造一個可控制的led燈條，須配合android app做使用

## 軟體-android app

待補

---

## 韌體-ESP32

### 1.安裝arduino ide

https://www.arduino.cc/en/software

### 2.建立開發板編譯環境

可以參考其他人的文章

https://hackmd.io/@arduino/esp32

https://sites.google.com/site/arduinochutiyan/esp32_%E5%9F%BA%E7%A4%8E/1-cp2102x-%E9%A9%85%E5%8B%95%E7%A8%8B%E5%BC%8F%E5%AE%89%E8%A3%9D

進入arduino ide->上方`工具`->`開發板`->`開發板管理員`->搜尋`ESP32`->下載espressif提供的`ESP32`->版本選擇最新->`安裝`->等待下載完成

### 3.驅動
我購買的是帶CP2102 UART的NodeMCU-32s，視自己購買的轉換器下載對應的驅動
|UART|驅動下載網址|
|:-:|:-:|
|CH340|https://www.wch.cn/downloads/ch341ser_exe.html|
|CP2102|https://www.silabs.com/developer-tools/usb-to-uart-bridge-vcp-drivers?tab=downloads|

### 4.程式碼上傳到開發板

#### 開發板設定

上方`工具`->`開發板`-> `ESP32` -> 捲動到中間的位置選擇Node32S

上方`工具`->`開發板`-> `連接埠`選擇對應的`COM X`

上方`工具`->`Upload Speed`-> 選擇最快的，我這邊是`921600`

#### 開始上傳

上方`上傳`按鈕，注意有一些板子要在輸出視窗出現`connecting...`的時候，按下板子的`IO0`按鈕進入燒錄模式才會上傳成功，可以自己買一個10µf的電容焊在EN-GND，這樣就可以不用每次上傳都按按鈕了

---

## 硬體

### esp32 開發+轉接板

|電路圖|eagle 3d圖|
|:-:|:-:|
|<img src="https://user-images.githubusercontent.com/24865458/209184921-fefda4e8-f83a-4ead-a8e7-ffb5af8c9ecd.png" width="90%">|<img src="https://user-images.githubusercontent.com/24865458/209185310-f2b804c0-40cb-456e-8bad-e8ba2a46cf84.png" width="90%">|

### 燈條

| 晶片            | 晶片位置 | 斷點續傳 | 電壓 | 資料通道 | Pin腳 | 附註 |
|:---------------:|:---------:|:---------:|:-----:|:---------:|:------:|:------:|
| WS2811         | 外置1控3 | 否       | 12V  | RGB      | VDG   | 為了降壓所以以三顆燈珠為群組一起控制 |
| WS2818B        | 外置1控3 | 是       | 12V  | RGB      | VBDG  | 支援斷點續傳的WS2811               |
| WS2801         | 外置1控1 | 否       | 5V   | RGB      | VCDG  | 老晶片 但是比WS2812傳輸速度高       |
| WS2812         | 內置1控1 | 否       | 5V   | GRB      | VDG   | 將晶片內置的WS2801                 |
| WS2812B        | 內置1控1 | 否       | 5V   | GRB      | VDG   | 更好的顏色表現及防反接的WS2812     |
| WS2812E        | 內置1控1 | 否       | 5V   | GRB      | VDG   | 節能的WS2812B但較暗               |
| WS2813B        | 內置1控1 | 是       | 5V   | GRB      | VBDG  | 支援斷點續傳的WS2812B             |
| WS2815         | 內置1控1 | 是       | 12V  | GRB      | VBDG  | 12V版本的WS2813B                 |
| SK6812         | 內置1控1 | 是       | 5V   | GRB/RGBW | VDG   | 多白光版本的WS2813                |
| SK9822         | 內置1控1 | 否       | 5V   | BGR      | VCDG  | 更高且可控傳輸速度的WS2812B        |

這邊選擇的是WS2812B，優點就是便宜，表格整理自[DERUN](https://web.archive.org/web/20230124150536/https://www.derunledlights.com/zh/the-difference-between-addressable-rgb-led-strip-ws2811-ws2812b-ws2813-ws2815-sk6812-sk9822/#subtt-7)
