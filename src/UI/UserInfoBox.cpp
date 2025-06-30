/*
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE
 *
 * UserInfo クラス
 * 各ユーザーの詳細を表示する。
 */

#include "UserInfoBox.h"
#include "../Mastodon/MastodonAPI.h"
#include "../Mastodon/TootData.h"
#include "../Network/Network.h"
#include "ImageLabel.h"
#include "MainWindow.h"
#include "TextLabel.h"
#include "TootContent.h"
#include <QNetworkReply>
#include <QtWidgets>

UserInfoBox::UserInfoBox(const TootAccountData &user_data, MainWindow *rw,
                         Qt::WindowFlags f)
    : QMainWindow(rw, f), root_window(rw), user(user_data), mstdn(nullptr),
      menu(nullptr) {
  // ウィンドウ準備
  setWindowTitle(tr("User Info"));
  setAttribute(Qt::WA_DeleteOnClose);
  QScrollArea *main_scroll_area = new QScrollArea;
  main_scroll_area->setFrameShape(QFrame::NoFrame); // 枠線をなくす
  main_scroll_area->setVerticalScrollBarPolicy(
      Qt::ScrollBarAlwaysOn);                 // 常に表示
  main_scroll_area->setWidgetResizable(true); // 先にこれを設定する。
  QWidget *center = new QWidget;
  QPalette palette = main_scroll_area->palette();
  palette.setColor(QPalette::Window, Qt::black); // 背景を黒く
  palette.setColor(QPalette::WindowText, Qt::white);
  main_scroll_area->setPalette(palette);
  main_scroll_area->setAutoFillBackground(true);

  main_layout = new QVBoxLayout;
  center->setLayout(main_layout);

  main_scroll_area->setWidget(center);
  setCentralWidget(main_scroll_area);

  infobox_layout = new QVBoxLayout;
  relationinfo_layout = new QVBoxLayout;
  main_layout->addLayout(infobox_layout);
  main_layout->addLayout(relationinfo_layout);
  main_layout->addStretch();

  // API情報受け渡し
  if (root_window != nullptr) {
    mstdn = root_window->copyMastodonAPI();
  } else {
    mstdn = new MastodonAPI;
  }

  // 基本情報を表示
  createNameBox();
  createInfoBox();
}

