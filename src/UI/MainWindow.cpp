/*
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE
 */
#include "MainWindow.h"
#include "../Mastodon/MastodonAPI.h"
#include "../Mastodon/MediaUpload.h"
#include "../Mastodon/Streamer.h"
#include "../Setting/Setting.h"
#include "../Sweetfish.h"
#include "ImageLabel.h"
#include "ImageViewer.h"
#include "TextLabel.h"
#include "TootContent.h"
#include "TootInfo.h"
#include "VideoPlayer.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QtWidgets>

// MEMO:trは翻訳する可能性のあるものに使う。
// MEMO:mocのincludeはソースでQObjectの子クラスを作る場合に必要らしい
// MEMO:MainWindowを閉じたときにQApplication::closeAllWindowsを呼ばないのはMainWindowを複数出せるようにする計画が少なからずあるから。ただしメニューの終了は除く。
// MEMO:ウィジットはヒープ上に作らないと破棄時にバグる
MainWindow::MainWindow() : QMainWindow() {
  //ウィンドウ準備
  QWidget *center = new QWidget;         //センターウィジット
  main_layout = new QVBoxLayout(center); //メインレイアウト
  QPalette Palette = center->palette();
  Palette.setColor(QPalette::Window, Qt::black); //背景を黒く
  Palette.setColor(QPalette::WindowText, Qt::white);
  center->setAutoFillBackground(true);
  center->setPalette(Palette);
  setCentralWidget(center);

  //メニュー
  createMenus();
  // TimeLine
  createTimeLine();
  //トゥートボックス
  createTootBox();
  //通知
  tray_info = new QSystemTrayIcon(qApp->windowIcon(), this);

  // Stream APIのスレッド
  timeline_streamer = new Streamer; //親がいるとスレッドが動かせない。
  timeline_thread = new QThread(this);
  timeline_streamer->moveToThread(timeline_thread); //動作スレッドを移動する。
  timeline_thread->start();
  // connect祭り
  connect(timeline_streamer, &Streamer::newToot, this, &MainWindow::showToot);
  connect(timeline_streamer, &Streamer::deleteToot, this,
          &MainWindow::removeToot);
  connect(timeline_streamer, &Streamer::newNotification, this,
          &MainWindow::showNotification);
  connect(timeline_streamer, &Streamer::abort, this,
          &MainWindow::abortedTimeLine);
  connect(timeline_thread, &QThread::finished, timeline_streamer,
          &QObject::deleteLater); //意外と重要
}

MainWindow::~MainWindow() {
  timeline_thread->quit(); // timeline_streamerはdeleteLaterにより削除される。
  timeline_thread->wait(); //呼ばれても動かないようにする。
  delete mstdn;
  delete setting;
}

/*
 * 引数:setting_file_name(設定ファイルの名前)
 * 戻値:成功はtrue、失敗はfalse
 * 概要:渡された設定ファイルから設定を読み込み、基本設定を行う。初期化のあとに呼ぶ。
 */
