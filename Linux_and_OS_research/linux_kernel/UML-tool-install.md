### 建構 User-Mode Linux 和搭配的檔案系統

```note
若沒有特別說明，本文在開發端 (即編輯修改原始程式碼、編譯和準備相關工具等等動作) 的工作目錄皆位於上方解壓縮後的核心原始程式碼目錄。

為避免目錄切換導致的錯誤，可用環境變數保存: (此處 WS 指 "workspace"，命名沒有特別意思)

export WS=`pwd`
```

</br>

#### 基本設定：

<font color = red>皆為在 Ubuntu 中安裝</font>。

1. 安裝套件

```bash
$ sudo apt install build-essential libncurses-dev flex bison
$ sudo apt install xz-utils wget ca-certificates bc
```

</br>

2. 取得 Linux 核心原始程式碼（v5.12.0 為例）

```bash
$ wget https://cdn.kernel.org/pub/linux/kernel/v5.x/linux-5.12.tar.xz
$ tar xvf linux-5.12.tar.xz
```

3.  Linux 核心原始程式碼

```bash
$ cd linux-5.12
```

</br>

4. 設定核心組態

```bash
$ make mrproper
$ make defconfig ARCH=um SUBARCH=x86_64
$ make linux ARCH=um SUBARCH=x86_64
```

