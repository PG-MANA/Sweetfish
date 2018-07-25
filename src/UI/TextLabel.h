/*
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE
 *
 * TextLabel クラス
 * どこでも折り返せるラベル。
 */
#pragma once

#include <QFrame>

class TextLabel : public QFrame {
  Q_OBJECT
  Q_PROPERTY(QString text READ text WRITE setText)
public:
  explicit TextLabel(const QString &text, QWidget *parent = Q_NULLPTR,
                     Qt::WindowFlags f = Qt::WindowFlags());
  QString text() const;
  virtual QSize sizeHint() const override;
public slots:
  void setText(const QString &text);

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  QStringList string;
};
