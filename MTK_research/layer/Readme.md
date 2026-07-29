# Making your OWN meta-layer

新增 Layer
```bash
bitbake-layers create-layer ../Genio-LCM/meta-lcm
```

新增 Layer 進入 Yocto
```bash
bitbake-layers add-layer ../Genio-LCM/meta-lcm
```

成功
```bash
yocto@yocto:/media/yocto/usblab/iot-yocto/build$ bitbake-layers create-layer ../Genio-LCM/meta-lcm
NOTE: Starting bitbake server...
Add your new layer with 'bitbake-layers add-layer ../Genio-LCM/meta-lcm'
yocto@yocto:/media/yocto/usblab/iot-yocto/build$ bitbake-layers add-layer ../Genio-LCM/meta-lcm
NOTE: Starting bitbake server...
yocto@yocto:/media/yocto/usblab/iot-yocto/build$ 
```

</br>

---

</br>

撰寫 Recipes 可以看：https://docs.yoctoproject.org/scarthgap/contributor-guide/recipe-style-guide.html

</br>

# 紀錄

目前把自製的 meta-layer 跟預設 src 分開，Filepath 可能會有抓不到的問題，所以 recipes 要標明，也有可能是因為版本問題。

```
SRC_URI += "file://hello.c file://Makefile"
```

</br>

加入 image :

```
IMAGE_INSTALL:append = "hello"
```

