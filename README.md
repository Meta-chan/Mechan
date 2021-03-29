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
2) Install [CMake](https://cmake.org)
3) Install [Ironic Library](https://github.com/Meta-chan/ironic_library).
4) Install [NeuroG](https://github.com/Meta-chan/NeuroG).
5) Install [tdlib](https://core.telegram.org/tdlib) to `td` subdirectory. Note that original instructions help to install `tdlib` to `td/tdlib`.
6) (optional) Download [Files](https://drive.google.com/drive/folders/145HLT_S2EaRzAvD0R1s121Do4rLAN5V9?usp=sharing) and place them in `data` subdirectory.
7) Modify `build.bat` to set paths to libraries.
8) Run `build.bat`

Note: I guarantee nothing. There might be problems with installation and usage of `tdlib`, it's interface continues changing. Use your programming skills to fix it.

### Configuration
You can edit `mechan_config` to configure your own Mechan instance. Options are:
 - `FIRST_NAME` \- first name, relevant for `mechan_telegram_interface`
 - `SECOND_NAME` \- second name, relevant for `mechan_telegram_interface`
 - `PHONE` \- phone number, relevant for `mechan_telegram_interface`
 - `DEVICE` \- device model, relevant for `mechan_telegram_interface`
 - `PASSWORD` \- password to `tdlib` database, relevant for `mechan_telegram_interface`
 - `IP` \- IPv4 address of `mechan`, relevant to `mechan` and all interfaces
 - `PORT` \- TCP port of `mechan`, relevant to `mechan` and all interfaces

### Contact
Contact original Mechan by [@halfdead_mechan](https://t.me/halfdead_mechan).

###### P.S. My code is not dirty, it is alternatively clean.
