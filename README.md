# Sweetfish for Linux
(Format:UTF-8)  
The Mastodon's client for Linux.

## Abstract
Sweetfish is the Mastodon(SNS)'s client for X11/Linux.  
It is the fork from Salmon(The Twitter client https://github.com/PG-MANA/Salmon ).  
Sweetfish uses Qt5 as a framework, so you may be able to build and run it on macOS or BSD or Windows.  
This software chooses LGPLv3 as Qt license. If you want to see the Qt's license, please see https://www.qt.io/licensing/ .  
I write this software to study C++, so the source code may be dirty and not good. I will be very glad if you tell me when you realize points to improvement.  

I'm developing with KDevelop 5.  

## Branches

 * master : the tested code and release commit
 * develop : the main branch of develop, may be crash or not working

## Requirements

 * Qt 5.9 or later
 * Phonon4qt5

## License

Copyright 2017 PG_MANA  
This software is Licensed under the Apache License Version 2.0  
See LICENSE  
Attention: "src/Resources/icon/icon-normal.png" is "Trademarks" and you can not include it your work.  

## Coding Style

I format the source codes by clang-format  

## Build
### Requirements

 * C++ header files of Qt 5.9 or later(Qt5::Widgets Qt5::Network) ( qt5-dev package)
 * C++ header files of Phonon4qt5 (if you don't have, CMake will select QMultiMedia, but you may not see video of the toot)
 * C++17-enabled compiler
 * Cmake 3.1 or later

### Build Process

```shell
cd Sweetfish
mkdir build
cd build
cmake ..
make
./sweetfish
```

## The binary package repository
* RPM(x86_64) https://repo.taprix.org/pg_mana/linux/rpm/

## Links
### The website of Sweetfish
  https://soft.taprix.org/product/sweetfish.html
### Qt's documentation
  https://doc.qt.io/
### The Mastodon account of PG_MANA
  https://don.taprix.org/@PG_MANA
### The Twitter account of PG_MANA
  [https://twitter.com/PG_MANA_](https://twitter.com/PG_MANA_)
### PG_MANA's website
  https://pg-mana.net
