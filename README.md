# Sweetfish for Linux
(Format:UTF-8)  
LinuxデスクトップユーザのためのMastodon Client
## 概要
Linux/X11を対象としたMastodon Clientです。
Salmon(Twitter)のソースコードを持ってきました。
Qtを使用しているので環境さえ整えばMacやWindowsでも動作すると思います。
なおこのソフトウェアはQtオープンソース版のLGPLv3を選択しています。
Qtのライセンスは https://www.qt.io/licensing/ をご覧ください。
Qtを使用したC++の勉強用でもあるので、うまくかけてないかもしれません。
「コードの書き方がナンセンス」「何だこのソースは(驚愕)」と言うことがありましたら優しく教えてくださると嬉しいです。  
基本的にはサラッと書いたので、メモリ使用量・コード最適化がしっかりできてないと思います。
ただそれなりに軽く動くようにしているつもりです。

KDevelop 5で開発していますが、使用しなくてもビルドはできます。  
openSUSE Tumbleweed で開発してます。
## 開発状況
* master : 一応区切りがついたもの。安定版とは限らない。
* develop : 細かなことやとんでもないコードがあるかもしれないもの。実験要素満載。

## 動作環境
* Qt 5.9
* Phonon4qt5
* CPU:各デスクトップ環境の推奨する性能以上のもの、2コア以上はほしい。
* メモリ:起動時間が長いほどメモリ使用量は増加していく傾向にあります。たくさん積んでおきましょう。

ブラウザが快適に動作する程度ならこのソフトウェアも快適に動作するはずです。
## ライセンス
Copyright 2017 PG_MANA  
This software is Licensed under the Apache License Version 2.0  
See LICENSE  
注意:src/Resources/icon/以下のアイコンなどは「ロゴ」にあたり、派生成果物に含めることはできません。  

## コーディングスタイル
clang-formatかけています。

## バージョニング
バージョンは3つの数字で構成されており、左から順に  
メジャーバージョン: 大きな(仕様変更を含む)機能追加・修正を行った回数  
マイナーバージョン: 機能追加・修正を行った回数  
バグフィックスバージョン: 動作の不具合・コードの整理など見た目の変更を伴わない修正を行った回数  
を示します。

## ビルド
### 環境
* Qt 5.9 (Qt5::Widgets Qt5::Network) 開発用ヘッダファイルなど
* Phonon4qt5 開発用ヘッダファイルなど  (無い場合自動的にQtMultiMediaを選択しますが、一部環境では動画が再生できない可能性があります)
* c++17対応C++コンパイラ
* cmake  3.1以上

### 方法

```shell
cd Sweetfish
mkdir build
cd build
cmake ..
make
./sweetfish
```

## ビルド済みバイナリ
* RPM(x86_64) https://repo.taprix.org/pg_mana/linux/rpm/x86_64/Sweetfish/

https://repo.taprix.org/pg_mana/linux/rpm/ はx86_64用のrpmリポジトリになっています。
## 備考
* 動作確認は https://netstat.app で行っています。

## リンク
### SweetfishについてのWebページ
  https://soft.taprix.org/product/sweetfish.html
### Qtのドキュメントページ
  https://doc.qt.io/
### 開発者のマストドンアカウント
  https://netstat.app/@PG_MANA
### 開発者のTwitterアカウント
  [https://twitter.com/PG_MANA_](https://twitter.com/PG_MANA_)
### 開発者のWebページ
  https://pg-mana.net