UserInfoBox::~UserInfoBox() { delete mstdn; }

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
  if (!icon->setPixmapByName(user.getAvatar())) {
    // アイコンのキャッシュがない
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
  infobox_layout->addLayout(icon_box);
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:ユーザの主な情報を表示する。
 */
void UserInfoBox::createInfoBox() {
  // ユーザー説明
  QLabel *user_description = new QLabel(user.getDescription());
  user_description->setStyleSheet("color:white;");
  user_description->setWordWrap(true);
  user_description->setTextFormat(Qt::RichText);
  infobox_layout->addWidget(user_description);
  QLabel *following_info = new QLabel(
      tr("Following") + ": " + QString::number(user.getFollowingCount()));
  following_info->setStyleSheet("color:white;");
  following_info->setWordWrap(true);
  infobox_layout->addWidget(following_info);
  QLabel *followers_info = new QLabel(
      tr("Followers") + ": " + QString::number(user.getFollowersCount()));
  followers_info->setStyleSheet("color:white;");
  followers_info->setWordWrap(true);
  infobox_layout->addWidget(followers_info);
  if (user.getId() != mstdn->getUserId()) {
    connect(mstdn->requestUserRelationship(user.getId()),
            &QNetworkReply::finished, this, &UserInfoBox::showRelationship);
  }
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:ウィンドウを表示する時呼ばれる。
 */
void UserInfoBox::show() {
  QMainWindow::show();
  connect(mstdn->requestUserStatuses(user.getId()), &QNetworkReply::finished,
          this, &UserInfoBox::showTimeLine);
  resize(root_window->width(), root_window->height() / 2);
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:特定ユーザーと使用しているアカウントの関係を表示する。requestUserRelationshipが終わったら呼ばれる。
 */
void UserInfoBox::showRelationship() {
  QNetworkReply *rep = qobject_cast<QNetworkReply *>(sender());
  if (rep->error() == QNetworkReply::NoError) {
    relation = TootRelationshipData(
        QJsonDocument::fromJson(rep->readAll()).array()[0].toObject());
    if (relation.isfollowing()) {
      relationinfo_layout->addWidget(new QLabel(tr("Following")));
    }
    if (relation.isfollowed()) {
      relationinfo_layout->addWidget(new QLabel(tr("Followed you")));
    }
    if (relation.isblocking()) {
      relationinfo_layout->addWidget(new QLabel(tr("Blocking")));
    }
  }
  rep->deleteLater();
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:ユーザーの投稿を表示する。requestUserStatusesが終わったら呼ばれる。
 */
void UserInfoBox::showTimeLine() {
  QNetworkReply *rep = qobject_cast<QNetworkReply *>(sender());
  if (rep->error() == QNetworkReply::NoError) {
    // こうしてるのは将来ストリームに対応するかもしれないから
    // 水平線作成
    QFrame *horizontal_line = new QFrame;
    horizontal_line->setFrameShape(QFrame::HLine);
    horizontal_line->setFrameShadow(QFrame::Sunken);
    main_layout->addWidget(horizontal_line);

    QBoxLayout *timeline_layout =
        new QBoxLayout(QBoxLayout::BottomToTop); // 下から上
    // 一個づつ表示する
    QJsonArray toots = QJsonDocument::fromJson(rep->readAll()).array();
    for (int i = toots.size() - 1; i >= 0; i--) {
      QJsonObject obj = toots[i].toObject();
      TootData *tdata = new TootData(obj);
      if (!tdata->isEmpty()) {
        TootContent *content =
            new TootContent(tdata, TootContent::Mode::Normal, root_window);
        connect(content, &TootContent::action, root_window,
                &MainWindow::contentAction);
        timeline_layout->addWidget(content);
      }
    }
    main_layout->addLayout(timeline_layout);
  }
  rep->deleteLater();
}

/*
 * 引数:event
 * 戻値:なし
 * 概要:マウス操作がされたときに呼び出される。
 */
void UserInfoBox::mousePressEvent(QMouseEvent *event) {
  switch (event->button()) {
  case Qt::RightButton:
    if (!menu) {
      menu = new QMenu(user.getDisplayName(),
                       this); // Title表示されない...
      createMenu();
    }
    menu->popup(event->globalPosition().toPoint());
    break;
  default:
    event->ignore();
    return;
  }
  event->accept();
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:ポップアップメニュー作成。
 */
void UserInfoBox::createMenu() {
  // relation変数がセットされてないときはデフォルトとして扱う。
  // TODO: サブユニットとして分離
  if (mstdn->getUserId() != user.getId()) {
    if (relation.isfollowing()) {
      menu->addAction(tr("Unfollow(&U)"), this, [this] {
        connect(mstdn->requestUnfollow(user.getId()), &QNetworkReply::finished,
                this, [this] {
                  QNetworkReply *rep = qobject_cast<QNetworkReply *>(sender());
                  if (rep->error() != QNetworkReply::NoError) {
                    QMessageBox::critical(this, APP_NAME,
                                          tr("Failed to unfollow"));
                    rep->deleteLater();
                  } else {
                    resetRelationInfo();
                  }
                });
      });
    } else if (relation.isblocking()) {
      menu->addAction(tr("Unblock(&U)"), this, [this] {
        connect(mstdn->requestUnblock(user.getId()), &QNetworkReply::finished,
                this, [this] {
                  QNetworkReply *rep = qobject_cast<QNetworkReply *>(sender());
                  if (rep->error() != QNetworkReply::NoError) {
                    QMessageBox::critical(this, APP_NAME,
                                          tr("Failed to unblock"));
                    rep->deleteLater();
                  } else {
                    resetRelationInfo();
                  }
                });
      });
    } else {
      menu->addAction(tr("Follow(&F)"), this, [this] {
        connect(mstdn->requestFollow(user.getId()), &QNetworkReply::finished,
                this, [this] {
                  QNetworkReply *rep = qobject_cast<QNetworkReply *>(sender());
                  if (rep->error() != QNetworkReply::NoError) {
                    QMessageBox::critical(this, APP_NAME,
                                          tr("Failed to follow"));
                    rep->deleteLater();
                  } else {
                    resetRelationInfo();
                  }
                });
      });
    }
    if (!relation.isblocking()) {
      menu->addAction(tr("Block(&B)"), this, [this] {
        if (QMessageBox::question(
                this, APP_NAME, tr("Do you want to really block this account?"),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No) == QMessageBox::Yes) {
          connect(
              mstdn->requestBlock(user.getId()), &QNetworkReply::finished, this,
              [this] {
                QNetworkReply *rep = qobject_cast<QNetworkReply *>(sender());
                if (rep->error() != QNetworkReply::NoError) {
                  QMessageBox::critical(this, APP_NAME, tr("Failed to block"));
                  rep->deleteLater();
                } else {
                  resetRelationInfo();
                }
              });
        }
      });
    }
  }
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:フォローなどしたあとに表示などの再更新をするため
 */
void UserInfoBox::resetRelationInfo() {
  QLayoutItem *label;
  while ((label = relationinfo_layout->takeAt(0)) != nullptr) {
    delete label->widget();
  }

  QNetworkReply *rep = qobject_cast<QNetworkReply *>(sender());
  if (rep->error() == QNetworkReply::NoError) {
    relation =
        TootRelationshipData(QJsonDocument::fromJson(rep->readAll())
                                 .object()); // ここがshowRelationshipと違う
    if (relation.isfollowing()) {
      relationinfo_layout->addWidget(new QLabel(tr("Following")));
    }
    if (relation.isfollowed()) {
      relationinfo_layout->addWidget(new QLabel(tr("Followed you")));
    }
    if (relation.isblocking()) {
      relationinfo_layout->addWidget(new QLabel(tr("Blocking")));
    }
  }

  rep->deleteLater();
  menu->deleteLater();
  menu = nullptr;
}
