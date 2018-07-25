/*
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE
 *
 * MediaUpload クラス
 * MediaUploadに使う。media_idを返す。
 */
#pragma once

#include <QByteArray>
#include <QObject>

class Mastodon;

class MediaUpload : public QObject {
  Q_OBJECT
public:
  explicit MediaUpload(const QByteArrayList &_list, const QByteArrayList &mime,
                       Mastodon *m, QObject *parent = Q_NULLPTR);
  virtual ~MediaUpload();
  bool start();

signals:
  void finished(const QByteArray &media_id);
  void aborted();

public slots:
  void next();
  void retry();

private:
  QByteArrayList list;
  QByteArrayList mimetype;
  QByteArray id;       //=media_ids
  QByteArray media_id; //操作中のmedia_id
  Mastodon *mastodon;
  unsigned int counter;
};
