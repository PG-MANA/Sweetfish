/*
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE
 *
 * Mastodon Stream APIクラス
 * メインスレッドとは別のスレッドで動作する。よってGUIを操作してはいけない。
 */
#pragma once

#include <QObject>

class MastodonAPI;
class QNetworkReply;
class TootData;
class TootNotificationData;

class Streamer : public QObject {
  Q_OBJECT
public:
  explicit Streamer(QObject *parent = nullptr);
  ~Streamer();
  enum Error : unsigned int {
    CannotConnect = 1, /*接続不可能*/
    NetworkError,      /*切断、APIエラー*/
    BadPointer         /*不正なポインタがあった*/
  };

  enum StreamType : unsigned int { UserStream = 0, ListStream };

signals:
  void newToot(TootData *twdata);
  void deleteToot(const QString &id);
  void newNotification(TootNotificationData *nfdata);
  void abort(unsigned int err);

public slots:
  void setMastodonAPI(const MastodonAPI *original_mastodon);
  void startStream(const StreamType stream_type,
                   const QByteArray &id = QByteArray());
  void startStream(const unsigned int stream_type,
                   const QByteArray &id = QByteArray()) {
    /* For invokeMethod */
    return startStream(static_cast<StreamType>(stream_type), id);
  };
  void stopStream();
  void readStream();
  void finishedStream();

protected:
  QByteArray buffer;
  MastodonAPI *mastodon_api;
  QNetworkReply *reply;
};
