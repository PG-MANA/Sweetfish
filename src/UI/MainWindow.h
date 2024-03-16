/*
 * Copyright 2017 PG_MANA
 *
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE.md
 *
 * メインウィンドウ クラス
 * 主役である画面の表示を請け負っている。
 */
#pragma once

#include "../Mastodon/Streamer.h"
#include "../Network/Network.h"
#include <QMainWindow>

class MastodonAPI;
class TootData;
class TootNotificationData;
class TootInfo;
class Setting;
class QCloseEvent;
class QKeyEvent;
class QVBoxLayout;
class QHBoxLayout;
class QBoxLayout;
class QPushButton;
class QPlainTextEdit;
class QThread;
class QString;
class QByteArray;
class QScrollArea;
class QSystemTrayIcon;

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  explicit MainWindow();
  virtual ~MainWindow();
  bool init(const QString setting_file_name);
  MastodonAPI *copyMastodonAPI() const;
public slots:
  void show();
  void toot();
  void tootWithMedia(const QByteArray &id);
  void addMedia();
  void showToot(TootData *tdata);
  void removeToot(const QString &id);
  void showNotification(TootNotificationData *nfdata);
  void showTimeLine();
  void updateTimeLine();
  void showAbout();
  void contentAction(TootData *tdata, unsigned char act);
  void finishedToot();
  void finishedRequest();
  void abortedTimeLine(unsigned int error);
  void setAlwayTop(bool checked);
  void setListsMenu();
  void changeStreamStatus(bool checked);

protected:
  virtual void closeEvent(QCloseEvent *event) override;
  virtual void keyPressEvent(QKeyEvent *qkey) override;
  virtual bool eventFilter(QObject *obj, QEvent *event) override;
  void boost(TootData *tdata);
  void favourite(TootData *tdata);
  void deleteToot(TootData *tdata);
  Network net;

private:
  void createMenus();
  void createTimeLine();
  void createTootBox();
  void clearToots();

  void authorizeMastodon();
  bool addMediaByClipboard();
  inline void setEnabledToot(bool enable);
  void processToot(QString &text);

  QString showAuthCodeInputDialog();
  QString showInstanceDomainInputDialog();
  MastodonAPI *mstdn; // 将来複数持てるかも
  Setting *setting;
  QVBoxLayout *main_layout;
  QScrollArea *info_scroll_area; // ツイートに画像などを添付する時表示されるもの
  QPushButton *toot_button;
  TootInfo *toot_info;
  QPlainTextEdit *toot_editer;
  QBoxLayout *timeline_layout;
  Streamer *timeline_streamer;
  QThread *timeline_thread;
  QAction *stream_status;
  QSystemTrayIcon *
      tray_info; // これだとMainWindowが複数できたときにそれごとにトレイに追加されるのでstaticで管理するか、Sweetfish.cppが管理する必要が出てくるかもしれない。ただし、show()=>showMessage()=>hide()であたかもメッセージだけ表示された感じになる。これもヒープ上に作るのが世の常らしい(QtドキュメントもHeap上に作ってる。)。
  QMenu *list_menu;
  Streamer::StreamType stream_type = Streamer::StreamType::UserStream;
  QByteArray stream_id = QByteArray();
};