bool MainWindow::init(const QString setting_file_name) {
  //設定読み込み
  setting = new Setting(setting_file_name);
  restoreGeometry(setting->getGeometry());
  mstdn = new MastodonAPI;

  if (setting->getAccessToken().isEmpty()) {
    try {
      (QMessageBox::question(this, APP_NAME,
                             tr("マストドンアプリの認証を行いますか。"),
                             QMessageBox::Yes | QMessageBox::No,
                             QMessageBox::Yes) == QMessageBox::Yes)
          ? authorizeMastodon() /*エラーはthrowされて下に行く*/
          : throw tr(
                "マストドンアプリの認証を行わなければこのソフトウェアは使用でき"
                "ません。");
    } catch (QString &error) {
      // delete mstdn;<=~MainWindowで呼ばれる。
      QMessageBox::critical(this, APP_NAME, error);
      return false;
    }
  } else {
    mstdn->setAccessToken(setting->getAccessToken());
    mstdn->setInstanceDomain(setting->getInstanceDomain());
    mstdn->setUserId(setting->getUserId());
    TootData::addOwnerUserId(setting->getUserId());
  }

  //タイトル
  setWindowTitle(setting->getUserName() + "@" + setting->getInstanceDomain() +
                 " - " APP_NAME);
  QMetaObject::invokeMethod(
      timeline_streamer, "setMastodonAPI", Qt::BlockingQueuedConnection,
      QGenericReturnArgument(nullptr),
      Q_ARG(
          const MastodonAPI *,
          mstdn)); //別スレッドでMastodonAPIクラスを作らないといろいろ怒られる。
  return true;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:メニューバーを作って設定する。ウィンドウ初期化用。
 */
void MainWindow::createMenus() {
  menuBar()->setNativeMenuBar(true);

  //設定
  QMenu *setting_menu = menuBar()->addMenu(tr("設定(&S)"));
  setting_menu->setToolTipsVisible(true);
  stream_status = setting_menu->addAction(
      style()->standardIcon(QStyle::SP_BrowserReload), tr("ストリーム接続(&S)"),
      this, &MainWindow::changeStatusStream); //アイコンの意図が違っていて微妙
  stream_status->setCheckable(true);
  stream_status->setChecked(true);
  stream_status->setToolTip(
      tr("チェックされるとストリームに接続し、外されると切断します。"));
  setting_menu
      ->addAction(style()->standardIcon(QStyle::SP_TitleBarCloseButton),
                  tr("終了(&E)"), qApp, &QApplication::closeAllWindows)
      ->setToolTip(
          tr("すべてのウィンドウを閉じ、アプリケーションを終了します。"));

  //表示
  QMenu *timeline_menu = menuBar()->addMenu(tr("表示(&V)"));
  timeline_menu->setToolTipsVisible(true);
  timeline_menu->addAction(tr("ホーム(&H)"), this, [] {});
  list_menu = timeline_menu->addMenu(tr("リスト(&L)"));

  //ウィンドウ
  QMenu *window_menu = menuBar()->addMenu(tr("ウィンドウ(&W)"));
  window_menu->setToolTipsVisible(true);
  QAction *always_top_action = window_menu->addAction(
      style()->standardIcon(
          QStyle::SP_ArrowUp /*QStyle::SP_TitleBarShadeButton*/),
      tr("常に最前面に表示(&A)"), this, &MainWindow::setAlwayTop);
  always_top_action->setToolTip(
      tr("常にこのウィンドウを手前に表示します。("
         "ウィンドウマネージャで設定できる場合はそちらで設定してください。)"));
  always_top_action->setCheckable(true);

  //ヘルプ
  QMenu *help_menu = menuBar()->addMenu(tr("ヘルプ(&H)"));
  help_menu->setToolTipsVisible(true);
  help_menu
      ->addAction(
          QIcon(":/icon-normal.png"),
          tr("このソフトウェアについて(&A)" /*分離すると翻訳しづらそう*/), this,
          &MainWindow::showAbout)
      ->setToolTip(
          tr("バージョンやライセンスについてのダイアログを表示します。"));
  help_menu
      ->addAction(style()->standardIcon(QStyle::SP_TitleBarMenuButton),
                  tr("Qtについて(&Q)"), qApp, &QApplication::aboutQt)
      ->setToolTip(tr("使用されているQtのライブラリのバージョンやライセンスにつ"
                      "いてのダイアログを表示します。"));
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:タイムライン(トゥートを並べていくところ)を作って設定する。ウィンドウ初期化用。
 */
void MainWindow::createTimeLine() {
  QScrollArea *timeline_area = new QScrollArea;
  QPalette palette = timeline_area->palette();
  palette.setColor(QPalette::Window, Qt::black);
  timeline_area->setPalette(palette);
  timeline_area->setFrameShape(QFrame::NoFrame); //枠線をなくす
  // timeline_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//表示しない
  timeline_area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn); //常に表示
  timeline_area->setWidgetResizable(true); //先にこれを設定する。

  //トゥートを並べていくスクロールエリア
  QWidget *timeline_center = new QWidget;
  timeline_layout =
      new QBoxLayout(QBoxLayout::BottomToTop, timeline_center); //下から上
  timeline_area->setWidget(timeline_center);
  main_layout->addWidget(timeline_area);
  return;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:トゥートボックス(入力欄とボタンなど)を作って設定する。ウィンドウ初期化用。
 */
void MainWindow::createTootBox() {
  //入力ボックス
  toot_editer = new QPlainTextEdit; // HTMLが扱えないようにする
  toot_editer->setFixedHeight(100); // Editボックスのサイズ固定(サイズは適当)
  toot_editer->installEventFilter(this);
  main_layout->addWidget(toot_editer);

  //色々トゥートに関する情報を出すためのスクロールエリア
  info_scroll_area = new QScrollArea;
  info_scroll_area->setFixedHeight(100); //サイズ固定(サイズは適当)
  info_scroll_area->setWidgetResizable(true);
  toot_info = new TootInfo(this);
  info_scroll_area->setWidget(toot_info); //ここの順序大切
  info_scroll_area->setVisible(false);
  main_layout->addWidget(info_scroll_area);

  //トゥートボタンボックス
  QHBoxLayout *toot_button_layout =
      new QHBoxLayout; //将来ボタンを増やした時のために
  toot_button_layout->addStretch();

  // Home Time Line更新
  QPushButton *ReloadButton = new QPushButton;
  ReloadButton->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
  ReloadButton->setToolTip(tr("タイムラインを更新します。"));
  ReloadButton->setStyleSheet("background-color: #255080;");
  connect(ReloadButton, &QPushButton::clicked, this,
          &MainWindow::updateTimeLine);
  toot_button_layout->addWidget(ReloadButton);

  //メディア追加ボタン
  QPushButton *MediaButton = new QPushButton;
  MediaButton->setIcon(QIcon(":/add.png"));
  MediaButton->setToolTip(tr("写真・動画を追加します。"));
  MediaButton->setStyleSheet("background-color: #255080;");
  connect(MediaButton, &QPushButton::clicked, this, &MainWindow::addMedia);
  toot_button_layout->addWidget(MediaButton);

  //トゥート送信ボタン
  toot_button = new QPushButton;
  toot_button->setIcon(QIcon(":/send.png"));
  toot_button->setStyleSheet("background-color: #255080;");
  toot_button->setToolTip(tr("トゥートを送信します。"));
  connect(toot_button, &QPushButton::clicked, this, &MainWindow::toot);
  toot_button_layout->addWidget(toot_button);

  // main_layoutにトゥートボタンボックスを追加
  main_layout->addLayout(toot_button_layout);
  return;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:Mastodonアプリ連携の認証を行う。それ以外では呼ばない。同期処理を行う。
 */
void MainWindow::authorizeMastodon() {
  /*throwしまくりである...(throw時のmstdnの削除は他でするので気にしなくてよい*/
  QEventLoop event; //ここだけ同期処理

  QString &&domain = showInstanceDomainInputDialog();

  if (domain.isEmpty()) {
    throw tr("無効なドメイン名です。");
  }
  QNetworkReply *rep = mstdn->registerApp(domain);
  connect(rep, &QNetworkReply::finished, &event, &QEventLoop::quit);
  event.exec();

  if (rep->error() != QNetworkReply::NoError) {
    delete rep;
    throw tr("アプリケーションの登録に失敗しました。");
  }
  QJsonObject &&result = QJsonDocument::fromJson(rep->readAll()).object();
  delete rep;

  QByteArray &&client_id = result["client_id"].toString().toLatin1();
  QByteArray &&client_secret = result["client_secret"].toString().toLatin1();

  if (!QDesktopServices::openUrl(mstdn->getAuthorizeUrl(domain, client_id)))
    throw tr("ブラウザの起動に失敗しました。");

  QString &&auth_code = showAuthCodeInputDialog();
  if (auth_code.isEmpty())
    throw tr("認証がキャンセルされました。");

  rep = mstdn->requestAccessToken(domain, client_id, client_secret, auth_code);
  connect(rep, &QNetworkReply::finished, &event, &QEventLoop::quit);
  event.exec();

  if (rep->error() != QNetworkReply::NoError) {
    delete rep;
    throw tr("アクセストークンの取得に失敗しました。");
  }
  QByteArray &&access_token = QJsonDocument::fromJson(rep->readAll())
                                  .object()["access_token"]
                                  .toString()
                                  .toUtf8();
  delete rep;

  mstdn->setInstanceDomain(domain);
  mstdn->setAccessToken(access_token);

  rep = mstdn->requestCurrentAccountInfo();
  connect(rep, &QNetworkReply::finished, &event, &QEventLoop::quit);
  event.exec();

  if (rep->error() != QNetworkReply::NoError) {
    delete rep;
    throw tr("ユーザ情報の取得に失敗しました。");
  }
  TootAccountData user_info(QJsonDocument::fromJson(rep->readAll()).object());
  rep->deleteLater();

  mstdn->setUserId(user_info.getId());

  setting->setClientId(client_id);
  setting->setClientSecret(client_secret);
  setting->setInstanceDomain(domain);
  setting->setAccessToken(access_token);
  setting->setUserId(user_info.getId());
  setting->setUserName(user_info.getUserName());
}

/*
 * 引数:なし
 * 戻値:入力されたドメイン名
 * 概要:インスタンスのドメインのダイアログを作って表示して結果を返す。
 */
QString MainWindow::showInstanceDomainInputDialog() {
  QDialog dialog;
  QVBoxLayout *layout = new QVBoxLayout(&dialog);
  QLineEdit *domain_editer = new QLineEdit;
  QPushButton *ok_button = new QPushButton(tr("OK"));

  connect(ok_button, &QPushButton::clicked, &dialog, &QWidget::close);
  layout->addWidget(
      new QLabel(tr("認証するインスタンスのドメイン名を入力してください。("
                    "https://は不要です。)\n例) don.taprix.org")));
  layout->addWidget(domain_editer);
  layout->addWidget(ok_button);
  dialog.exec();
  return domain_editer->text();
}

/*
 * 引数:なし
 * 戻値:入力された文字列
 * 概要:認証コード入力ダイアログを作って表示して結果を返す。
 */
QString MainWindow::showAuthCodeInputDialog() {
  QDialog dialog;
  QVBoxLayout *layout = new QVBoxLayout(&dialog);
  QLineEdit *pincode_editer = new QLineEdit;
  QPushButton *ok_button = new QPushButton(tr("OK"));

  connect(ok_button, &QPushButton::clicked, &dialog, &QWidget::close);
  layout->addWidget(
      new QLabel(tr("表示されたブラウザでMastodonの認証して、表示された認証コー"
                    "ドを入力してください。")));
  layout->addWidget(pincode_editer);
  layout->addWidget(ok_button);
  dialog.exec();
  return pincode_editer->text();
}

/*
 * 引数:error(エラー番号)
 * 戻値:なし
 * 概要:Streamerでエラーが起きた時に呼ばれる。再接続するか確認する。
 */
void MainWindow::abortedTimeLine(unsigned int error) {
  QMessageBox mes_box;

  mes_box.setWindowTitle(APP_NAME);
  mes_box.setIcon(QMessageBox::Critical);
  stream_status->setChecked(false);

  switch (static_cast<Streamer::Error>(error)) {
  case Streamer::CannotConnect:
    mes_box.setText(tr("タイムラインに接続できませんでした。再接続しますか。"));
    mes_box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    break;
  case Streamer::NetworkError:
    mes_box.setText(tr("タイムラインから切断されました。再接続しますか。"));
    mes_box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    break;
  case Streamer::BadPointer:
    mes_box.setText(tr("メモリアクセス違反が発生しました。"));
    break;
  default:
    mes_box.setText(tr("不明なエラーが発生しました。"));
  }
  if (mes_box.exec() == QMessageBox::Yes) {
    stream_status->setChecked(true);
    QMetaObject::invokeMethod(timeline_streamer, "startUserStream",
                              Qt::QueuedConnection);
  }
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:メニューでストリームから切断がクリックされた時呼ばれる。
 */
void MainWindow::changeStatusStream(bool checked) {
  //この処理ではいつか問題が生じるのでもっと細かく制御すべき
  if (checked)
    QMetaObject::invokeMethod(timeline_streamer, "startUserStream",
                              Qt::QueuedConnection);
  else
    timeline_streamer->stopUserStream();
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:メインウィンドウを表示する時呼ばれる。requestHomeTimeLineの取得を行う。
 */
void MainWindow::show() {
  QMainWindow::show();
  connect(mstdn->requestHomeTimeLine(), &QNetworkReply::finished, this,
          &MainWindow::showTimeLine);
  connect(mstdn->requestGetLists(), &QNetworkReply::finished, this,
          &MainWindow::setListsMenu);
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:ウィンドウ表示の際、メニューの表示=>リストに所持してるリストを設定する。
 */
MastodonAPI *MainWindow::copyMastodonAPI() const {
  return new MastodonAPI(*mstdn);
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:ウィンドウ表示の際、メニューの表示=>リストに所持してるリストを設定する。
 */
void MainWindow::setListsMenu() {
  QNetworkReply *rep = qobject_cast<QNetworkReply *>(sender());
  for (const QJsonValue &list_entry :
       QJsonDocument::fromJson(rep->readAll()).array()) {
    QJsonObject list_entry_object = list_entry.toObject();
    list_menu
        ->addAction(list_entry_object["title"].toString(), this,
                    [] { /*Temporary*/ })
        ->setData(list_entry_object["id"].toString());
  }
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:QNetworkReplyによって呼ばれる。requestHomeTimeLineの処理を行う。
 */
void MainWindow::showTimeLine() {
  QNetworkReply *rep = qobject_cast<QNetworkReply *>(sender());
  if (rep->error() == QNetworkReply::NoError) {
    QJsonArray toots = QJsonDocument::fromJson(rep->readAll()).array();

    for (int i = toots.size() - 1; i >= 0; i--) {
      QJsonObject obj = toots[i].toObject();
      TootData *tdata = new TootData(obj);
      if (!tdata->isEmpty())
        showToot(tdata);
    }
  }
  rep->deleteLater();
  if (stream_status->isChecked())
    QMetaObject::invokeMethod(timeline_streamer, "startUserStream",
                              Qt::QueuedConnection); //ストリームスタート
  return;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:ホームタイムラインの更新を行う。
 */
void MainWindow::updateTimeLine() {
  if (const int count = timeline_layout->count(); count > 0) {
    connect(mstdn->requestHomeTimeLine(
                (qobject_cast<TootContent *>(
                     timeline_layout->itemAt(count - 1)->widget()))
                    ->getTootData()
                    ->getId()),
            &QNetworkReply::finished, this, &MainWindow::showTimeLine);
  }
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:トゥートに画像・動画を追加する。動画は未対応。
 */
void MainWindow::addMedia() {
  try {
    QString &&file_path = QFileDialog::getOpenFileName(
        this, tr("メディア追加"), "",
        tr("メディア (*.png *.jpg *.gif *.bmp *.pbm *.pgm *.ppm *.xbm *.xpm "
           "*.svg)")); //対応拡張子はImageViewer参考にして動的に作るべき
    if (file_path.isEmpty())
      return;
    QFileInfo file_info(file_path);

    //動画は分岐させる
    QByteArrayList &&supported_formats = QImageReader::supportedImageFormats();
    QByteArray &&suffix = file_info.suffix().toLower().toUtf8();
    for (const QByteArray &e : supported_formats) {
      if (e == suffix)
        break;
      if (e == supported_formats.last())
        throw tr("サポートしていない画像です。");
    }

    unsigned int counter = toot_info->getNumOfImage();
    if (counter > 4)
      throw tr("４枚以上の画像は投稿できません。");

    // toot_infoに追加
    toot_info->setImage(QPixmap(file_path).scaled(50, 50), file_path, counter);
    info_scroll_area->setVisible(true);
  } catch (QString &e) {
    QMessageBox::critical(this, APP_NAME, e);
  }
  return;
}

/*
 * 引数:なし
 * 戻値:クリップボードから設定できたか。
 * 概要:画像をクリップボードから設定する。toot_editerの右クリックメニューで設定できたら面白そう。
 */
bool MainWindow::addMediaByClipboard() {
  QPixmap &&pic = QApplication::clipboard()->pixmap();
  if (pic.isNull())
    return false;

  unsigned int counter = toot_info->getNumOfImage();
  if (counter > 4) {
    QMessageBox::critical(this, APP_NAME,
                          tr("４枚以上の画像は投稿できません。"));
    return false;
  }
  // toot_infoに追加
  toot_info->setImage(pic, "", counter);
  info_scroll_area->setVisible(true);
  return true;
}

/*
 * 引数:checked(メニューがチェックされてるかどうか)
 * 戻値:なし
 * 概要:Windowを最前面に表示するかどうか。ウィンドウマネージャで設定できるならそっちのほうが適切。
 */
void MainWindow::setAlwayTop(bool checked) {
  setWindowFlag(Qt::WindowStaysOnTopHint, checked);
  QMainWindow::show();
}

/*
 * 引数:tdata(新規表示するトゥート)
 * 戻値:なし
 * 概要:主にStreamerより呼ばれる。
 */
void MainWindow::showToot(TootData *tdata) {
  if (tdata == nullptr)
    return;
  TootContent *content = new TootContent(
      tdata, TootContent::Normal,
      this); //クリックの検出などをするため(Layoutを直接噛ませるのとどちらがいいんだろう)

  connect(content, &TootContent::action, this, &MainWindow::contentAction);

  //古いものを削除
  if (timeline_layout->count() >= setting->getTootLimit()) {
    QLayoutItem *old =
        timeline_layout->takeAt(0); //一番最初に追加したやつから番号が振られる。
    if (old)
      delete old->widget();
  }

  //追加
  timeline_layout->addWidget(content);
  return;
}

/*
 * 引数:Id(削除されたトゥートのid)
 * 戻値:なし
 * 概要:主にStreamerのDeletetootシグナルによって呼ばれる。削除されたトゥートのTootContentを消す。
 */
void MainWindow::removeToot(const QString &id) {
  for (unsigned int c = timeline_layout->count() - 1; c; c--) {
    QLayoutItem *item = timeline_layout->itemAt(c);
    if (item == nullptr)
      continue;
    TootData *tdata =
        (qobject_cast<TootContent *>(item->widget()))->getTootData();
    if (tdata == nullptr)
      continue;
    if (tdata->getId() == id ||
        (tdata->getBoostedData() && tdata->getBoostedData()->getId() == id)) {
      item = timeline_layout->takeAt(c); // takeAtを一回呼ぶ
      if (item != nullptr)
        delete item->widget();
    }
  }
}

/*
 * 引数:tdata(新規表示するトゥート)
 * 戻値:なし
 * 概要:主にStreamerより呼ばれる。NotificationDataを使って通知を表示する。
 */
void MainWindow::showNotification(TootNotificationData *nfdata) {
  if (nfdata == nullptr)
    return;

  QString message;
  switch (nfdata->getType()) {
  case TootNotificationData::Event::Favourite:
    message = nfdata->getAccount().getDisplayName() + tr("さんが") +
              tr("以下のトゥートをお気に入りに登録しました。\n") +
              ((nfdata->getStatus().getContent().isEmpty())
                   ? tr("不明")
                   : nfdata->getStatus().getContent());
    break;

  case TootNotificationData::Event::Boost:
    message = nfdata->getAccount().getDisplayName() + tr("さんが") +
              tr("以下のトゥートをブーストしました。\n") +
              ((nfdata->getStatus().getContent().isEmpty())
                   ? tr("不明")
                   : nfdata->getStatus().getContent());
    break;

  case TootNotificationData::Event::Mention:
    message = nfdata->getAccount().getDisplayName() +
              tr("さんがあなたに向けてトゥートしました。\n") +
              nfdata->getStatus().getContent();
    break;

  case TootNotificationData::Event::Follow:
    message = nfdata->getAccount().getDisplayName() + "(" +
              nfdata->getAccount().getAcct() + ")" +
              tr("さんがあなたをフォローしました。");
    break;

  default:
    return delete nfdata; //未対応
  }
  tray_info->show();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
  //もしアイコンが取得済みならアイコンを表示(ネットから取ってくるのには手間がかかる)(beta)
  QIcon icon;
  ImageLabel icon_search;

  if (icon_search.setPixmapByName(nfdata->getAccount().getAvatar())) {
    const QPixmap icon_pixmap = icon_search.pixmap();
    if (!icon_pixmap.isNull()) {
      icon.addPixmap(icon_pixmap);
    }
  } else {
    icon = qApp->windowIcon();
  }
  tray_info->showMessage(APP_NAME, message, icon);
#else
  tray_info->showMessage(APP_NAME, message);
#endif
  tray_info->hide(); //これで一応メッセージだけ出る。
  delete nfdata;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:トゥートが完了した時に呼ばれる。トゥートボックスの内容の削除やトゥートボタンの有効化を行う。
 */
void MainWindow::finishedToot() {
  QNetworkReply *rep = qobject_cast<QNetworkReply *>(sender());
  if (rep->error() != QNetworkReply::NoError) {
    QMessageBox::critical(this, tr("トゥートに失敗しました ") + APP_NAME,
                          rep->errorString());
  } else {
    toot_editer->clear();
    toot_info->deleteImageAll();
    toot_info->deleteQuoteToot();
    toot_info->deleteReplyToot();
    info_scroll_area->setVisible(false);
  }
  setEnabledToot(true);
  rep->deleteLater(); //成功時、本来は読むべきだが...特に読む必要もないので静かに閉じる
  return;
}

/*
 * 引数:bool(トゥート入力を受け付けるかどうか)
 * 戻値:なし
 * 概要:トゥートボックスの有効化やトゥートボタンの有効化を行う。
 */
void MainWindow::setEnabledToot(bool enable) {
  toot_button->setEnabled(enable);
  toot_editer->setEnabled(enable);
  if (enable)
    toot_editer->setFocus(); //こうしとくと便利
  return;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:後処理の必要ない作業が完了したら呼ばれる。
 */
void MainWindow::finishedRequest() {
  QNetworkReply *rep = qobject_cast<QNetworkReply *>(sender());
  if (rep->error() != QNetworkReply::NoError) {
    QMessageBox::critical(this, tr("作業に失敗しました ") + APP_NAME,
                          rep->errorString());
  }
  rep->deleteLater();
  return;
}

/*
 * 引数:qkey
 * 戻値:なし
 * 概要:キーが押されたときに呼ばれる。ショートカットキーを拾う。
 */
void MainWindow::keyPressEvent(QKeyEvent *qkey) {
  qkey->ignore();
  if (qkey->modifiers().testFlag(Qt::ControlModifier)) {
    switch (qkey->key()) {
    case Qt::Key_Return:
      toot(); // Ctrl + Enter
      break;
    case Qt::Key_V:
      if (!addMediaByClipboard())
        return;
      break;
    default:
      return;
    }
    qkey->accept();
  }
  return;
}

/*
 * 引数:obj(発生源), event(イベントの種類)
 * 戻値:処理したかどうか
 * 概要:installEventFilterしたオブジェクトのイベントが流れてくる。
 */
bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
  event->ignore();
  if (obj == toot_editer) {
    switch (event->type()) {
    case QEvent::KeyPress:
      keyPressEvent(static_cast<QKeyEvent *>(event));
      break;
    default: //警告回避
             ;
    }
  }
  return event->isAccepted(); //意味ない気も..
}

/*
 * 引数:event
 * 戻値:なし
 * 概要:ウィンドウを閉じる時に呼ばれる。ウィンドウ位置の保存を行う。
 */
void MainWindow::closeEvent(QCloseEvent *event) {
  //ウィンドウ状態保存
  setting->setGeometry(saveGeometry());
  QMainWindow::closeEvent(event);
  tray_info->hide();
  return;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:メインウィンドウ下のトゥートボックスを使ってトゥートするときに呼ぶ。トゥート作業を行う。
 */
void MainWindow::toot() {
  if (!toot_button->isEnabled())
    return; //作業中
  setEnabledToot(false);
  try {
    if (unsigned int c = toot_info->getNumOfImage()) {
      //画像あり
      QList<QIODevice *> media;
      QByteArrayList mime;
      media.reserve(c);
      mime.reserve(c);
      for (unsigned int i = 0; i < c; i++) {
        QString file_path = toot_info->getImagePath(i);
        if (file_path.isEmpty()) {
          //クリップボードなどの画像
          QBuffer *buff = new QBuffer;
          buff->open(QIODevice::WriteOnly);
          if (!toot_info->getImage(i).save(buff, "PNG"))
            throw tr("画像の読み込みに失敗しました。");
          mime.push_back(QByteArray("image/png"));
          buff->close();
          buff->open(QIODevice::ReadOnly);
          media.append(buff);
        } else {
          QFile *file = new QFile(file_path);
          if (!file->open(QIODevice::ReadOnly)) {
            throw tr("画像の読み込みに失敗しました。");
          }
          mime.push_back(QMimeDatabase().mimeTypeForData(file).name().toUtf8());
          media.append(file);
        }
      }
      MediaUpload *upload = new MediaUpload(media, mime, mstdn, this);
      connect(upload, &MediaUpload::finished, this, &MainWindow::tootWithMedia);
      connect(upload, &MediaUpload::aborted, this, [this] {
        MediaUpload *upload = qobject_cast<MediaUpload *>(sender());
        if (QMessageBox::question(
                this, APP_NAME,
                tr("アップロード中にエラーが発生しました。再試行しますか。")) ==
            QMessageBox::Yes)
          upload->retry();
        else
          delete upload;
        setEnabledToot(true);
      });
      if (!upload->start()) {
        delete upload;
        throw tr("アップロードの初期化作業に失敗しました。");
      }
      return;
    }
    QString text = toot_editer->toPlainText();
    processToot(text);
    if (text.isEmpty())
      return setEnabledToot(true);

    connect(mstdn->requestToot(text, QByteArray(),
                               (toot_info->getReplyToot())
                                   ? toot_info->getReplyToot()->getId()
                                   : QByteArray()),
            &QNetworkReply::finished, this, &MainWindow::finishedToot);
  } catch (QString &e) {
    QMessageBox::critical(this, APP_NAME, e);
    setEnabledToot(true);
  }
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:MediaUploadによって呼ばれる。画像とともにトゥートする。
 */
void MainWindow::tootWithMedia(const QByteArray &id) {
  qobject_cast<MediaUpload *>(sender())->deleteLater();
  QString text = toot_editer->toPlainText();
  processToot(text);
  connect(mstdn->requestToot(text, id,
                             (toot_info->getReplyToot())
                                 ? toot_info->getReplyToot()->getId()
                                 : QByteArray()),
          &QNetworkReply::finished, this, &MainWindow::finishedToot);
}

/*
 * 引数:加工するトゥートテキスト
 * 戻値:なし
 * 概要:toot_infoにある情報を使ってトゥートテキストを加工する。
 */
void MainWindow::processToot(QString &text) {
  if (TootData *reply_toot = toot_info->getReplyToot()) {
    if (!text.contains("@" + reply_toot->getOriginalAccountData().getAcct())) {
      text.prepend("@" + reply_toot->getOriginalAccountData().getAcct() + " ");
    }
  }
  if (TootData *quote_toot = toot_info->getQuoteToot()) {
    text += "  https://" + setting->getInstanceDomain() + "/web/statuses/" +
            quote_toot->getId();
  }
  return;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:バージョン情報を表示するダイアログを出す。
 */
void MainWindow::showAbout() {
  QMessageBox::about(
      this, APP_NAME,
      "<b>" APP_NAME_LONG "</b>"
      "<p>Ver " APP_VERSION "</p>"
      "<p>" +
          tr("Qtを使用して製作されているMastodonクライアント。") +
          "</p>"
          "<p>" +
          tr("本ソフトウェアはQtオープンソース版のLGPLv3を選択しています。詳し"
             "くは<a "
             "href=\"https://www.qt.io/licensing/\">https://www.qt.io/"
             "licensing/</a>をご覧ください。") +
          "</p>" //ここは<a>で区切ると訳しにくいはず
          "<b>" +
          tr("License") +
          "</b>"
          "<p>" APP_COPYRIGHT "<br /><br />"
          "Licensed under the Apache License, Version 2.0 (the "
          "\"License\");<br />"
          "you may not use this file except in compliance with the License.<br "
          "/>"
          "You may obtain a copy of the License at<br /><br />"
          "<a href=\"https://www.apache.org/licenses/LICENSE-2.0\" "
          ">https://www.apache.org/licenses/LICENSE-2.0</a><br />"
          "Unless required by applicable law or agreed to in writing, software"
          "distributed under the License is distributed on an \"AS IS\" BASIS, "
          "WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or "
          "implied.<br />"
          "See the License for the specific language governing permissions and"
          "limitations under the License.</p>"
          "<a href=\"" APP_HOMEPAGE "\">" +
          tr("このソフトウェアについてのページを開く") + "</a>");
}

/*
 * 引数:tdata(ブーストしたいトゥート)
 * 戻値:なし
 * 概要:ブーストをする。
 */
void MainWindow::boost(TootData *tdata) {
  if (tdata == nullptr) {
    ;
  } else if (tdata->isBoosted()) {
    QMessageBox::information(this, APP_NAME,
                             tr("このトゥートはすでにブーストしています。"));
  } else if (tdata->isPrivateToot()) {
    QMessageBox::information(this, APP_NAME,
                             tr("非公開のトゥートのためブーストできません。"));
  } else if (tdata->iSDirectMessage()) {
    QMessageBox::information(
        this, APP_NAME, tr("ダイレクトメッセージのためブーストできません。"));
  } else {
    connect(mstdn->requestBoost(tdata->getId()), &QNetworkReply::finished, this,
            &MainWindow::finishedRequest);
  }
}

/*
 * 引数:tdata(お気に入りに登録したいトゥート)
 * 戻値:なし
 * 概要:お気に入りに登録する。
 */
void MainWindow::favourite(TootData *tdata) {
  if (tdata == nullptr) {
    ;
  } else if (tdata->isFavourited()) {
    QMessageBox::information(
        this, APP_NAME, tr("このトゥートはすでにお気に入りに登録しています。"));
  } else if (QMessageBox::question(
                 this, APP_NAME,
                 tdata->getOriginalAccountData().getDisplayName() +
                     tr("さんのトゥートをお気に入りに登録しますか。")) ==
             QMessageBox::Yes) {
    connect(mstdn->requestFavourite(tdata->getId()), &QNetworkReply::finished,
            this, &MainWindow::finishedRequest);
  }
}

/*
 * 引数:tdata(削除したいトゥート)
 * 戻値:なし
 * 概要:トゥートを削除する。
 */
void MainWindow::deleteToot(TootData *tdata) {
  if (tdata == nullptr) {
    return;
  }
  if (QMessageBox::question(this, APP_NAME, tr("トゥートを削除しますか。")) ==
      QMessageBox::Yes) {
    connect(mstdn->requestDeleteToot(tdata->getId()), &QNetworkReply::finished,
            this, &MainWindow::finishedRequest);
  }
}

void MainWindow::contentAction(TootData *tdata, unsigned char act) {
  if (tdata == nullptr) {
    return;
  }
  switch (act) { // TODO: enum化
  case 'b':
    boost(tdata);
    break;
  case 'q':
    if (tdata->isPrivateToot()) {
      QMessageBox::information(this, APP_NAME,
                               tr("非公開のトゥートのため引用できません。"));
    } else if (tdata->iSDirectMessage()) {
      QMessageBox::information(
          this, APP_NAME, tr("ダイレクトメッセージのため引用できません。"));
    } else {
      TootContent *content =
          new TootContent(new TootData(*tdata), TootContent::Info);
      toot_info->setQuoteToot(content);
      toot_info->setVisible(true);
      info_scroll_area->setVisible(true);
    }
    break;
  case 'r': {
    TootContent *content =
        new TootContent(new TootData(*tdata), TootContent::Info);
    toot_info->setReplyToot(content);
    toot_info->setVisible(true);
    info_scroll_area->setVisible(true);
    break;
  }
  case 'f':
    favourite(tdata);
    break;
  case 'd':
    deleteToot(tdata);
    break;
  default:; //とりあえず今の所何もしない
  }
}
