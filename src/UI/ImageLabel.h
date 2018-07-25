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
                      TootContent *init_parent_content = Q_NULLPTR,
                      QWidget *parent = Q_NULLPTR,
                      Qt::WindowFlags f = Qt::WindowFlags());
  bool setPixmapByName(const QString &name);

  TootContent *getParentContent();
  void setParentContent(TootContent *new_parent_content);
  void getSize(unsigned int &ref_sizex, unsigned int &ref_sizey);
  void setSize(unsigned int new_sizex, unsigned int new_sizey);
  unsigned int getIndex();
  void setIndex(unsigned int index);

signals:
  void clicked(TootData *tdata, unsigned int index);
  void rightClicked(TootData *tdata, unsigned int index);
public slots:
  void setPixmapByNetwork(); // QNetworkReplyのfinishedと接続する。
protected:
  void mousePressEvent(QMouseEvent *event) override;
  static QHash<QString, QPixmap> images; // C++でいう、unordered_map

  QString url;
  TootContent *parent_content; //親(TweetDataを引っ張り出すため)
  unsigned int index;          //何番目か(0から始まる)
  unsigned int sizex, sizey;   //縮小サイズ(0なら縮小しない)
};
