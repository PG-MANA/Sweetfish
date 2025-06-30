/*
 * Copyright 2017 PG_MANA
 *
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE.md
 *
 * ImageLabel クラス
 * Salmonで画像の表示するためのキャッシュ機能などをつけたQLabel
 */
#pragma once

#include <QHash>
#include <QLabel>

class QMouseEvent;
class TootContent;
class TootData;

class ImageLabel : public QLabel {
  Q_OBJECT

public:
  explicit ImageLabel(const unsigned int init_sizex = 0,
                      const unsigned int init_sizey = 0,
                      const unsigned int init_index = 0,
                      QString init_url = QString(), QWidget *parent = Q_NULLPTR,
                      Qt::WindowFlags f = Qt::WindowFlags());

  bool setPixmapByName(const QString &name);

  void getSize(unsigned int &ref_sizex, unsigned int &ref_sizey) const;

  void setSize(unsigned int new_sizex, unsigned int new_sizey);

  unsigned int getIndex() const;

  void setIndex(unsigned int index);

signals:
  void clicked(unsigned int index);

  void rightClicked(unsigned int index);

public slots:
  void setPixmapByNetwork(); // QNetworkReplyのfinishedと接続する。
protected:
  void mousePressEvent(QMouseEvent *event) override;

  static QHash<QString, QPixmap> images; // C++でいう、unordered_map

  QString url;
  unsigned int index;        // 何番目か(0から始まる)
  unsigned int sizex, sizey; // 縮小サイズ(0なら縮小しない)
};
