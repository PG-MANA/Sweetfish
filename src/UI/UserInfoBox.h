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
#include <QMainWindow>

class MainWindow;
class MastodonAPI;
class QVBoxLayout;
class QMenu;

class UserInfoBox : public QMainWindow {
  Q_OBJECT
public:
  explicit UserInfoBox(const TootAccountData &user_data, MainWindow *rw,
                       Qt::WindowFlags f = Qt::WindowFlags());
  virtual ~UserInfoBox();

  void show();
public slots:
  void showTimeLine();
  void showRelationship();

private:
  virtual void mousePressEvent(QMouseEvent *event) override;
  void createNameBox();
  void createInfoBox();
  void createMenu();
  void resetRelationInfo();
  MainWindow *root_window;
  TootAccountData user;
  TootRelationshipData relation;
  MastodonAPI *mstdn;
  QVBoxLayout *main_layout;
  QVBoxLayout *infobox_layout;
  QVBoxLayout *relationinfo_layout;
  QMenu *menu;
};
