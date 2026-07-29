# RTOS

本章將實作在STM32F1系列與HT32F52XXX系列
</br>

## 簡介與基礎認知

### 前後台系統
</br>

前後台系統常建設在一般的程序上，簡單來說就是直接裸機操作(直接套用while無限回圈內)。須注意此系統並沒有嵌入式操作系統的改念

1. 前台系統：中斷(interrupt)用於處理系統的異步事件(callback)。
2. 後台系統：程序是個死循環，在循環中不斷調用API函數完成所需的事件。

```
前台系統為中斷級，後台為任務級
```

![image](https://hackmd.io/_uploads/S1Ar1_XMA.png)

### RTOS 系統
</br>

即為即時性作業系統，並強調即時性，又分為軟即時與硬即時。將需要實現的功能劃分為多個任務，並具有可剝奪性。

```
1.軟即時：系統能讓絕大多數任務在確定時間內完成。

2.硬即時：系統必須使任務在確定的時間內完成。

3.可剝奪性：CPU執行多個任務中優先權最高的那個任務，即使CPU正在執行某個低階任務，當高階任務準備好時
，高階任務就會優先搶奪執行權。
```

代表作：FreeRTOS、UCOS、RT-Thread、DJYOS

![image](https://hackmd.io/_uploads/HyBAO_XfA.png)

### FreeRTOS 簡介

FreeRTOS 是一個可剪裁、可剝奪型的多任務核心，而且沒有任務數量限制。

官網：https://www.freertos.org/index.html

## 架構
FreeRTOS 的程式碼可以分為三個主要區塊：任務、通訊和硬體界面。

* 任務(Tasks)：FreeRTOS 的核心程式碼約有一半是用來處理多數作業系統首要關注的問題：任務，任務是擁有優先權的用戶所定義的 C 函數。task.c 和 task.h 負責所有關於建立、排程和維護任務的繁重工作。

* 通訊(Communication)：FreeRTOS 核心程式碼大約有 40% 是用來處理通訊的。queue.c 和 queue.h 負責處理 FreeRTOS 的通訊，任務和中斷(interrupt)使用佇列(佇列，queue)互相發送數據，並且使用 semaphore 和 mutex 來派發 critical section 的使用信號。(~~資料結構很重要~~)

* 硬體介面：同一份程式碼在不同硬體平台上的 FreeRTOS 都可以運行。大約有 6% 的 FreeRTOS 核心代碼，在與硬體無關的 FreeRTOS 核心和與硬體相關的程式碼間扮演著墊片(shim)的角色。

</br>

#### 原始碼

下載：https://freertos.org/a00104.html

* 核心程式碼：(Source)
    * tasks.c：掌管所有 task 的檔案
    * queue.c：管理 task 間的通訊
    * list.c：提供系統與應用實作會用到的 list 結構

* 硬體相關檔案：以 ARM Cortext-M3 為例，可在 Source/portable/GCC/ARM_CM3 中找到
    * portmacro.h：定義了與硬體相關的變數，如資料型態定義，以及與硬體相關的函式呼叫名稱定義(以 portXXXXX 命名)等，統一各平臺的函式呼叫
    * port.c：定義了包含與硬體相關的程式碼實作
    * FreeRTOSConfig.h：包含 clock speed, heap size, mutexes 等等都在此定義(需自行建立)

# Run on STM32F103C8T6

## CubeIDE Setting

```
須注意RTOS會占用Systik資源
```

1. 設定版本：CMSIS_V1

2. 相關設定：

</br>

![image](https://hackmd.io/_uploads/HJGUZoXzC.png)

</br>

* Kenerl setting：
    * USE_PREEMPTION：Enabled：RTOS使用搶佔式調度器；Disabled：RTOS使用協作式調度器（時間片）。
    * TICK_RATE_HZ：值設定為 1000，即週期就是 1ms。 RTOS系統節拍中斷的頻率，單位為 HZ。
    * MAX_PRIORITIES：可使用的最大優先權數量。設定好以後任務就可以使用從 0 到（MAX_PRIORITIES - 1） 的優先級，其中 0 位最低優先級，（MAX_PRIORITIES - 1） 為最高優先級。
    * MINIMAL_STACK_SIZE：設定空閒任務的最小任務堆疊大小，以字為單位，而不是位元組。如該值設定為128Words，那麼真正的堆疊大小就是 128*4 = 512 Byte。
    * MAX_TASK_NAME_LEN：設定任務名稱最大長度。
    * IDLE_SHOULD_YIELD：Enabled 空閒任務放棄 CPU 使用權給其他同優先權的使用者任務。
    * USE_MUTEXES：為 1 時使用互斥訊號量，相關的 API 函數會被編譯。
    * USE_RECURSIVE_MUTEXES：為 1 時使用遞迴互斥訊號量，相關的 API 函數會被編譯。
    * USE_COUNTING_SEMAPHORES：為 1 時啟用計數型訊號量， 相關的 API 函數會被編譯。
    * QUEUE_REGISTRY_SIZE：設定可以註冊的佇列和信號量的最大數量，在使用內核偵錯器查看信號量和佇列的時候需要設定此宏，而且要先將訊息佇列和訊號量進行註冊，只有註冊了的佇列和訊號量才會在核心偵錯器中看到，如果不使用核心偵錯器的話次巨集設定為 0 即可。
    * USE_APPLICATION_TASK_TAG：為 1 時可使用 vTaskSetApplicationTaskTag 函數。
    * ENABLE_BACKWARD_COMPATIBILITY：為1時可以使 V8.0.0 之前的FreeRTOS使用者程式碼直接升級到 V8.0.0 之後，不需要做任何修改。
    * USE_PORT_OPTIMISED_TASK_SELECTION：FreeRTOS 有兩種方法來選擇下一個要運行的任務，一個是通用的方法，另一個是特殊的方法，也就是硬體方法，使用MCU自帶的硬體指令來實現。STM32有計算前導零指令嗎，所以這裡強制為 1 。
    * USE_TICKLESS_IDLE：1：啟用低功耗tickless模式；0：保持系統節拍（tick）中斷一直運作。假設開啟低功耗的話可能會導致下載出現問題，因為程式在睡眠中，可用 ISP 下載辦法解決。
    * USE_TASK_NOTIFICATIONS：為 1 時使用任務通知功能，相關的 API 函數會被編譯。開啟了此功能，每個任務會多消耗 8 個位元組。
    * RECORD_STACK_HIGH_ADDRESS：為1時棧開始位址會被儲存到每個任務的TCB（假如棧是向下生長的）。

* Memory management setting：
    * Memory Allocation： Dynamic/Static 支援動態/靜態記憶體申請
    * TOTAL_HEAP_SIZE：設定堆大小，如果使用了動態記憶體管理，FreeRTOS 在創建 task、queue、mutex、software timer or semaphore 的時候就會使用 heap_x.c( x為1~5 ) 中的記憶體申請函數來申請記憶體。這些記憶體就是從堆 ucHeap[configTOTAL_HEAP_SIZE] 申請的。
    * Memory Management scheme：記憶體管理策略 heap_4。



3. Task 設定：

</br>

![image](https://hackmd.io/_uploads/SkD1Mo7f0.png)

</br>

* Task Name：任務名稱
* Priority：任務優先權
    * osPriorityIdle：6
    * osPriorityLow：5
    * osPriorityBelowNormal：4
    * osPriorityNormal：3
    * osPriorityAboveNormal：2
    * osPriorityHigh：1
    * osPriorityRealtime：0
* Stack Size(Words)：任務大小(容量)
* Entry Function：進入函數名稱

## 任務

需要先定義一個 C 的函數，然後再用 xTaskCreate() 這個 API 來建立一個 task，此 C 函數有幾個特點，它的返回值必須是 void，其中通常會有一個無限迴圈，所有關於這個 task 的工作都會在迴圈中進行，且函數不會有 return，FreeRTOS 不允許 task 自行結束 ( 使用 return 或執行到函數的最後一行 )。

### 任務狀態

![FreeRTOS-TaskWork](https://hackmd.io/_uploads/SkYMatXf0.png)

</br>

* 阻塞與掛起比較：
    * 阻塞：是說如果有個 task 將要等待某個目前無法取得的資源(被其他 task 佔用中)，則會被設為 blocked 狀態，這是被動的，OS 會呼叫 blocking API 來設定 task 進入 blocked queue。
    * 掛起：task 主動呼叫 API 來要求讓自己進入暫停狀態的。

* 每一種狀態 FreeRTOS 都會給予一個 list 儲存（除了 running)

### 建立任務 API
```cpp
portBASE_TYPE xTaskCreate( pdTASK_CODE pvTaskCode,
                           const signed portCHAR * const pcName,
                           unsigned portSHORT usStackDepth,
                           void *pvParameters,
                           unsigned portBASE_TYPE uxPriority,
                           xTaskHandle *pxCreatedTask );
```

* pvTaskCode：就是我們定義好用來建立 task 的 C 函數
* pcName：任意給定的 task name，這個名稱只被用來作識別，不會在 task 管理中被採用
* usStackDepth：堆疊的大小 (以 Byte 計算)
* pvParameters：要傳給 task 的參數陣列，也就是我們在 C 函數宣告的參數
* uxPriority：定義這個任務的優先權，在 FreeRTOS 中，0 最低，(configMAX_PRIORITIES – 1) 最高
* pxCreatedTask：handle，是一個被建立出來的 task 可以用到的識別符號

### 刪除任務 API

```cpp
void vTaskDelete( xTaskHandle pxTaskToDelete );
```

* pxTaskToDelete: 利用 handle 去識別出哪一個 task。 
* 這種可能性存在於如果在 loop 中發生執行錯誤 (fail)，則需要跳出迴圈並終止(自己)執行，此時就需要使用 vTaskDelete 來刪除自己。

### Ready list 資料型態

* 管理準備好要執行的 tasks

![image](https://hackmd.io/_uploads/S1ugycQfA.png)

* Task Control Block (TCB)

## 阻塞 (Blocked)

<font color="#f00">Task 的 blocked 狀態通常是 task 進入了一個需要等待某事件發生的狀態，這個事件通常是執行時間到了(例如 systick interrupt)或是同步處理的回應</font>，如果像while(1) 這樣的無限迴圈來作等待事件，會占用 CPU 運算資源，也就是 task 實際上是在 running，但又沒做任何事情，占用著資源只為了等待 event，所以比較好的作法是改用 vTaskDelay()，當 task 呼叫了 vTaskDelay()，task 會進入 blocked 狀態，就可以讓出 CPU 資源了

```
在STM32F103中語法為 osDelay()，單位ms
```

## 掛起 (Suspend)

* 如果一個 task 會有一段時間不會執行，那就可以進入 suspend 狀態。

```cpp
void vTaskSuspend( TaskHandle_t xTaskToSuspend )
```

## 恢復 (Resume)

* 將掛起的 task 恢復正常使用。

```cpp
void vTaskResume( TaskHandle_t xTaskToResume )
```

```cpp
BaseType_t xTaskResumeFromISR( TaskHandle_t xTaskToResume )

/*該函數為在中斷中使用*/
```

# Run on HT32F52352

## 環境建置

Holtek 官網下載 example code：https://www.holtek.com.tw/page/vg/HT32F52342-52

FreeRTOS 官網下載 code：https://www.freertos.org/a00104.html

![Screenshot 2024-05-05 012721](https://hackmd.io/_uploads/Bk8VleEfA.png)

下載 FreeRTOS202212.01

* 解壓縮 FreeRTOS202212.01 ＞ 進入 FreeRTOS ＞ 複製 License 與 Source 資料夾至自行移植之資料夾

* 對 Source 資料夾內部之 Keil 與 MemMang 加工
    * Keil：先進入 RVDS 資料夾 > 只留下 ARM_CM0 (因為 HT32F52352 為 Cortex-M0+)
    * MemMang：進入 MemMang 資料夾 > 只留下 heap_4.c

* 將該資料夾放置在 Holtek 之 exmaple code 總資料夾中
* 進入 > FreeRTOSv202212.01\FreeRTOS\Demo\CORTEX_M0+_NUCLEO_L010RB_GCC_IAR\Config 中，將FreeRTOSConfig.h 加到 > \library\CMSIS\Include 中

接著進入到 HT 的 example 中，點選隨機範例程式，以下將以最簡單的 GPIO Output 為範例

建置專案教學：



</br>


* 先在專案底下新增一個 Group (FreeRTOS)，並新增以下圖中檔案 

![image](https://hackmd.io/_uploads/Hk6iNgNzC.png)

</br>


* 新增方式為使用 Keil 中的 Manage Project Items

![image](https://hackmd.io/_uploads/SJ9VHgNzA.png)

</br>


* 增加 C/C++ Include Paths 需設置以下路徑

![image](https://hackmd.io/_uploads/B1quSeNGR.png)

</br>


* 打開 FreeRTOSConfig.h 需更改兩項地方：
    * extern uint32_t SystemCoreClock; => extern volatile uint32_t SystemCoreClock;

    * 取消靜態記憶體申請：#define configSUPPORT_STATIC_ALLOCATION 1 => #define configSUPPORT_STATIC_ALLOCATION 0

![image](https://hackmd.io/_uploads/ByuZv3jMC.png)


![image](https://hackmd.io/_uploads/rkGp8e4z0.png)

</br>

### 簡易任務實作

先以盛群版上的 LED1 與 LED2 簡單實作兩個 Task 實作

* 在 main 中加入 FreeRTOS.h/task.h

</br>

* 設立基本變數：
```cpp
#define LED1_TASK_PRIO		3         /*優先級*/   
#define LED1_STK_SIZE 		50        /*任務大小*/

TaskHandle_t LED1Task_Handler;        
void led1_task(void *pvParameters);


#define LED2_TASK_PRIO		3
#define LED2_STK_SIZE 		50  

TaskHandle_t LED2Task_Handler;
void led2_task(void *pvParameters);

```

</br>

* 設計 Task 內容：
```cpp
void led1_task(void *pvParameters)
{
	for(;;)
	{
		HT32F_DVB_LEDOn(HT_LED1);
		vTaskDelay(800);
		
		HT32F_DVB_LEDOff(HT_LED1);
		vTaskDelay(800);
	}
}

void led2_task(void *pvParameters)
{
	for(;;)
	{
		HT32F_DVB_LEDOn(HT_LED2);
		vTaskDelay(300);
		
		HT32F_DVB_LEDOff(HT_LED2);
		vTaskDelay(300);
	}
}
```

</br>

* 撰寫 main 函式
```cpp
int main(void)
{
    //GPIO_Configuration();
    taskENTER_CRITICAL();
    HT32F_DVB_LEDInit(HT_LED1);
    HT32F_DVB_LEDInit(HT_LED2);
	
    xTaskCreate((TaskFunction_t )		led1_task,     	
              (const char*    )		"led1_task",   	
              (uint16_t       )		LED1_STK_SIZE, 
              (void*          )		NULL,				
              (UBaseType_t    )		LED1_TASK_PRIO,	
              (TaskHandle_t*  )		&LED1Task_Handler);
							
    xTaskCreate((TaskFunction_t )		led2_task,     	
              (const char*    )		"led2_task",   	
              (uint16_t       )		LED2_STK_SIZE, 
              (void*          )		NULL,				
              (UBaseType_t    )		LED2_TASK_PRIO,	
              (TaskHandle_t*  )		&LED2Task_Handler);
	
							
	
    vTaskStartScheduler();
	
    while (1);
}
```

</br>

* 燒入驗證 

### UART 部分

實作 UART 部分，Task 的撰寫與一般實作內容相同 !!

中斷 (interrupt) 部分則可以直接使用 ht32f5xxxx_01_it.c 實作 !!

```cpp
char RXRD_BUF[100];
u32 RXRD;
u32 RXRD_Index = 0;
char rx_data;

void HTCFG_UART_IRQHandler(void)
{
     if(USART_GetFlagStatus(HTCFG_UART_PORT, USART_FLAG_RXDR) != RESET)
     {
            RXRD = USART_GetFlagStatus(HTCFG_UART_PORT, USART_FLAG_RXDR);

            rx_data = USART_ReceiveData(HTCFG_UART_PORT);									
            RXRD_BUF[RXRD_Index++] = rx_data;

            if(RXRD_BUF[RXRD_Index - 1] == '\n')
            {
                xTaskResumeFromISR(UARTTask_Handler);

                memset(RXRD_BUF, '\0' ,100);
                RXRD_Index = 0;
            }
     }
}
```


# 參考資料

https://neyzoter.cn/2018/03/30/FreeRTOS-Note2-Transplant/

