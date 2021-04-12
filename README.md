# Mechan is glad to greet you, desu!
<img src="data/mechan.png" alt="Mechan" width="60%" />
     
Mechan is a telegram bot. Unlike most of bots, she doesn't use bot API \- instead she uses [tdlib](https://core.telegram.org/tdlib) that allows her to look like a human.

### Structure
 - `mechan` is the core of the project. It is a TCP server that processes requests from interfaces.
 - `mechan_console_interface` is console interface. It is a TCP client that interacts with user with command line and sends requests to `mechan`.
 - `mechan_telegram_interface` is Telegram interface. It is a TCP client that interacts with user with Telegram client and sends requests to `mechan`.
 - `mechan_dialog_downloader` is utility to download dialogs from [fanfics.me](https://fanfics.me) and [ficbook.net](https://ficbook.net) to `data/dialog.txt`.
 - `mechan_trainer` is utility to train validating neuronal network.

### Building
1) Clone repository.
2) Install [CMake](https://cmake.org).
3) Install [Ironic Library](https://github.com/Meta-chan/ironic_library).
4) Install [NeuroG](https://github.com/Meta-chan/NeuroG).
5) Install [tdlib](https://core.telegram.org/tdlib) to `td` subdirectory. Note that original instructions help to install `tdlib` to `td/tdlib`.
6) (optional) Download [Files](https://drive.google.com/drive/folders/145HLT_S2EaRzAvD0R1s121Do4rLAN5V9?usp=sharing) and place them in `data` subdirectory.
7) Modify `build.bat` to set paths to libraries.
8) Run `build.bat`

Note: I guarantee nothing. There might be problems with installation and usage of `tdlib`, it's interface continues changing. Use your programming skills to fix it.

### Configuration
You can edit `mechan_config` to configure your own Mechan instance. Options are:
 - `IP` \- IPv4 address of `mechan`, relevant to `mechan` and all interfaces
 - `PORT` \- TCP port of `mechan`, relevant to `mechan` and all interfaces
 - `TELEGRAM_FIRST_NAME` \- first name, relevant for `mechan_telegram_interface`
 - `TELEGRAM_SECOND_NAME` \- second name, relevant for `mechan_telegram_interface`
 - `TELEGRAM_PHONE` \- phone number, relevant for `mechan_telegram_interface`
 - `TELEGRAM_DEVICE` \- device model, relevant for `mechan_telegram_interface`
 - `TELEGRAM_PASSWORD` \- password to `tdlib` database, relevant for `mechan_telegram_interface`
 - `NEURO_WORD_NUMBER` \- Number of last words in sentence to pass to neural network (default is `7`), relevant to `mechan_trainer`
 - `NEURO_CHAR_NUMBER` \- Number of last characters in word to pass to neural network (default is `7`), relevant to `mechan_trainer`
 - `NEURO_BATCH_SIZE` \- Batch size (default is `1024`), relevant to `mechan_trainer`
 - `NEURO_INTERVAL` \- Interval between saves in seconds (default is `3600`), relevant to `mechan_trainer`
 - `NEURO_LAYER2` \- Size of second layer (default is `500`), relevant to `mechan_trainer`
 - `NEURO_LAYER3` \- Size of third layer if not zero (default is `0`), relevant to `mechan_trainer`
 - `NEURO_TRAIN_PART` \- Part of messages to be used in training (default is `0.7`), relevant to `mechan_trainer`
 - `NEURO_TEST_PART` \- Part of messages to be used in testing (default is `0.3`), relevant to `mechan_trainer`
 - `NEURO_COEFFICIENT` \- Training coefficient (default is `0.01`), relevant to `mechan_trainer`
 - `NEURO_DEVIANCE` \- Standard deviance for initialization of weights (default is `0.1`), relevant to `mechan_trainer`

### Contact
Contact original Mechan by [@halfdead_mechan](https://t.me/halfdead_mechan).

###### P.S. My code is not dirty, it is alternatively clean.
