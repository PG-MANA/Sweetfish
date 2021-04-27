/*
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE
 *
 * TootInfo クラス
 * MainWindowのしたにある画像添付したりリプライしたりするときに出てくるやつ。
 */
#include "TootInfo.h"
#include "../Mastodon/TootData.h"
#include "ImageLabel.h"
#include "MainWindow.h"
#include "TootContent.h"
#include <QtWidgets>

TootInfo::TootInfo(MainWindow *parent_window, QWidget *parent,
                   Qt::WindowFlags f)
    : QWidget(parent, f), win(parent_window) {
  main_layout = new QVBoxLayout(this);
  media_layout = new QHBoxLayout;

  QLabel *info_text_label = new QLabel;
  info_text_label->setPixmap(QPixmap(":/add.png"));
  info_text_label->setVisible(false);

  media_layout->addWidget(info_text_label);
  media_layout->addStretch(); //こっちの方が見た目がいいかな
  main_layout->addLayout(media_layout);

  reply_layout = new QHBoxLayout;
  info_text_label = new QLabel;
  info_text_label->setPixmap(QPixmap(":/rp.png"));
  info_text_label->setVisible(false);
  reply_layout->addWidget(info_text_label);
  main_layout->addLayout(reply_layout);

  quote_layout = new QHBoxLayout;
  info_text_label = new QLabel;
  info_text_label->setPixmap(QPixmap(":/bst.png"));
  info_text_label->setVisible(false);
  quote_layout->addWidget(info_text_label);
  main_layout->addLayout(quote_layout);
}

TootInfo::~TootInfo() {}

/*
 * 引数:index(0から始まる数で何番目のImageLabelか指定)
 * 戻値:QPixmap
 * 概要:ImageLabelのPixmapをとってくる。
 */
QPixmap TootInfo::getImage(const unsigned int index) const {
  if (getNumOfImage() <= index)
    return QPixmap();
  QLayoutItem *item = media_layout->itemAt(index + 2 /*QLabel + addStretch分*/);
  return item ? (qobject_cast<ImageLabel *>(item->widget()))->pixmap()
              : QPixmap();
}

/*
 * 引数:index(0から始まる数で何番目のImageLabelか指定)
 * 戻値:QPixmap
 * 概要:保存されている添付ファイルのファイルパスをとってくる。クリップボードからの画像など存在しない場合もある
 */
QString TootInfo::getImagePath(const unsigned int index) const {
  if (getNumOfImage() <= index)
    return QString();
  return media_file_path_list.at(index);
}

/*
 * 引数:pixmap,
 * index(0から始まる数で何番目のImageLabelか指定、すでにある場合は置き換える。)
 * 戻値:なし
 * 概要:Pixmapを追加し、必要であればImagelabelを生成する。
 */
void TootInfo::setImage(const QPixmap &pixmap, QString file_path,
                        const unsigned int index) {
  unsigned int num_of_img = getNumOfImage();
  if (num_of_img == 0)
    media_layout->itemAt(0)->widget()->setVisible(true);
  if (num_of_img <= index) {
    ImageLabel *iml = new ImageLabel(50, 50, num_of_img);
    connect(iml, &ImageLabel::rightClicked, this, &TootInfo::showImageMenu);
    iml->setPixmap(pixmap);
    iml->setFixedSize(50, 50);
    media_layout->addWidget(iml);
    if (media_file_path_list.size() > index) {
      media_file_path_list[index] = file_path;
    } else {
      media_file_path_list.append(file_path);
    }
  } else {
    QLayoutItem *item =
        media_layout->itemAt(index + 2 /*QLabel + addStretch分*/);
    if (item != nullptr) {
      (qobject_cast<ImageLabel *>(item->widget()))->setPixmap(pixmap);
      media_file_path_list.replace(index, file_path);
    }
  }
}

/*
 * 引数:_tdata(シグナルの関係上受け取ってるだけ),
 * index(0から始まる数で何番目のImageLabelか指定) 戻値:なし
 * 概要:画像を削除するメニューをだす。
 */
void TootInfo::showImageMenu(unsigned int index) {
  if (ImageLabel *label = qobject_cast<ImageLabel *>(sender())) {
    QMenu *popup = new QMenu(tr("操作"), this);
    popup->setAttribute(Qt::WA_DeleteOnClose);
    popup
        ->addAction(style()->standardIcon(QStyle::SP_TrashIcon),
                    tr("Delete(&D)"), this,
                    [this] {
                      if (QAction *action = qobject_cast<QAction *>(sender()))
                        deleteImage(action->data().toUInt());
                    })
        ->setData(index);
    popup->popup(QCursor::pos());
  }
}

/*
 * 引数:index(0から始まる数で何番目のImageLabelか指定)
 * 戻値:なし
 * 概要:Pixmapを削除する。
 */
