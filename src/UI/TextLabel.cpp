/*
 * Copyright 2017 PG_MANA
 *
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE.md
 *
 * TextLabel クラス
 * どこでも折り返せるラベル。
 */
#include <QPainter>
#include <QTextLayout>
#include <QTextOption>
#include "TextLabel.h"

TextLabel::TextLabel(const QString &text, QWidget *parent, Qt::WindowFlags f)
    : QFrame(parent, f), string(text.split('\n')) {}

/*
 * 引数:event
 * 戻値:なし
 * 概要:文字を表示するときに呼ばれる。
 */
void TextLabel::paintEvent(QPaintEvent *event) {
  QFrame::paintEvent(event);

  QPainter paint(this);
  int leading = paint.fontMetrics().leading(), w = width(), total_height = 0;
  QTextOption text_option;

  paint.setRenderHint(QPainter::TextAntialiasing);
  text_option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
  // TextOption.setWrapMode(QTextOption::WrapAnywhere);

  for (QString &line : string) {
    QTextLayout Layout(line, paint.font());
    Layout.setTextOption(text_option);
    Layout.setCacheEnabled(true);
    Layout.beginLayout();
    for (unsigned int h = 0;;) {
      QTextLine Line = Layout.createLine();
      if (!Line.isValid()) {
        Layout.endLayout();
        Layout.draw(&paint, QPoint(0, total_height));
        total_height += h;
        break;
      }
      Line.setLineWidth(w);
      h += leading;
      Line.setPosition(QPoint(0, h));
      h += Line.height();
    }
  }
  setFixedHeight(total_height);  //これ肝
  return;
}

/*
 * 引数:なし
 * 戻値:sizeHint
 * 概要:これがないと表示されないときがある。
 */
QSize TextLabel::sizeHint() const { return QSize(width(), height()); }

/*
 * 引数:text
 * 戻値:なし
 * 概要:文字列をセットして描画し直す。
 */
void TextLabel::setText(const QString &text) {
  string = text.split('\n');
  update();
}

/*
 * 引数:なし
 * 戻値:text(表示されてる文字列)
 * 概要:表示されてる文字列を返す。
 */
QString TextLabel::text() const {
  QString result;
  for (const QString &line : string) result += line;
  return result;
}
