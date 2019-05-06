/*
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE
 *
 * UserInfo クラス
 * 各ユーザーの詳細を表示する。
 */

#pragma once

/*#include "../Mastodon/MastodonAPI.h"*/
#include "../Mastodon/TootData.h"
#include <QWidget>

class MainWindow;
class QVBoxLayout;
class QMenu;

class UserInfoBox : public QWidget {
  Q_OBJECT
public:
  explicit UserInfoBox(const TootAccountData &user_data, /*Mastodon &mstdn,*/
                       QWidget *root_window,
                       Qt::WindowFlags f = Qt::WindowFlags());

  void show();

private:
  virtual void mousePressEvent(QMouseEvent *event) override;
  void createNameBox();
  void createInfoBox();
  void createMenu();
  QWidget *root_widget;
  TootAccountData user;
  /*MastodonAPI mstdn;*/
  QVBoxLayout *main_layout;
  QMenu *menu;
};
