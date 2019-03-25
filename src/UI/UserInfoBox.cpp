/*
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE
 *
 * UserInfo クラス
 * 各ユーザーの詳細を表示する。
 */

/*#include "../Mastodon/MastodonAPI.h"*/
#include "../Mastodon/TootData.h"
#include "../Network/Network.h"
#include "ImageLabel.h"
#include "TextLabel.h"
#include "UserInfoBox.h"
#include <QNetworkReply>
#include <QtWidgets>

UserInfoBox::UserInfoBox(const TootAccountData &user_data /*, Mastodon &mstdn*/,
                         QWidget *root_window, Qt::WindowFlags f)
    : QWidget(root_window, f), root_widget(root_window), user(user_data)
/*,mstdn(mstdn)*/ {
  setAttribute(Qt::WA_DeleteOnClose);
  //ウィンドウ準備
  main_layout = new QVBoxLayout; //メインレイアウト
  QPalette Palette = palette();
  Palette.setColor(QPalette::Window, Qt::black); //背景を黒く
  Palette.setColor(QPalette::Text, Qt::white);
  setAutoFillBackground(true);
  setPalette(Palette);
  //基本情報を表示
  createNameBox();
  createInfoBox();
  setLayout(main_layout);
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:名前関係を表示する。
 */
void UserInfoBox::createNameBox() {
  Network net;
  QHBoxLayout *icon_box = new QHBoxLayout;
  QVBoxLayout *name_box = new QVBoxLayout;
  ImageLabel *icon = new ImageLabel(40, 40);
  icon->setFixedSize(40, 40);
  if (!icon->setPixmapByName(user.getAvatar())) { //アイコンのキャッシュがない
    connect(net.get(user.getAvatar()), &QNetworkReply::finished, icon,
            &ImageLabel::setPixmapByNetwork);
  }
  icon_box->addWidget(icon);
  QLabel *display_name = new QLabel(user.getDisplayName());
  display_name->setStyleSheet("font-weight:bold;color:white;");
  display_name->setWordWrap(true);
  name_box->addWidget(display_name);
  QLabel *user_name = new QLabel('@' + user.getAcct());
  user_name->setStyleSheet("color:white;");
  user_name->setWordWrap(true);
  name_box->addWidget(user_name);
  icon_box->addLayout(name_box);
  main_layout->addLayout(icon_box);
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:ユーザの主な情報を表示する。
 */
void UserInfoBox::createInfoBox() {
  //ユーザー説明
  QLabel *user_description = new QLabel(user.getDescription());
  user_description->setStyleSheet("color:white;");
  user_description->setWordWrap(true);
  user_description->setTextFormat(Qt::RichText);
  main_layout->addWidget(user_description);
  QLabel *following_info =
      new QLabel(tr("フォロー中:") + QString::number(user.getFollowingCount()));
  following_info->setStyleSheet("color:white;");
  following_info->setWordWrap(true);
  main_layout->addWidget(following_info);
  QLabel *followers_info =
      new QLabel(tr("フォロワー:") + QString::number(user.getFollowersCount()));
  followers_info->setStyleSheet("color:white;");
  followers_info->setWordWrap(true);
  main_layout->addWidget(followers_info);
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:ウィンドウを表示する時呼ばれる。
 */
void UserInfoBox::show() { QWidget::show(); }
