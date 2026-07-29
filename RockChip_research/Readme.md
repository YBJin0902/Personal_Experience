# RK3588 開發筆記



</br>


# 專案開發步驟

### Step 1. 先取得官方 Source project。

作業系統要求：ubuntu 20.04

SDK 需要的環境：

```bash
sudo apt-get install git ssh make gcc libssl-dev liblz4-tool expect \
g++ patchelf chrpath gawk texinfo chrpath diffstat binfmt-support \
qemu-user-static live-build bison flex fakeroot cmake gcc-multilib \
g++-multilib unzip device-tree-compiler ncurses-dev libgucharmap-2-90-dev \
bzip2 expat gpgv2 cpp-aarch64-linux-gnu g++-aarch64-linux-gnu
```

SDK 中有 source 可以直接建置環境。

至官網下載專案包，當下載完成與環境建置完成後根據官方提供的說明文件編譯即可。

編譯成功後會出現 `rockdev` 資料夾，裡面會有所有需要的 `.img`。

---

</br>

### Step 2. 下載與安裝 Debian 專案包

https://gitlab.com/rk3588_linux/linux/debian/-/tree/develop-bullseye?ref_type=heads

須注意不要使用預設的分支，要使用 bullseye。

</br>

安裝 Debian 開發工具：

```bash
sudo dpkg -i debian/ubuntu-build-service/packages/*
```

```bash
sudo apt-get install -f
```

</br>

目前到這裡安裝完成 Debian 開發相關工具與專案。

---

</br>

### Step3. Debian 專案編譯設定與解說

Debian 在編譯時是使用 live-build 的工具，相關資料：https://live-team.pages.debian.net/live-manual/html/live-manual/index.en.html

</br>

專案整體結構如下：

```tree
debian
├── mk-base-debain.sh 				# 獲取 Debian 基礎軟體包的編譯
├── mk-image.sh 					# 打包 image
├── mk-rootfs-buster/bullseye.sh 	# 針對 Rockchip 相關硬體週邊的設定
├── mk-rootfs.sh 					# 編譯 rootfs
├── overlay 						# 針對 Rockchip 平台通用性
├── overlay-debug 					
├── overlay-firmware 				# 設備 firmware 存放處，如 NPU,DP 等
├── packages 						# 預編包
├── packages-patches 				# 預編包補丁
├── readme.md 
└── ubuntu-build-service			# 主要為官方提供的 Debian DTS。可依賴軟體資源和訂製安裝相關軟體
```

其實最主要是要利用官方寫好的腳本建好 rootfs，建好後要一起至外層 SDK 一起打包成 update.img。

</br>

[各腳本說明](#debian) 

</br>

可以閱讀跟目錄下的 Readme 進行編譯，使用的指令如下：

1. 建置 linaro Debian 系統

```
RELEASE=bullseye TARGET=desktop ARCH=arm64 ./mk-base-debian.sh
```

2. 建置含有 rockchip firmware 的 [OverlayFS](https://docs.kernel.org/filesystems/overlayfs.html) 

```
VERSION=debug ARCH=arm64 ./mk-rootfs-bullseye.sh
```

3. 建置 rootfs image

```
./mk-image.sh
```

</br>

### 小結

至此專案的基本介紹結束，接下來講解基本的操作與順序：

我們需要利用 SDK 所提供的底層與 Kernel 來對版子進行 boot 等操作，Debian live-build 只提供我們需要的 Root file system（rootfs）。

各項改動說明：
[1. 自己寫的 APP 預先打包至 rootfs]()
[2. Kernel 改動]()

</br>

在做完這些改動後，我們可以透過 SDK 提供的腳本（build.sh）打包整份 image 進行燒錄，這裡需要注意，我們需要將 live-build 最後產出的 rootfs.img 移動到 SDK 中的 rockdev 進行打包。

需要有 root 權限，CMD：

``` bash
$ sudo cp /your/path/to/linaro-rootfs.img /your/path/to/SDK/rockdev

$ sduo rm -rf /your/path/to/SDK/rockdev/rootfs.img

$ sudo mv linaro-rootfs.img rootfs.img
```

最後在執行腳本打包即可（`./build.sh updateimg`）

---

</br>

# 編譯腳本說明

</br>

## Debian

### 1. mk-base-debain.sh

進入 ubuntu-build-service 並根據我們指定的 release 與 arch 尋找需要的 service。

```bash
if [ "$RELEASE" == "stretch" ]; then
	RELEASE='stretch'
elif [ "$RELEASE" == "buster" ]; then
	RELEASE='buster'
elif [ "$RELEASE" == "bullseye" ]; then
	RELEASE='bullseye'
else
    echo -e "\033[36m please input the os type,stretch, buster or bullseye...... \033[0m"
fi

if [ "$ARCH" == "armhf" ]; then
	ARCH='armhf'
elif [ "$ARCH" == "arm64" ]; then
	ARCH='arm64'
else
    echo -e "\033[36m please input the os type,armhf or arm64...... \033[0m"
fi

if [ ! $TARGET ]; then
	TARGET='desktop'
fi

if [ -e linaro-$RELEASE-alip-*.tar.gz ]; then
	rm linaro-$RELEASE-alip-*.tar.gz
fi

cd ubuntu-build-service/$RELEASE-$TARGET-$ARCH

echo -e "\033[36m Staring Download...... \033[0m"
```

---

</br>

進入後會開始跑 Makefile 的 clean，注意這裡會將 config 與 chroot 整個刪除！

```bash
make clean
```

---

</br>

clean 完成後，跑 configure 進行 `lb config` 的設定，還會將整個 customization 資料夾的內容複製至 chroot 中

```bash
./configure
```

---

</br>

在 configure 完成後會開始跑整份軟體包的編譯，並且將結果壓縮至 debian 的跟目錄。

```bash
make

if [ -e linaro-$RELEASE-alip-*.tar.gz ]; then
	sudo chmod 0666 linaro-$RELEASE-alip-*.tar.gz
	mv linaro-$RELEASE-alip-*.tar.gz ../../
else
	echo -e "\e[31m Failed to run livebuild, please check your network connection. \e[0m"
fi
```

---

</br>

基本上這裡可以觀察出我們若是需要預先客製化 rootfs 就需要在 customization 新增內容，以下是 customization 的各項說明：

```tree

```

</br>

### 2. mk-rootfs-bullseye.sh

透過 live-build 的產物進一步產出可以配合 RK 硬體以及自製的內容。
   - 解壓 live-build 的壓縮包加入其餘內容，後產出 binary 的資料夾。

```

```

</br>

### 3. mk-image.sh

透過 binary 資料夾的內容產出 roofts.img。
   - 編譯完成後會產出 linaro-roofts.img。

```

```

</br>

</br>

# live-build Debian