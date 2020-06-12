/*
 * Copyright 2017 PG_MANA
 *
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE.md
 *
 * TootInfo クラス
 * MainWindowのしたにある画像添付したりリプライしたりするときに出てくるやつ。トゥートに関するデータも保有する。
 */
#pragma once

#include "ImageLabel.h"
#include <QWidget>

class QHBoxLayout;
class QVBoxLayout;
class MainWindow;
class TootData;

class TootInfo : public QWidget {
  Q_OBJECT
public:
  explicit TootInfo(MainWindow *parent_window, QWidget *parent = Q_NULLPTR,
                    Qt::WindowFlags f = Qt::WindowFlags());
  virtual ~TootInfo();
  const QPixmap *getImage(const unsigned int index) const;
  void setImage(const QPixmap &pixmap, const unsigned int index);
  void deleteImage(const unsigned int index);
  void deleteImageAll();
  TootData *getQuoteToot();
  void setQuoteToot(TootContent *data);
  TootData *getReplyToot();
  void setReplyToot(TootContent *data);
  unsigned int countImage() const;
  bool isEmpty();

public slots:
  void deleteQuoteToot();
  void deleteReplyToot();
  void ImageMenu(unsigned int index);

private:
  void closeWhenEmpty();
  MainWindow *win;
  QVBoxLayout *main_layout;
  QHBoxLayout *media_layout;
  QHBoxLayout *reply_layout;
  QHBoxLayout *quote_layout;
};
