/*
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE
 *
 * 役目は、受信した文字をJSONパースして必要事項を詰め込んで、メインスレッドに投げることだけ。
 */
#include "Streamer.h"
#include "MastodonAPI.h"
#include "TootData.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QUrlQuery>

Streamer::Streamer(QObject *parent)
    : QObject(parent), mastodon_api(nullptr), reply(nullptr) {}

Streamer::~Streamer() {
  stopStream();
  delete mastodon_api;
}

/*
 * 引数:original_mastodon(コピー元Mastodonクラス)
 * 戻値:なし
 * 概要:Mastodonクラス生成。動作するスレッドでよぶ。(QMetaObject::invokeMethodを使ってQt::ConnectionTypeはQt::BlockingQueuedConnection)
 */
void Streamer::setMastodonAPI(MastodonAPI *original_mastodon) {
  if (original_mastodon != nullptr) {
    mastodon_api = new MastodonAPI(*original_mastodon);
  } else {
    mastodon_api = new MastodonAPI;
  }
}

/*
 * 引数:type(接続するStreamの種別), id(MastodonAPIに渡すid,
 * typeによって意味が異なる) 戻値:なし
 * 概要:指定したStreamに再接続する。reply->closeするか、deleteするまで永遠と動く。
 */
void Streamer::startStream(const Streamer::StreamType stream_type,
                           const QByteArray &id) {

  if (mastodon_api == nullptr)
    return emit abort(BadPointer);
  if (reply != nullptr && reply->isRunning())
    return;
  switch (stream_type) {
  case UserStream: {
    reply = mastodon_api->requestUserStream();
    break;
  }
  case ListStream: {
    reply = mastodon_api->requestListStream(id);
    break;
  }
  }

  if (reply->error() != QNetworkReply::NoError) {
    delete reply;
    reply = nullptr;
    return emit abort(CannotConnect);
  }
  connect(reply, &QNetworkReply::readyRead, this, &Streamer::readStream);
  // qnet->getの前にconnectしたい(ただQtのサンプルを見る限り間違った実装ではなさそう)
  connect(reply, &QNetworkReply::finished, this, &Streamer::finishedStream);
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:Streamを停止する。スレッドは削除されない。なおこのときfinishedシグナルが出される。
 */
void Streamer::stopStream() {
  if (reply != nullptr) {
    reply->close();
    delete reply;
    reply = nullptr;
  }
  buffer.clear();
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:QNetworkReply::readyReadのslot。読み込んでTootDataに格納しNewTootシグナルを発生させる。
 */
void Streamer::readStream() {
  // https://developer.mozilla.org/ja/docs/Server-sent_events/Using_server-sent_events#Event_stream_format
  buffer += reply->readAll();
  // もうちょっといい実装ありそう
  QList<QByteArray> &&message_list = buffer.split('\n');
  unsigned int cnt;
  unsigned int size = message_list.size();
  TootData *tdata = nullptr;
  TootNotificationData *nfdata = nullptr;
  QByteArray delete_id;

  for (cnt = 0; cnt < size; cnt++) {
    if (message_list.at(cnt).startsWith("event:")) {
      if (cnt + 2 >= size || !message_list.at(cnt + 2).isEmpty()) {
        break; // まだ読み込み途中
      }
      // switch使いたい
      QByteArray &&event_type =
          message_list.at(cnt).right(message_list.at(cnt).size() - 7);
      if (event_type == "update") {
        cnt++;
        tdata = new TootData(
            QJsonDocument::fromJson(
                message_list.at(cnt).right(message_list.at(cnt).size() - 6))
                .object());
        cnt++;
        if (tdata->isEmpty()) {
          delete tdata;
          tdata = nullptr;
        }
      } else if (event_type == "notification") {
        cnt++;
        nfdata = new TootNotificationData(
            QJsonDocument::fromJson(
                message_list.at(cnt).right(message_list.at(cnt).size() - 6))
                .object());
        cnt++;
        if (nfdata->isEmpty()) {
          delete nfdata;
          nfdata = nullptr;
        }
      } else if (event_type == "delete") {
        cnt++;
        delete_id = message_list.at(cnt).right(message_list.at(cnt).size() - 6);
        cnt++;
      }
    }
  }
  buffer.clear();
  if (cnt < size) {
    // 遅そう
    for (; cnt < size; cnt++) {
      if (!buffer.isEmpty()) {
        buffer += '\n';
      }
      buffer += message_list.at(cnt);
    }
  }

  if (tdata != nullptr) {
    return emit newToot(tdata);
  }
  if (nfdata != nullptr) {
    return emit newNotification(nfdata);
  }
  if (!delete_id.isEmpty()) {
    return emit deleteToot(delete_id);
  }
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:QNetworkReply::finishedのslot。まずストリームを閉じて、シグナルを送出する。
 */
void Streamer::finishedStream() {
  if (reply) {
    QNetworkReply::NetworkError error = reply->error();
    if (reply->isRunning())
      stopStream();
    if (error != QNetworkReply::OperationCanceledError)
      emit abort(NetworkError);
  }
}
