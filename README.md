# ShadowBlock
ShadowBlock is a modified Chromium browser that hides ads in a stealthy manner so it won't trigger any anti-adblocking response from websites, at rather low runtime overhead. Evaluations shows it achieves 100% evasion success rate on a small-scale anti-adblocker dataset and over 98% ad coverage among Alexa Top-1000 websites. At the same time, its average performance (in terms of both Page Load Time and SpeedIndex) is on par with stock Chromium running Adblock Plus. Kindly refer to our [WWW paper](#Reference) for more details.

**Note we have only tested ShadowBlock on Manjora Linux (KDE) 18.0.3 and Ubuntu 16.04, but it should be compatible with other distributions with only minor adjustments**

Feel free to contact [Shitong Zhu](mailto:shitong.zhu@email.ucr.edu) or open GitHub issues if you have any problem running/building ShadowBlock. 

## Outline
```
├── bin
│   ├── shadowblock_css_socket_server
│   └── ShadowBlock-Release.tar.gz
├── LICENSE
├── patches
│   ├── patch.diff
│   └── v8_patch.diff
├── README.md
└── src
    └── shadowblock_css_socket_server.cpp
```

We provide both the binaries (**download them from GitHub's "Releases" tab**) and source code (as Chromium patches) to allow for reproducibility and future extensions from research commiunity.

## Run
Binaries are located in the release folder. Please follow the steps below to run it.
1. Unzip the folder `ShadowBlock-Release` from `ShadowBlock-Release.tar.gz`;
2. Run the standalone socket server to supply with site-specific CSS HTML rules by `./shadowblock_css_socket_server`;
3. Open another terminal and start Chromium with a fresh user profile by `ShadowBlock-Release/chrome --no-sandbox --user-data-dir=/PATH/TO/NEW/PROFILE`. Note you need to wait for several minutes for Chromium to automatically download the EasyList that will be used by ShadowBlock;
4. Once the `Indexed Rules` folder is generated under the `Subreousrce Filter` folder in user profile (i.e. `/PATH/TO/NEW/PROFILE` in Step 4), it means the EasyList is already in place and you are ready to browse. Note this only needs to be done once as the downloaded ruleset persists in the profile;
5. Restart Chromium and browse normally. Make sure the `shadowblock_css_socket_server` is running in background.

## Build from scratch
Our source code is provided as Chromium patches. Follow these steps to apply and integrate them into stock Chromium.
1. Check out Chromium code from [this tutorial](https://www.chromium.org/developers/how-tos/get-the-code);
2. `git checkout` the main **`src/`** repo to commit `2f76aeef54efba5408522beb7b1dd14973c9338b` and apply our patch by `git apply /PATH/TO/patches/patch.diff`;
3. `git checkout` the **`v8/`** repo to commit `c670249b587112de339a8fa49e7cea13f8f559b4` and apply our patch by `git apply /PATH/TO/patches/v8_patch.diff`;
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
```
5. `autoninja -C out/ShadowBlock-Release` and you will get the binary in `out/ShadowBlock-Release` that is essentially as same as the one provided by us. Now if you choose to use existing `shadowblock_css_socket_server` executable, you can move to the **"Run"** section above to start using it.

If you prefer building `shadowblock_css_socket_server` from scratch as well, follow these steps further:
1. Clone the repo of [**libadblockplus**](https://github.com/adblockplus/libadblockplus);
2. `./ensure_dependencies.py` to install necessary dependencies;
3. `make get-prebuilt-v8` to download the v8 library `libv8_monolith.a`;
4. `make` to build the library `libadblockplus.a`;
5. Copy `shadowblock_css_socket_server.cpp` to directory `libadblockplus`, and build executable `shadowblock_css_socket_server` by `clang++ -stdlib=libc++ -std=c++14 -fuse-ld=lld -Iinclude -Ithird_party/prebuilt-v8/include shadowblock_css_socket_server.cpp -o shadowblock_css_socket_server -L./ -ladblockplus -lv8_monolith -lpthread -lcurl`.

## Reference
Check our WWW '19 paper for more architectural and technical details:

Shitong Zhu, Umar Iqbal, Zhongjie Wang, Zhiyun Qian, Zubair Shafiq, and Weiteng Chen. 2019. **ShadowBlock: A Lightweight and Stealthy Adblocking Browser**. In *Proceedings of the 2019 World Wide Web Conference (WWW’19), May 13–17, 2019, San Francisco, CA, USA*. ACM, New York, NY, USA, 11 pages. https://doi.org/10.1145/3308558.3313558