編譯完成後會出現 linux 的執行檔。由於目前只是一班的執行檔所以我們需要準備一個 rootfs（root file system）。後續會說明 [rootfs](#root-file-system)。

</br>

5. 安裝 rootfs 套件

```bash
$ sudo apt install fakeroot
```

</br>

6. 套件管理系統 APK 建立 rootfs

```bash
$ export REPO=http://dl-cdn.alpinelinux.org/alpine/v3.13/main
$ mkdir -p rootfs
$ curl $REPO/x86_64/APKINDEX.tar.gz | tar -xz -C /tmp/
$ export APK_TOOL=`grep -A1 apk-tools-static /tmp/APKINDEX | cut -c3- | xargs printf "%s-%s.apk"`
$ curl $REPO/x86_64/$APK_TOOL | fakeroot tar -xz -C rootfs
$ fakeroot rootfs/sbin/apk.static \
    --repository $REPO --update-cache \
    --allow-untrusted \
    --root $PWD/rootfs --initdb add alpine-base
$ echo $REPO > rootfs/etc/apk/repositories
$ echo "LABEL=ALPINE_ROOT / auto defaults 1 1" >> rootfs/etc/fstab
```

逐行輸入即可。

</br>

7. 建立方便測試的 .sh 腳本，命名為 UML.sh

```bash
#!/bin/sh
./linux umid=uml0 \
        root=/dev/root rootfstype=hostfs hostfs=./rootfs \
        rw mem=64M init=/bin/sh quiet

stty sane ; echo
```

</br>

8. 啟動 UML

```bash
$ chmod +x UML.sh
$ ./UML.sh
```

<br>

稍早準備的檔案系統，已有 busybox，不過相關的 symbolic link 還未設定，我們需要執行以下: (只要做一次)

```bash
UML $ /bin/busybox --install
```

</br>


由於目前沒有特別去撰寫 init scripts，像是 procfs 沒預先掛載，需要手動執行以下命令：

```bash
UML $ mount -t proc none /proc
```

</br>

我們先前在編譯 Linux 核心程式碼時指定 ARCH=um，這對於 UML 環境的影響是什麼呢？執行下列命令：

```bash
UML $ cat /proc/cpuinfo
```

Output

```bash
processor	: 0
vendor_id	: User Mode Linux
model name	: UML
mode		: skas
host		: Linux node1 4.15.0-72-generic #81-Ubuntu SMP Tue Nov 26 12:20:02 UTC 2019 x86_64
bogomips	: 7722.59
```

</br>

目前工具的安裝就到一段落，接下來可以做一些客製化的安裝（可選）

tini：

```bash
$ wget -O rootfs/sbin/tini https://github.com/krallin/tini/releases/download/v0.19.0/tini-static
$ chmod +x rootfs/sbin/tini
```

建立 rootfs/init.sh 檔案，記得要變更檔案權限：

```sh
#!/bin/sh

mount -t proc proc /proc
mount -t sysfs sys /sys

#更改 UML 環境中命令提示訊息字樣與顏色
export PS1='UML:\w\ $ '
export PS1='\[\033[01;32mUML:\w\033[00m \$ '

exec /sbin/tini /bin/sh +m
```

```bash
$ chmod +x rootfs/init.sh
```

修改 UML.sh 中的 init 選項，改為指定 init.sh：

```sh
#!/bin/sh
./linux umid=uml0 \
        root=/dev/root rootfstype=hostfs hostfs=./rootfs \
        rw mem=64M init=/init.sh quiet
stty sane ; echo
```

---

</br>

#### 準備核心模組：

1. 編譯核心模組

```bash
$ make ARCH=um SUBARCH=x86_64 modules
```

</br>

2. 安裝核心模組到 rootfs 所在的目錄

```bash
$ make modules_install MODLIB=`pwd`/rootfs/lib/modules/VER ARCH=um
```

</br>

測試：

```bash
UML $ cd /lib/modules
UML $ ls # 這裡會發現有剛剛掛載的資料夾 VER
UML $ depmod -ae VER
UML $ modprobe isofs
UML $ lsmod
```

Output

```bash
Module                  Size  Used by    Tainted: G  
isofs                  25330  0 
```

</br>

### GDB 工具（GNU Degbugr）

除錯器（debugger），可以在一個精準受控的環境下執行另一個程式。例如：單步執行程式，跟蹤程式，查看變數內容，記憶體地址，以及程式中每一條指令指行完畢後CPU暫存器的變化情況，檢視程式呼叫堆疊等等。

GDB，全名為 gnu debugger，是在 GNU 軟體系統中的標準除錯器，介面為互動式的 shell，許多類 Unix，如 :FreeBSD，Linux 等作業系統中都能夠使用，支援許多語言，包括 C, C++ 等。

</br>

1. 建構 GDB script

```bash
$ echo "CONFIG_GDB_SCRIPTS=y" > .config-fragment
$ ARCH=um scripts/kconfig/merge_config.sh .config .config-fragment
$ make ARCH=um scripts_gdb
```

</br>

2. 用下行命令來啟動 GDB，指定載入 Linux 核心提供的 GDB script

```bash
$ gdb -ex "add-auto-load-safe-path scripts/gdb/vmlinux-gdb.py" \
      -ex "file vmlinux" \
      -ex "lx-version" -q
```

</br>

3. 準備 gdbinit 檔案（檔案名稱直接 gdbinit 即可）

```sh
python gdb.COMPLETE_EXPRESSION = gdb.COMPLETE_SYMBOL
add-auto-load-safe-path scripts/gdb/vmlinux-gdb.py
file vmlinux
lx-version
set args umid=uml0 root=/dev/root rootfstype=hostfs rootflags=FULLPATH/rootfs rw mem=64M init=/init.sh quiet
handle SIGSEGV nostop noprint
handle SIGUSR1 nopass stop print
```

</br>

執行：

```bash
$ sed -i 's|FULLPATH|'"$PWD"'|' gdbinit 
```

```bash
$ gdb -q -x gdbinit
```

Output

```bash
(gdb) run
```

</br>

一些 GDB CMD：

```bash
(gdb) lx-mounts 
(gdb) lx-cmdline
(gdb) lx-ps
(gdb) lx-dmesg
(gdb) lx-lsmod
```

</br>

以上就是可以幫助我們工具的安裝。

---

### 小結

UML（User Mode Linux）是學習 Linux Kernel 的最佳起點之一，因為它讓你不用燒機、不用模擬器、不用 root 權限，就能直接在 Linux 上執行與除錯你所編譯的 Kernel。

</br>

**核心啟動流程（Boot process）**

* 觀察 `start_kernel()`、`init/main.c` 的流程
* 分析系統初始化：記憶體子系統、排程器、裝置初始化

</br>

**System Call 系統呼叫**

* UML 會透過 user-space syscall 模擬 Linux 真實系統呼叫
* 可觀察 `sys_read()`, `sys_write()`, `sys_execve()` 的進入與返回

</br>

**行程管理（Process / Task Scheduling）**

* trace `schedule()`、`wake_up()`、`do_fork()` 等
* 分析 Task struct（`struct task_struct`）內容

</br>

**虛擬記憶體管理**

* 分析 kernel 如何建立 page table
* 熟悉 `kmalloc()`、`vmalloc()`、`brk()`、`mmap()` 的機制

</br>

**核心模組（Kernel Modules）**

* 在 UML 中可以載入 `.ko` 模組，如：

  ```bash
  insmod hello.ko
  ```
* 可用 GDB trace `init_module()`、模組初始化流程

</br>

**檔案系統 VFS 與 syscall interaction**

* UML 支援擬態的磁碟 image（ubd），你可以在其中執行 `open()`, `read()`, `write()` 並 trace kernel 的 VFS 層

</br>

**Signal / IPC / clone / exec 系統行為**

* UML 可以真實模擬行程之間的 IPC、signal 傳遞、execve 換程式

</br>

那工具備齊了，讓我們開始吧 ～

</br>