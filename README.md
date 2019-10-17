# IP Cam Client

## Setting up the cross compile environment

### MT7688

1. Download the MT7688 toolchain. (Official download page : <https://docs.labs.mediatek.com/resource/linkit-smart-7688/en/downloads>)

    ```bash
    $ cd ~

    $ wget -O "OpenWrt-Toolchain-ramips-mt7688_gcc-4.8-linaro_uClibc-0.9.33.2.Linux-x86_64.tar.bz2" https://labs.mediatek.com/en/download/2whWQCuB

    $ tar jxvf "OpenWrt-Toolchain-ramips-mt7688_gcc-4.8-linaro_uClibc-0.9.33.2.Linux-x86_64.tar.bz2"
    ```

2. Creat a soft lint to the toolchain folder

    ```bash
    $ ln OpenWrt-SDK-ramips-mt7688_gcc-4.8-linaro_uClibc-0.9.33.2.Linux-x86_64 mt7688_sdk
    ```
3. Change the directory into the target directory

    ```bash
    cd mt7688_sdk/staging_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2
    ```
4. Copy the path.

    ```bash
    $ pwd
    /home/clx/OpenWrt-SDK-ramips-mt7688_gcc-4.8-linaro_uClibc-0.9.33.2.Linux-x86_64/staging_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2
    # ^^^ Copy the path
    ```
5. Change the directory into `staging_sir`

    ```bash
    $ cd ~/mt7688_sdk/staging_dir/
    ```
6. Use `nano` to create a CMake configuration file

    ```bash
    nano mt7688.cmake
    ```
7. Then insert the following

    ```bash
    SET(CMAKE_SYSTEM_NAME Linux)
    SET(CMAKE_SYSTEM_VERSION 1)
    SET(CMAKE_C_COMPILER $ENV{HOME}/mt7688_sdk/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-gcc)
    SET(CMAKE_CXX_COMPILER $ENV{HOME}/mt7688_sdk/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-g++)
    SET(CMAKE_FIND_ROOT_PATH $ENV{HOME}/mt7688_sdk/staging_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2)
    SET(CMAKE_LIBRARY_PATH $ENV{HOME}/mt7688_sdk/staging_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/usr/lib)
    SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
    ```

### Raspberry Pi
(TAB)

## Compile with CMake

### MT7688

1. Use `git` to clone this repository

    ```bash
    $ git clone https://gitlab.com/niu.voiplab/project-ip-cam/ip-cam-client.git
    ```

2. The `git` command will ask you to enter your GitLab username and password.

3. Change the directory into build folder

    ```bash
    $ cd ip-cam-client/builds
    ```

4. Create a build folder for MT7688

    ```bash
    $ mkdir mt7688
    ```

5. Use `cmake` to generate the `Makefile`
    
    ```bash
    $ cmake -D CMAKE_TOOLCHAIN_FILE=~/mt7688_sdk/staging_dir/mt7688.cmake ../../
    ```

6. Compile the program with `make`

    ```bash
    $ make
    ```

### Raspberry Pi

(TAB)
<!---
  ```bash
  $ cmake -D CMAKE_TOOLCHAIN_FILE=/home/clx/raspberrypi/pi.cmake ../../
  ```
->