# Hardware

USB Lab 硬體

版本：AD22、24

# 基本介紹與使用

### 設定中文

* 按下齒輪後，點選 General > Locallization，勾選 Use localized resources

![image](https://hackmd.io/_uploads/B15HWEGNR.png)

</br>

### 新增專案

* 檔案 > 新增 > 專案
    * Project Name：專案名稱
    * Floder：存檔位置

</br>

* 專案中新增各類檔案
    * 對專案點擊右鍵 > 點選新增檔案到專案

</br>

* 工作區面板
    * 點選檢視，下滑即可看到

</br>

### 基本認知

* Schematic
    * 原理圖、電路圖
    * 已元件為主的電路原理圖
    * 需先設計 SCH 原理圖才能匯至 PCB 設計與繪製電路板

</br>

* PCB
    * 電路板設計
    * 將已設計好的 SCH 繪製成 PCB 電路板圖
    * 又可以稱為 Layout 圖

</br>

* Library 類型
    * 設計不在預設裡的元件及電路圖
    * 一樣分為 SCH 與 PCB

</br>

* 在開始繪製與設計之前都應該先確認電路與材料，同時也需要先查找個元件之資訊，繪製順序應該為：Library > SCH > PCB

</br>

# 電路板基本認知

### 電路板基本板層分級

* 上層：Top Layer
    * 電路板的上層電路
    * 在 AD 中以紅色線段表示

</br>

* 元件 (component)
    * 同一種元件 (代號或大小)，可能會具有多種不同的型號 Ex. 10pF 電容(Cap Semi) 分為：0402、1608[0603]…等
    * 藉由Library設計可以有多種不同元件使用，只是需要自行繪製與設計
    * 每一個元件都一定會有 Datasheet 可以查詢，Datasheet 內容包含有元件的各項尺寸、元件定義與元件相關數據
    * 在SCH的設計中會有代號的設定，不同的元件會有各類不同代號，如下表所示



| 元件類型       | 代號 |
| -------------- | ---- |
| 集成電路、晶片 | U    |
|二極體|D|
|電阻|R|
|電容|C|
|排針、排坐|P|
|電感|L|
|變壓器|T|
|發光二極體|LED|
|可變電阻|MOV|
|振盪器|X or Y|
|開關|SW|
|繼電器|K|
|接頭類|J|

</br>

# Library

### Datasheet 查找

* 各元件之尺寸與各項詳細數據
* 根據 Datasheet 所提供之詳細尺寸建製
* 須注意的是同一名稱的元件可能會有不同的封裝，需細找到同一封裝之元件

</br>

### Library 分為 Schematic 與 PCB

* 建置順序應為 PCB Library > Schematic Library
* 在AD中需要將兩項結合起來，腳位對應正確才算建置成功
* 如下附圖一與圖二所示，分別為 PCB Library 與 Schematic Library

</br>

![image](https://hackmd.io/_uploads/S190EVfN0.png)

</br>

![image](https://hackmd.io/_uploads/HJeySVGER.png)

</br>

# PCB Library

## Altium Designer 建置環境

### 工作欄

* 如下附圖所示，為最主要工作面板從左至右依序為：
    * 標籤用
    * 標籤用
    * 單一選取工具
    * 框選用選取工具
    * 排列圖件工具
    * 3D模型工具
    * 焊點建置工具
    * 導孔建置工具
    * 字串 (註解)
    * 線段放置工具 (有分板層)
    * 禁制線段放置工具 (Keep-Out Layer)
    * 距離測量線段放置工具
    * 鋪銅切除

</br>

* 一般最常用為：焊點建置、字串、線段放置

</br>

![image](https://hackmd.io/_uploads/BkueSrGER.png)

</br>

### PCB Library選單

* 如下附圖所示，為 PCB Library 選單
    * 請將所有的元件建置在同一個 Library 專案底下即可，不要一直重複建置
    * 從上到下依序為：
    * Footprints 已件製的元件
    * Footprints Primitives 當前元件所使用各組件屬性之數據
    * 建置預覽圖

</br>

![image](https://hackmd.io/_uploads/BJAirrMNC.png)

</br>

### 屬性選單

* 點選不同的放置物品會有相對應的屬性設定
* 如下附圖所示，以放置好的焊點屬性為例
* 根據每個元件不同的用法可以設定不同的型號

</br>

![image](https://hackmd.io/_uploads/ry6RrHfV0.png)

</br>


## 建置方式

在 Altium Designer 中對於 PCB Library 有分成幾種方式設計，自行繪製或是可以透過 Altium Designer 自帶的功能進行設計，以下將一一介紹。

請切記一率都先找好 Datasheet 的資料過後才可以開始，並且一定要留意的就是尺寸的單位，有分成 mil 與mm !!

</br>

## 自行設計

在開始自行設計之前請先詳細閱讀關於 AD 的版面與工具列，以下將開始逐步教學如何自行設計元件。

</br>

1. 點選 PCB Library 選單中的 Add 功能新增一個元件

</br>

2. 點選 PCB Library 選單中的Edit即可對於該元件進行編輯

    * 如下附圖所示，編輯內容為
    * 名稱，此元件封裝的命名
    * 簡介，對於此元件的敘述
    * 類型
    * 高度
    * 空間
    * **請注意元件的命名一定要按照規定且清楚**

</br>

![image](https://hackmd.io/_uploads/rkBn8rzNR.png)

</br>

3. 焊點放置
    * 根據所選用元件的不同會有不同的焊點設定
    * 基本分為：圓形過孔設計 (上下層焊點或單一層焊點) 與單面圖形焊點
    * 如下附圖所示，為焊點屬性設定表單中的基本屬性設定，以下將介紹常用類型 : 
        * Designator：設定該焊點的編號，需要注意次編號必須與SCH Library中的編號相對應
        * Layer：設定該焊點的板層
        * Template：預設焊點類型，在同一專案中若是已經有設定過的類型則會自動記憶起來
        * (X/Y)：在座標中的位置
        * Rotation：此焊點的角度

</br>

![image](https://hackmd.io/_uploads/B1PWwSMVC.png)

</br>

* 如下附圖所示，為焊點屬性設定表單中的焊點設定，以下將介紹常用類型 : 
    * Preview：此焊點的預覽圖
    * Simple：最簡單的圓形過孔焊點
    * Shape：此焊點的形狀
    * (X/Y)：此焊點的形狀設定 (長/寬)
    * Round、Rect、Slot：為此焊點的過孔形狀
    * Ｈole Size：此焊點過孔的大小

</br>

![image](https://hackmd.io/_uploads/HkSBwSfER.png)

</br>

4. 線段的繪製
* Top Layer：元件上方電路繪製
* Bottom Layer：元件下方電路繪製
* Top Overlay：元件外型繪製
* 每個線段的粗細都需要額外注意

</br>

## IPC Compliant Footprint Wizard

* 此繪製工具為符合 IPC 封裝之元件設定工具

* 如下附圖所示，只需要選擇好相對應的元件封裝型號，接著步驟逐一設定即可產出

</br>

![image](https://hackmd.io/_uploads/SJ35wrG40.png)

![image](https://hackmd.io/_uploads/BkbsDBfVC.png)

</br>

* 最常用來設計晶片IC

</br>

## 零件精靈

* 此繪製工具為繪製一般元件之設定工具

* 如下附圖所示，只需要選擇好相對應的元件封裝型號，接著步驟逐一設定即可產出

</br>

![image](https://hackmd.io/_uploads/H1PJurGE0.png)

</br>

# SCH Library

## Altium Designer 建置環境

### 工作欄

* 如下附圖所示，為最主要工作面板從左至右依序為：
    * 標籤用
    * 單一選取工具
    * 框選用選取工具
    * 放置接腳
    * 放置 IEEE Symbol
    * 放置線段 (圖形類)
    * 字串 (註解)

</br>

* 一般最常用為：放置接腳、字串、放置線段

</br>

![image](https://hackmd.io/_uploads/H1moOHz40.png)

</br>

### SCH Library 選單

* 如下附圖所示，為 SCH Library 選單
* 請將所有的元件建置在同一個 Library 專案底下即可，不要一直重複建置
* 選單會顯示所有已建置之元件原理圖

</br>

![image](https://hackmd.io/_uploads/S14R_rzN0.png)

</br>


### 屬性選單

* 在 SCH Library 中，屬性選單分為兩種，一種為 Component 整體元件屬性選單；另一種為單一物件屬性選單
* 如下附圖所示，為 Component 元件屬性選單
    * Design Item ID：該元件之 ID 編號
    * Designator：該元件之指示符號
    * Comment：該元件之描述

</br>

![image](https://hackmd.io/_uploads/rkF-YBfE0.png)

</br>

* 如下附圖三所示，為單一物件屬性選單
    * Designator：腳位編號 (需要與 PCB Library 相對應)
    * Name：腳位命名 (需要完整且明確)
    * Electrical Type：腳位類型

</br>

![image](https://hackmd.io/_uploads/H1qMFBME0.png)

</br>

## 建置方式

在 Altium Designer 中對於 SCH Library 有分成幾種方式設計，自行繪製或是可以透過Altium Designer自帶的功能進行設計，以下將一一介紹。

</br>

## 自行設計

在開始自行設計之前請先詳細閱讀關於 AD 的版面與工具列，以下將開始逐步教學如何自行設計元件。

</br>

1. 點選 SCH Library 選單中的新增功能新增一個元件 : 
* 如下附圖所示，為點選後顯示之視窗
* 可以先對 ID 進行命名 (該元件之名稱)，若事後想更改點選編輯即可

</br>

![image](https://hackmd.io/_uploads/Bk83trzNC.png)

</br>

2.	圖形放置
* 對放置線段點右鍵即可以選擇多種不同的圖形

</br>

3.	放置接腳
* 點選放置接腳擊可以放置
* 須注意，要將有X標誌的地方朝向圖形的外部，如下附圖所示，也就是將有文字的地方朝內
* 需要將接腳的 Designator 命名，並且與 PCB Library 一樣

</br>

![image](https://hackmd.io/_uploads/ByYZ5rzVA.png)

![image](https://hackmd.io/_uploads/r1JGqHfNR.png)

</br>

## Symbol Wizard

* 此建置工具為幫助建置元件，根據不同的腳位與型號選擇相對應的 Layout Style 即可，如下圖一所示，為 Symbol Wizard 視窗

### Layout Style

* Dual in-line
    * 圖形雙邊編號，1 ~ N

</br>

![image](https://hackmd.io/_uploads/H1uwqrzN0.png)

</br>

* Quad side
    * 圖形四邊編號，1 ~ N

</br>

![image](https://hackmd.io/_uploads/SysY9rf4C.png)

</br>

* Connector zig-zag
    * 圖形雙邊編號，1 ~ N

</br>

![image](https://hackmd.io/_uploads/BkvjcBfER.png)

</br>

* Connector
    * 圖形雙邊編號，1 ~ N

</br>

![image](https://hackmd.io/_uploads/SyzaqSzEA.png)

</br>

* Single in-line
    * 圖形單邊編號，1 ~ N

</br>

![image](https://hackmd.io/_uploads/HyK6cSz4R.png)

</br>

* **皆須注意順序**

# Library 結合

當所有前製工作已完成，則可以將PCB與SCH的Library結合在一起。

</br>

1. 點擊在 SCH Library 中的 Add Footprint，如下附圖所示

![image](https://hackmd.io/_uploads/Sk5fsSM4R.png)

</br>

2. 瀏覽
* 如下圖所示，選擇相對應之 PCB 即可

![image](https://hackmd.io/_uploads/SkAEjrfVR.png)

</br>

3. 接腳對應
* 如下圖三所示，必須確認所有的接腳是否有對應成功

![image](https://hackmd.io/_uploads/SkR8jSGNC.png)

</br>

# Schematic

### 注意事項

開始繪製之前有幾項必須要注意之事項，如下詳列
* 元件命名必須清楚且元整
* 網路名稱佈線必須命名清楚且完整
* Designator必須命名不重覆

</br>

## 工作欄

* 如下附圖所示，為最主要工作面板從左至右依序為：
    * 標籤用工具
    * 單一選取工具
    * 框選用選取工具
    * 排列圖件工具
    * 放置零件 (Components)
    * 放置導線 (導線/網路名稱)
    * 放置符號 (GND/VCC)
    * 信號束線
    * 放置電路圖方塊 (多SCH合併)
    * 放置輸出入Port
    * 放置參數
    * 文字 (註解)
    * 放置一般線段

</br>

* **一般最常用為：放置零件、放置導線、放置符號、文字、放置一般線段**

</br>

![image](https://hackmd.io/_uploads/r1DgnSzE0.png)

</br>

**Schematic 的工作環境中有多項功能選單以下介紹常用，包括有：Components、布置導線**

</br>


## Components，元件功能選單

* 在這一選單中可以匯入以建置好的 Library、一般設備與連接用設備
    * Library：自行建置的
    * 一般設備：電阻、電容、電感…等
    * 連接用設備：排真、排坐…等
* 拉取表單內容即可以匯入圖紙中
* 須注意每個元件的型號
* 須注意每個設備之命名，必須按照規定
* 如下圖以電阻為示範
    * Designator：該元件之編號，須注意每個元件的編號需要不一樣
    * Comment：該元件之敘述

</br>

![image](https://hackmd.io/_uploads/B1FjTBGV0.png)

</br>

* 如下圖所示，Parameters為該元件之參數設定
    * 須注意的是 Value，必須注意型號的選擇

</br>

![image](https://hackmd.io/_uploads/B1DhaBGEC.png)

</br>

## 布置導線

* 快捷鍵，Ctrl + W
* 布置導線的方法分為兩種，一般導線布置與網路名稱布線
    * 一般導線：在元件之間建立實質的導線
    * 網路名稱：在元件之間建立利用名稱所構成之虛擬導線，該方法較為推薦
    * 若是使用網路名稱的方式布線，請注意一定要將名稱命名完整
* 如下圖所示，為一般布線與網路名稱布線之範例，利用一般布線連結 GND；利用網路名稱連接 5V 與 A6

</br>

![image](https://hackmd.io/_uploads/SyGg0HfNR.png)

</br>

## 完成繪製

當建置完畢時點選「設計」中的「Updata PCB Document」即可以匯入至指定 PCB 檔，其中需點選「檢查變更動作」與「執行變更動作」

**注意 : 「執行變更動作」必須全數通過檢查才可以**

</br>

# PCB

### 注意事項

* 開始繪製之前有幾項必須要注意之事項，如下詳列 : 
    * 板層分為上與下，Top Layer 與 Bottom Layer
    * 在繪製線段時所有設計線段不得小於 90∘
    * VCC 與GND 不得繞整圈電路板
    * 不管板層 VCC 與 GND 請不要經過任何訊號線
    * 控制跳點的數量
    * 在有限的板型中製作，不要無限放大

</br>

## 工作欄

* 如下附圖所示，為最主要工作面板從左至右依序為：
    * 標籤用工具
    * 標籤用工具
    * 單一選取工具
    * 框選用選取工具
    * 排列圖件工具
    * 放置零件 (Components)
    * 放置互動式導線 (電路線)
    * 調整互動式導線工具
    * 放置導孔 (跳點)
    * 鋪銅
    * 放置禁制線段 (板型外層)
    * 放置測量用線段工具
    * 文字 (註解)
    * 放置一般線段

</br>

* **一般最常用為：放置互動式導線、放置導孔、放置禁制線段、文字、放置一般線段**

</br>

![image](https://hackmd.io/_uploads/ryOcZLfNC.png)

</br>

**在從 SCH 匯入到 PCB 之後，所有元件都會在右下角，如下圖所示，請記得將紅色板先刪掉**

</br>

![image](https://hackmd.io/_uploads/Hk-TZ8GER.png)

</br>

## 開始繪製

1. 將所有匯入元件擺放好之後，再開始繪製電路，繪製之前請詳讀所有注意事項

* 板層劃分：
    * Top Layer
    * Bottom Layer
    * Keep-Out Layer

</br>

* 設計規則
    * Electrical / Clearance / Clearance：設定間距最低容許大小
    * Routing / Width：設定線段寬度

</br>

* 測量距離工具
    * 快捷鍵：Ctrl + M (取消 : Shift + C)

</br>

* 切換單位(mil/mm)
    * 快捷鍵：Q

</br>

2. 繪製互動式導線

* 快捷鍵：Ctrl + W
* 繪製時可以按下 TAB 調整線路設定
* 點選相對應板層可以繪製該板層之導線

</br>

3. 放置導孔

* 用於設計跳點孔位
* 連接上下板層

</br>

4. 放置禁制線段

* 繪製電路板外型
* 繪製完成之後，選取「設計」中的「板型設計」「根據選取物件定義板型」

</br>

# USBLab 雕刻機

一台60萬

### 需要檔案

1. 進入自己專案的PCB位置
EX. ![image](https://hackmd.io/_uploads/SkBJgUpvC.png)

2. 點選：檔案 > 輔助製造輸出
需要兩個檔案，佈線檔與鑽孔檔，分別為：Gerber Files 與 NC Drill Files
![image](https://hackmd.io/_uploads/BJgWeLaw0.png)

3. Gerber Files：
有版本差異需要注意！！
根據以下圖片設計
![image](https://hackmd.io/_uploads/SyCJWI6vA.png)
勾選 Top Layer 、 Bottom Layer 與 Keep-Out Layer
![image](https://hackmd.io/_uploads/HJ8QM86DA.png)


</br>

4. NC Drill Files
![image](https://hackmd.io/_uploads/HyijzLpPC.png)
</br>

```

注意：每次點選完確認都會跳到別的檔案，所有輸出都需要再 PCB 檔!!!

```

5. 檔案選取
![image](https://hackmd.io/_uploads/HyQUX8TwC.png)
需要四種檔案(類型)：
CAMtastic Bottom Layer Gerber Data、CAMtastic Top Layer Gerber Data、CAMtastic Keepout Layer Gerber Data 與 TXT(此txt檔名為 "專案名稱.txt" )
```

注意：若是設計的孔有橢圓形則會有兩種txt !!!

```

6. 雕刻機操作

```

注意：此影片為簡單操作請一定要先找學長詳細學習，並在使用時找學長陪同 !!

```

{%youtube c9_RimBFiq0 %}



# 快捷鍵總覽

```
!! 須注意必須在英文輸入法下才有效 !!
```
</br>

| 功能 | 輸入 |
| -------- | -------- |
| 測式距離定命令 | Ctrl + M |
| 消除測式距離定命令 | Shift + C |
| 關閉台頭顯示器 | Shift + H |
| 繪製導線 | Ctrl + W |
| 顯示單板層 | Shift + S |

</br>

# PCB電路圖檔案合併

Step 1. 先新增一個新專案

Step 2. 再專案中新增一個 PCB 檔

Step 3. 點選 Place，如下圖所示

</br>

![image](https://hackmd.io/_uploads/SJwmNUfNC.png)

</br>

Step 4. 點選電路板排板，顯示如下圖，在隨機地方放置即可 (最好是設計區塊)

</br>

![image](https://hackmd.io/_uploads/r17BEUGEA.png)

</br>

Step 5. 選取該電路板排板選項之屬性，點選 PCB Document，並選取檔案及可

</br>

![image](https://hackmd.io/_uploads/S1zwVUz4A.png)

</br>

# 匯入\安裝 Library

Step 1. 點選 Componets

Step 2. 點選選取欄旁的 Operations，如下圖所示

</br>

![image](https://hackmd.io/_uploads/rypcVIGV0.png)

</br>

Step 3. 點選 File-based Libraries Preferences，並點選系統，如下圖所示之視窗

</br>

![image](https://hackmd.io/_uploads/BkN3EIG4A.png)

</br>

Step 4. 首先選擇零件庫相對路徑，之後點選掛載，並選取相對應之檔案即可

</br>

# 共用型\自定義總零件庫

```
!! 檔案路徑需要在AD Library底下 !!

如：C:\Users\Public\Documents\Altium\AD22\Library\”在一個自己的資料夾”
```

Step 1. 新增 「整合式零件專案」

Step 2. 依照 Library 步驟依序新增零件

Step 3. 新增完成後儲存專案，之後編譯該檔案，如下圖所示

</br>

![image](https://hackmd.io/_uploads/SkWMBIMNA.png)

</br>

Step 4. 根據匯入 Library 方式匯入該零件庫，在此之後在該檔案新增零件即可 (新增完都需儲存與編譯)

</br>

# 階層式電路圖/模組化電路圖

簡單來說就是在一個專案底下利用多個 SCH 檔合併在一個主要的 SCH，方便以後更新或是以利複雜電路編輯。

Step 1. 在專案裡先新增另一個 SCH 檔案，如下圖所示

</br>

![image](https://hackmd.io/_uploads/rJNUHIGER.png)

</br>

Step 2. 先決定主要檔案與子檔案
* 主檔案：繪製總電路
* 子檔案：繪製周邊電路

</br>

Step 3. 設計子檔案，此時需要用到 Port 的設定，如下圖工具列所示，右邊第四個為放置 Port 接腳

</br>

![image](https://hackmd.io/_uploads/HkNcS8GEC.png)

</br>

Step 4. 以下設計為例在繪製好周邊電路後，將需要額外與主電路連接的接腳接上 Port ，如下圖所示

</br>

![image](https://hackmd.io/_uploads/H11pHUGE0.png)

</br>

Step 5. 在主電路圖中新增子電路區塊電路，如下圖工具列所示右邊第五個為放置放置電路方塊圖

</br>

![image](https://hackmd.io/_uploads/H19lILGV0.png)

</br>

Step 6. 放置好電路方塊圖後，即可從屬性欄位選取該子電路檔案，如下圖所示

</br>

![image](https://hackmd.io/_uploads/Sk1G8IfE0.png)

</br>

Step 7. 在選取好檔案之後需要放置方塊圖進出點，在放置完成後，點選屬性欄位輸入跟子電路圖一模一樣的名稱，如下圖所示

</br>

![image](https://hackmd.io/_uploads/S1iS88fV0.png)

</br>

