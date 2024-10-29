```
./
 |____ FreeRTOS-Kernel
 |      git clone https://github.com/raspberrypi/FreeRTOS-Kernel.git
 |
 |____ pico-sdk
 |      git clone https://github.com/raspberrypi/pico-sdk
 |
 |____ pico2-niji-yoji
```

```
cd pico2-niji-yoji
mkdir build
cd build
```

```
export PICO_SDK_PATH=../../pico-sdk
cmake -DPICO_PLATFORM=rp2350 -DPICO_BOARD=pico2 -DFREERTOS_KERNEL_PATH=../../FreeRTOS-Kernel ..
make -j4
```

```
pico2-niji-yoji/build/freertos/niji_yoji_freertos.uf2
```