void TootInfo::deleteImage(const unsigned int index) {
  if (getNumOfImage() <= index)
    return;
  QLayoutItem *old = media_layout->takeAt(
      index +
      2 /*QLabel + addStretch分*/); //一番最初に追加したやつから番号が振られる。
  if (old != nullptr)
    delete old->widget();
  if (getNumOfImage() == 0)
    media_layout->itemAt(0)->widget()->setVisible(false);
  else {
    for (int cnt = 0; QLayoutItem *item = media_layout->itemAt(index + 2 + cnt);
         cnt++) {
      if (ImageLabel *label = qobject_cast<ImageLabel *>(item->widget())) {
        media_file_path_list[index + cnt] =
            media_file_path_list[label->getIndex()];
        label->setIndex(index + cnt);
      }
    }
  }
  closeWhenEmpty();
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:すべてのPixmapを削除する。
 */
void TootInfo::deleteImageAll() {
  unsigned int i;
  if ((i = getNumOfImage()))
    for (i--;; i--) {
      deleteImage(i); // i==0のときも作業しないといけない
      if (!i)
        break;
    }
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:Imagelabel(Pixmap)の数を返す。
 */
unsigned int TootInfo::getNumOfImage() const {
  return media_layout->count() - 2; // QLabel + addStretch分
}

/*
 * 引数:なし
 * 戻値:TootData(対象のTootData)
 * 概要:引用するトゥートのTootDataを返す。
 */
TootData *TootInfo::getQuoteToot() {
  QLayoutItem *item = quote_layout->itemAt(1);
  return (item != nullptr)
             ? qobject_cast<TootContent *>(item->widget())->getTootData()
             : nullptr;
}

/*
 * 引数:対象のTootData
 * 戻値:なし
 * 概要:トゥートの引用表示を行う。
 */
void TootInfo::setQuoteToot(TootContent *data) {
  if (data == nullptr)
    return;
  if (quote_layout->count() > 1)
    deleteQuoteToot();
  data->setFrameShape(QFrame::StyledPanel);
  data->setFrameShadow(QFrame::Sunken);
  quote_layout->addWidget(data);
  connect(data, &TootContent::action, this,
          &TootInfo::deleteQuoteToot); //現在は削除以外ないのでこの実装
  quote_layout->itemAt(0)->widget()->setVisible(true);
}

/*
 * 引数:対象のTootData
 * 戻値:なし
 * 概要:トゥートの引用表示の削除を行う。
 */
void TootInfo::deleteQuoteToot() {
  QLayoutItem *old = quote_layout->takeAt(1);
  if (old != nullptr)
    old->widget()
        ->deleteLater(); // TootContentからのSignal時にクラッシュしないように
  quote_layout->itemAt(0)->widget()->setVisible(false);
  closeWhenEmpty();
  return;
}

/*
 * 引数:なし
 * 戻値:対象のTootData
 * 概要:返信するトゥートのTootDataを返す。
 */
TootData *TootInfo::getReplyToot() {
  QLayoutItem *item = reply_layout->itemAt(1);
  return (item != nullptr)
             ? qobject_cast<TootContent *>(item->widget())->getTootData()
             : nullptr;
}

/*
 * 引数:対象のTootData
 * 戻値:なし
 * 概要:トゥートの返信表示を行う。
 */
void TootInfo::setReplyToot(TootContent *data) {
  if (data == nullptr)
    return;
  if (reply_layout->count() > 1)
    deleteReplyToot();
  data->setFrameShape(QFrame::StyledPanel);
  data->setFrameShadow(QFrame::Sunken);
  reply_layout->addWidget(data);
  connect(data, &TootContent::action, this,
          &TootInfo::deleteReplyToot); //現在は削除以外ないのでこの実装
  reply_layout->itemAt(0)->widget()->setVisible(true);
  return;
}

/*
 * 引数:対象のTootData
 * 戻値:なし
 * 概要:トゥートの返信表示の削除を行う。
 */
void TootInfo::deleteReplyToot() {
  QLayoutItem *old = reply_layout->takeAt(1);
  if (old != nullptr)
    old->widget()
        ->deleteLater(); // TootContentからのSignal時にクラッシュしないように
  reply_layout->itemAt(0)->widget()->setVisible(false);
  closeWhenEmpty();
  return;
}

/*
 * 引数:なし
 * 戻値:何もセットされてない場合true、それ以外はfalse
 * 概要:画像などがセットされてるか返す。
 */
bool TootInfo::isEmpty() {
  return !(reply_layout->itemAt(0)->widget()->isVisible() ||
           quote_layout->itemAt(0)->widget()->isVisible() ||
           media_layout->itemAt(0)->widget()->isVisible());
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:何も中身がないとき非表示にする。
 */
void TootInfo::closeWhenEmpty() {
  if (isEmpty())
    parentWidget()->parentWidget()->setVisible(false);
}
