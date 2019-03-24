/*
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE
 *
 * UserInfo クラス
 * 各ユーザーの詳細を表示する。
 */

#pragma once

#include "../Mastodon/Mastodon.h"
#include "../Mastodon/TootData.h"
#include <QWidget>

class MainWindow;
class QVBoxLayout;

class UserInfoBox : public QWidget {
  Q_OBJECT
public:
  explicit UserInfoBox(const TootAccountData &user_data, /*Mastodon &mstdn,*/
                       QWidget *root_window,
                       Qt::WindowFlags f = Qt::WindowFlags());

  void show();

private:
  void createNameBox();
  void createInfoBox();
  QWidget *root_widget;
  TootAccountData user;
  Mastodon mstdn;
  QVBoxLayout *main_layout;
};
