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

  virtual ~Streamer();

  enum class Error : unsigned int { CannotConnect, NetworkError, BadPointer };

  enum class StreamType : unsigned int { UserStream, ListStream };

signals:
  void newToot(TootData *twdata);

  void deleteToot(const QString &id);

  void newNotification(TootNotificationData *nfdata);

  void abort(Error err);

public slots:
  void setMastodonAPI(MastodonAPI *original_mastodon);

  void startStream(StreamType stream_type, const QByteArray &id = QByteArray());

  void startStream(const unsigned int stream_type,
                   const QByteArray &id = QByteArray()) {
    startStream(static_cast<StreamType>(stream_type), id);
  }

  void stopStream();

  void readStream();

  void finishedStream();

protected:
  QByteArray buffer;
  MastodonAPI *mastodon_api;
  QNetworkReply *reply;
};
