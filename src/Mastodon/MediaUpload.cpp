/*
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE
 * MediaUpload クラス
 * MediaUploadに使う。INIT=>APPEND=>FINALIZEと処理していき、media_idを返す。
 */
#include "MediaUpload.h"
#include "MastodonAPI.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>

// MEMO:APIを叩くときはmastodonクラスを使う。(additional_owners対応したら面白いかも。)
MediaUpload::MediaUpload(QList<QIODevice *> _list, const QByteArrayList &mime,
                         MastodonAPI *m, QObject *parent)
    : QObject(parent), list(_list), mimetype(mime), mastodon_api(m),
      counter(0) {}

MediaUpload::~MediaUpload() {}

/*
 * 引数:なし
 * 戻値:開始作業が成功したか。
 * 概要:アップロード作業を開始する。
 */
bool MediaUpload::start() {
  if (mastodon_api == nullptr || !list.size() || !mimetype.size())
    return false;
  connect(
      mastodon_api->requestMediaUpload(*list.at(counter), mimetype.at(counter)),
      &QNetworkReply::finished, this, &MediaUpload::next);
  return true;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:次のファイルを上げる。
 */
void MediaUpload::next() {
  QNetworkReply *rep = qobject_cast<QNetworkReply *>(sender());
  rep->deleteLater();
  if (rep->error() != QNetworkReply::NetworkError::NoError) {
    return emit aborted();
  }
  id += QJsonDocument::fromJson(rep->readAll())
            .object()["id"]
            .toString()
            .toUtf8() +
        ",";

  list.at(counter)->close();
  delete list.at(counter);

  counter++;
  if (counter >= list.count()) {
    emit finished(id);
  } else {
    start();
  }
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:再試行する。失敗したファイルを上げ直す
 */
void MediaUpload::retry() {
  start();
  return;
}
