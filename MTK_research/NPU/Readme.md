# NPU

在 MediaTek 中 NPU 的使用＆學習：[官網](https://neuropilot.mediatek.com/)

Main Board : MediaTek Genio 720 EVK

Notice : 無 NDA access 也可以使用，用的是 [NeuroPilot Public](https://neuropilot-developer.mediatek.com)

---

### 簡介

很多文獻跟參考資料大部都會稱 MTK 的 NPU 為 APU，但是在 Genio 720 開始後統稱 NPU 了，MTK 提供的開發工具為  [NeuroPilot](https://neuropilot-developer.mediatek.com/sphinx/neuropilot-8-public/html/l1_introduction/l2_sw_ecosystem/sw_ecosystem.html)，NeuroPilot 是一套開發者工具和API集合，幫助使用者在聯發科平台上有效開發 AI 應用，使用者可以極其有效地在邊緣設備上開發和部署 AI 應用。

</br>

---

</br>

開始前先搞清楚一些名詞

### NPU (Neural Processing Unit):

- NPU 作為聯發科人工智慧硬體加速器的正式名稱。
- NPU 指 MDLA 和 MVPU 兩個組成部分的總稱。

### MDLA (MediaTek Deep Learning Accelerator):

- MDLA 致力於高效率加速卷積神經網路（CNN）工作負載。

### MVPU (MediaTek Vision Processing Unit):

- MVPU 提供通用 DSP 功能，並加速複雜的成像和電腦視覺演算法，包括 AI 模型處理。

### APU(AI Processing Unit):

- APU 是上一代聯發科NPU硬體的術語。
- 對於當前產品和文檔，請使用 NPU 代替 APU。

### AIA (AI Accelerator):

- 向 MDLA 表示，提及「AI 加速器」（AIA）作為聯發科硬體 AI 加速功能的一般描述。


</br>

</br>


# G720

[G720 Documentation](https://neuropilot-developer.mediatek.com/sphinx/g720-public/html/)

</br>

### Supported Operations
- [TFLite Operations](https://neuropilot-developer.mediatek.com/sphinx/g720-public/html/l1_supported_operations/l2_supported_operations/l3_supported_ops/supported_operations_public.html)

- [CPU Guidelines](https://neuropilot-developer.mediatek.com/sphinx/g720-public/html/l1_supported_operations/l2_supported_operations/l3_cpu_guide/cpu_64bit_fp16_guidelines.html)

- [GPU Guidelines](https://neuropilot-developer.mediatek.com/sphinx/g720-public/html/l1_supported_operations/l2_supported_operations/l3_gpu_guide/gpu_guidelines.html)

- [MDLA 5.3 Guidelines](https://neuropilot-developer.mediatek.com/sphinx/g720-public/html/l1_supported_operations/l2_supported_operations/l3_mdla_guide/mdla_guidelines_5_3_public.html)

MTK 提供很多跑 Edge AI 的方法，可以針對自己的需求進行開發。

每一項裡面都有詳系列出支援的 Data Type 與模型。

</br>

# NeuroPilot

這邊介紹與筆記 NeuroPilot 的使用與開發。

</br>

首先先確定 [NeuroPilot Versions](https://neuropilot-developer.mediatek.com/sphinx/neuropilot-8-public/html/l1_introduction/l2_np_versions/neuropilot_versions.html)，我們是做使用與應用重點不是完整開發這顆 NPU，所以不用 NDA 也足夠。

</br>

Public 的功能：

| Feature | Type | Description |
| :------ | :--- | :---------- |
| 轉換工具 | 命令列工具 | 預先訓練和最佳化的 PyTorch 或 TensorFlow 模型轉換為 TensorFlow Lite 模型，並執行訓練後量化 | 
| TFLite Shim API | API | TFLite Shim API 是一個便利 API，它封裝了 TensorFlow Lite 的原生 C++ API |

</br>

## How to USE

整個開發流程如下：

```scss
PC 上訓練模型
(TensorFlow / PyTorch)
        ↓
NeuroPilot Converter
(模型轉換 + 量化)
        ↓
NeuroPilot Runtime
(NPU 執行)
        ↓
你的 App
(GTK / C++ / Python / GStreamer)
```

</br>

架構會像：

```scss
Yocto Linux
│
├─ NeuroPilot Runtime (userspace)
│
├─ Your App
│   ├─ GTK / Qt / CLI
│   ├─ Camera / USB / File
│   └─ 呼叫 NeuroPilot API
│
└─ MediaTek NPU Driver (kernel)
```

</br>

所以簡單來說在 G720 上使用 NPU 我們會將訓練好的模型量化過後 (或不用)，燒錄到板子上，接著透過官方提供的 API 去掉用模型進行推論。

### NeuroPilot 的三大核心元件

- NeuroPilot Converter
    - 模型轉換工具
    - 把你在 PC 訓練的模型轉成 NPU 可執行格式
    - 支援來源
        - TensorFlow
        - TensorFlow Lite
        - ONNX
    - 可以做量化

- NeuroPilot Runtime
    - 在 Genio 720 上執行 `.neu` 模型

- NeuroPilot SDK
    - 整合開發用
    - 提供 API、 Sample、 Library

</br>

## NeuroPilot Converter Tools

作業系統需求：
- 64-bit Linux.
- Python 3.5 to Python 3.11
- pip >= 8.1.0 (required by manylinux1)

Python 需求：
- argparse >= 1.2
- flatbuffers >= 1.12.0
- jsonschema
- lxml
- matplotlib >= 3.2
- natsort
- networkx
- numpy >= 1.13.3
- packaging
- plotly
- protobuf >= 3.5.1 and < 4.0
- pybind11 >= 2.2
- singleton-decorator
- tqdm >= 4.0
- xxhash

</br>

### Step 1. 安裝

下載：https://neuropilot-developer.mediatek.com/sphinx/neuropilot-8-public/html/l1_downloads/downloads_public.html#neuropilot-downloads

安裝 tools：

```shell
# For Python 3.5
$ pip3 install mtk_converter-<version>+release-cp35-cp35m-manylinux_2_5_x86_64.manylinux1_x86_64.whl

# For Python 3.6
$ pip3 install mtk_converter-<version>+release-cp36-cp36m-manylinux_2_5_x86_64.manylinux1_x86_64.whl

# For Python 3.7
$ pip3 install mtk_converter-<version>+release-cp37-cp37m-manylinux_2_5_x86_64.manylinux1_x86_64.whl

# For Python 3.8
$ pip3 install mtk_converter-<version>+release-cp38-cp38-manylinux_2_5_x86_64.manylinux1_x86_64.whl

# For Python 3.9
$ pip3 install mtk_converter-<version>+release-cp39-cp39-manylinux_2_5_x86_64.manylinux1_x86_64.whl

# For Python 3.10
$ pip3 install mtk_converter-<version>+release-cp310-cp310-manylinux_2_17_x86_64.manylinux2014_x86_64.whl

# For Python 3.11
$ pip3 install mtk_converter-<version>+release-cp311-cp311-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
```

</br>

驗證安裝

```shell
$ python3 -c 'import mtk_converter; print(mtk_converter.__version__)'
```

</br>

環境變數

```shell
$ export PATH=~/.local/bin:$PATH
```

</br>

---

</br>

### Step 2. 使用

</br>

---

</br>

</br>

# meta-tensorflow

若要在無 NDS 的情況下要使用 Genio 平台進行 AI 模型推論則會需要使用 Tensorflow-Lite 。

</br>

在 Genio Linux 平台的 Yocto 下對應的 recipes 則為 meta-tensorflow

個人編譯用的 host PC 在編譯時會遇到 Github 429 Error 的問題，可以參考我的[解決方法](#solution)

</br>

### Solution

會有缺失的 repo 為： `tensorflow` , `tensorflow-native` , `keras` , `keras-native`

</br>

Step 1. 下載 local-repo

Github 429 不是指檔案找不到或是連結有問題，就只是由於要下載的檔案過多 Github 會擋。

已將所需要的 [repo](/local-bazel-repos) 準備好，下載即可。

</br>

Step 2. 先 unpack 對應的 recipes

推薦順序：
1. tensorflow-native
2. tensorflow
3. keras
4. keras-native

語法：

```sh
bitbake -c unpack 'recipes-name'
```

</br>

在 unpack 後可以在以下對應的位置找到 git 資料夾：

- tensorflow-native : build/tmp/work/x86_64-linux/tensorflow-native/2.16.1/git

- tensorflow : build/tmp/work/armv8a-poky-linux/tensorflow/2.16.1/git

- keras : build/tmp/work/armv8a-poky-linux/keras/2.16.0/git

- keras-native : build/tmp/work/armv8a-poky-linux/keras-native/2.16.0/git

</br>

Step 3. 編輯 bazelrc

在 git 資料夾中都會有一個 bazelrc 的檔案，我們需要在裡面加上 `override` 去指定本定的資料夾。

須注意這種補 git 的方式在下次 `bitbake clean` 時就會消失所以要在自己補上。

</br>

tensorflow-native

```txt
build --override_repository=rules_jvm_external=/media/yocto/usblab/iot-yocto/local-bazel-repos/rules_jvm_external-4.3
build --override_repository=rules_foreign_cc=/media/yocto/usblab/iot-yocto/local-bazel-repos/rules_foreign_cc
build --override_repository=envoy_api=/media/yocto/usblab/iot-yocto/local-bazel-repos/envoy_api
build --override_repository=rules_cc=/media/yocto/usblab/iot-yocto/local-bazel-repos/rules_cc
build --override_repository=rules_java=/media/yocto/usblab/iot-yocto/local-bazel-repos/rules_java
```

---

</br>

tensorflow

```txt
build --override_repository=rules_jvm_external=/media/yocto/usblab/iot-yocto/local-bazel-repos/rules_jvm_external
build --override_repository=rules_foreign_cc=/media/yocto/usblab/iot-yocto/local-bazel-repos/rules_foreign_cc
build --override_repository=envoy_api=/media/yocto/usblab/iot-yocto/local-bazel-repos/envoy_api
build --override_repository=rules_cc=/media/yocto/usblab/iot-yocto/local-bazel-repos/rules_cc
build --override_repository=rules_java=/media/yocto/usblab/iot-yocto/local-bazel-repos/rules_java
```

---

</br>

keras

```txt
build --override_repository=rules_python=/media/yocto/usblab/iot-yocto/local-bazel-repos/rules_python
build --override_repository=rules_cc=/media/yocto/usblab/iot-yocto/local-bazel-repos/rules_cc_keras
build --override_repository=rules_java=/media/yocto/usblab/iot-yocto/local-bazel-repos/rules_java
build --override_repository=rules_proto=/media/yocto/usblab/iot-yocto/local-bazel-repos/rules_proto
```

---

</br>

keras-native

```txt
build --override_repository=rules_proto=/media/yocto/usblab/iot-yocto/local-bazel-repos/rules_proto
build --override_repository=rules_cc=/media/yocto/usblab/iot-yocto/local-bazel-repos/rules_cc_keras
build --override_repository=rules_python=/media/yocto/usblab/iot-yocto/local-bazel-repos/rules_python
build --override_repository=rules_java=/media/yocto/usblab/iot-yocto/local-bazel-repos/rules_java
```

---

</br>

Step 4. Bitbake recipes

可以先針對對應的 recipes bitbake，或是直接 `bitbake tensorflow`

</br>