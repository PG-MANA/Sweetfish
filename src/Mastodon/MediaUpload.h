/*
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE
 *
 * MediaUpload クラス
 * MediaUploadに使う。media_idを返す。
 */
#pragma once

#include <QByteArray>
#include <QList>
#include <QObject>

class MastodonAPI;
class QIODevice;

class MediaUpload : public QObject {
  Q_OBJECT
public:
  explicit MediaUpload(QList<QIODevice *> _list /*upload後にfreeされる*/,
                       const QByteArrayList &mime, MastodonAPI *m,
                       QObject *parent = Q_NULLPTR);
  virtual ~MediaUpload();
  bool start();

signals:
  void finished(const QByteArray &media_id);
  void aborted();

public slots:
  void next();
  void retry();

private:
  QList<QIODevice *> list;
  QByteArrayList mimetype;
  QByteArray id;       //=media_ids
  QByteArray media_id; //操作中のmedia_id
  MastodonAPI *mastodon_api;
  unsigned int counter;
};
