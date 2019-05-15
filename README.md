# ShadowBlock
ShadowBlock is a modified Chromium browser that hides ads in a stealthy manner so it won't trigger any anti-adblocking response from websites, at rather low runtime overhead. Evaluations shows it achieves 100% evasion success rate on a small-scale anti-adblocker dataset and over 98% ad coverage among Alexa Top-1000 websites. At the same time, its average performance (in terms of both Page Load Time and SpeedIndex) is on par with stock Chromium running Adblock Plus. Kindly refer to our [WWW paper](https://www.shitong.me/pdfs/www19_shadowblock.pdf) for more details.

**Note we have only tested ShadowBlock on Ubuntu 18.04 (64-bit) and Mac OS X Mojave 10.14.4, but it should be compatible with other major Linux distros with only minor adjustments**

Feel free to contact [Shitong Zhu](mailto:shitong.zhu@email.ucr.edu) or open GitHub issues if you run into any problem running/building ShadowBlock. 

## Outline
1. Binaries (from [`release` tab](https://github.com/seclab-ucr/ShadowBlock/releases), currently versioned `ShadowBlock-v1.0`)
```
── ShadowBlock-Linux.tar.gz (modified Chromium binary for Linux)
── ShadowBlock-OSX.zip (modified Chromium binary for Mac OS X)
```
2. Source Code
```
├── LICENSE
├── README.md
├── patches
│   ├── patch.diff
│   └── v8_patch.diff
├── scripts
│   └── run_shadowblock.sh
└── src
    └── shadowblock_elemhide_server.cc
```

We provide both the binaries (**download them from GitHub's [`release` tab](https://github.com/seclab-ucr/ShadowBlock/releases)**) and source code (as Chromium patches) to allow for reproducibility and future extensions from research commiunity.

## Run
Binaries are located in the **`release`** folder. Please follow the steps below to run them.

1. Depending on the OS you use, unzip the folder `ShadowBlock-Linux` from `ShadowBlock-Linux.tar.gz`, or unzip the folder `ShadowBlock-OSX` from `ShadowBlock-OSX.zip`;
2. Run bash script `run_shadowblock.sh` in `ShadowBlock-Linux`/`ShadowBlock-OSX` folder to start the browser. Use command `bash run_shadowblock.sh` if you want to create a new user profile in the same directory, or `bash run_shadowblock.sh --use-default-profile` if you want to use the default Chromium user profile. Note that for the first run, you need to wait for several (~5) minutes for Chromium to automatically download the EasyList used by ShadowBlock;
4. Restart Chromium and browse normally.

## Build from scratch
Our source code is provided as Chromium patches. Follow these steps to apply and integrate them into stock Chromium.
1. Check out Chromium code from [this tutorial](https://www.chromium.org/developers/how-tos/get-the-code);
2. `git checkout` the main **`src/`** repo to commit `2f76aeef54efba5408522beb7b1dd14973c9338b` and apply our patch by `git apply /PATH/TO/patches/patch.diff`;
3. `git checkout` the **`src/v8/`** repo to commit `c670249b587112de339a8fa49e7cea13f8f559b4` and apply our patch by `git apply /PATH/TO/patches/v8_patch.diff`;
4. Set build parameters as below (by `gn args /PATH/TO/CHROMIUM/BUILD`):
```
# You may adjust this as needed, but 
# "use_jumbo_build = true" is required 
# as per our namespace usage
is_debug = false
symbol_level = 0
remove_webcore_debug_symbols = true
is_official_build = true
use_jumbo_build = true
enable_nacl = false
proprietary_codecs=true
ffmpeg_branding="Chrome"
```
5. `autoninja -C out/ShadowBlock-[Linux|OSX]` and you will get the resulting binary in `out/ShadowBlock-[Linux|OSX]` that is essentially as same as the one provided by us. Now if you choose to use existing `shadowblock_elemhide_server` executable, you can move to the **"Run"** section above to start using it.


If you prefer building `shadowblock_elemhide_server` from scratch as well, follow these steps further:
1. Clone the repo of [**libadblockplus**](https://github.com/adblockplus/libadblockplus);
2. `./ensure_dependencies.py` to install necessary dependencies;
3. `make get-prebuilt-v8` to download the v8 library `libv8_monolith.a`, and copy it to the root directory of `libadblockplus`;
4. `make` to build the library `libadblockplus.a`, and copy it to the root directory of `libadblockplus`;
5. Copy `shadowblock_elemhide_server.cc` to directory `libadblockplus`, and build executable `shadowblock_elemhide_server` by `clang++ -stdlib=libc++ -std=c++14 -Iinclude -Ithird_party/prebuilt-v8/include shadowblock_elemhide_server.cc -o shadowblock_elemhide_server -L./ -ladblockplus -lv8_monolith -lpthread -lcurl`.

## Reference
Check our WWW '19 paper for more architectural and technical details [[PDF](https://www.shitong.me/pdfs/www19_shadowblock.pdf)]:

**ShadowBlock: A Lightweight and Stealthy Adblocking Browser**  
Shitong Zhu, Umar Iqbal, Zhongjie Wang, Zhiyun Qian, Zubair Shafiq, and Weiteng Chen  
*World Wide Web Conference (WWW '19)*  
DOI: https://doi.org/10.1145/3308558.3313558
