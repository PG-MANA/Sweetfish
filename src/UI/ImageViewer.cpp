/*
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE
 *
 * ImageViewer クラス
 * 画像を表示する。
 */
#include "ImageViewer.h"
#include "../Sweetfish.h"
#include "ImageLabel.h"
#include <QNetworkReply>
#include <QtWidgets>

ImageViewer::ImageViewer(TootData *tdata, unsigned int index, QWidget *parent,
                         Qt::WindowFlags f)
    : QWidget(parent, f), now_index(index), first(true) {
  if (tdata == nullptr)
    return;

  for (unsigned int cnt = 0, size = tdata->getMediaData().size(); cnt < size;
       cnt++) {
    url_list.append(tdata->getMediaData().getEntry(cnt).getUrl());
  }
  init();
}

ImageViewer::ImageViewer(QStringList _url_list, unsigned int index,
                         QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f), now_index(index), first(true), url_list(_url_list) {
  init();
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:ImageViewerの画面を構成する
 */
void ImageViewer::init() {
  QVBoxLayout *main_layout = new QVBoxLayout(this);
  QScrollArea *image_area = new QScrollArea;
  iml = new ImageLabel(0, 0, 0, nullptr);

  setWindowTitle(tr("画像の詳細 ") + APP_NAME);
  setAttribute(Qt::WA_DeleteOnClose);

  int link_size = url_list.size();
  if (link_size <= now_index)
    now_index = 0;

  //画像
  image_area->setWidgetResizable(true);
  image_area->setWidget(iml); //先に追加しておいてボタンを下に持ってくる
  main_layout->addWidget(image_area);
  //ボタン作成
  createButtons(main_layout);

  setImage(url_list.at(now_index));
  if (now_index == 0)
    back_button->setEnabled(false);
  if (now_index == link_size - 1)
    next_button->setEnabled(false);
}

/*
 * 引数:main_layout(格納するレイアウト)
 * 戻値:なし
 * 概要:ボタンをを作って設定する。ウィンドウ初期化用。
 */
void ImageViewer::createButtons(QVBoxLayout *main_layout) {
  QHBoxLayout *button_layout = new QHBoxLayout;

  next_button = new QPushButton(tr("次へ(&N)"));
  back_button = new QPushButton(tr("前へ(&B)"));
  save_button = new QPushButton(tr("名前を付けて保存(&S)"));
  QPushButton *copy_button = new QPushButton(tr("コピー(&P)"));
  QPushButton *close_button = new QPushButton(tr("閉じる(&C)"));
  //アイコン設定
  close_button->setIcon(style()->standardIcon(
      QStyle::SP_TitleBarCloseButton)); //直感的に操作できるように
  next_button->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
  save_button->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
  copy_button->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
  back_button->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
  //イベント接続
  connect(close_button, &QPushButton::clicked, this, &ImageViewer::close);
  connect(next_button, &QPushButton::clicked, this, &ImageViewer::nextImage);
  connect(back_button, &QPushButton::clicked, this, &ImageViewer::backImage);
  connect(copy_button, &QPushButton::clicked, this, &ImageViewer::copy);
  connect(save_button, &QPushButton::clicked, this, &ImageViewer::save);
  //格納
  button_layout->addWidget(back_button);
  button_layout->addWidget(next_button);
  button_layout->addWidget(save_button);
  button_layout->addWidget(copy_button);
  button_layout->addWidget(close_button);
  main_layout->addLayout(button_layout);
}

/*
 * 引数:url(表示する画像)
 * 戻値:なし
 * 概要:ImageLabelに画像をセットする。
 */
void ImageViewer::setImage(const QString &url) {
  QNetworkReply *rep = net.get(url); //:origをつけると原寸大だが時間がかかる
  connect(rep, &QNetworkReply::finished, iml, &ImageLabel::setPixmapByNetwork);
  connect(rep, &QNetworkReply::finished, this, &ImageViewer::fit);
  connect(this, &ImageViewer::destroyed, rep,
          &QNetworkReply::deleteLater); // abortも入れるべきか?
  save_button->setEnabled(false);
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:画像のセットが完了したときに呼ばれる。各種作業。
 */
void ImageViewer::fit() {
  save_button->setEnabled(true);
  if (first) {
    resize(iml->sizeHint()); //関数名の由来(たまにでかすぎることが...)
    first = false;
  }
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:次の画像を表示する。
 */
void ImageViewer::nextImage() {
  if (!save_button->isEnabled())
    return; //作業中
  now_index++;
  setImage(url_list.at(now_index));

  back_button->setEnabled(true);
  if (now_index == url_list.size() - 1)
    next_button->setEnabled(false);
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:次の画像を表示する。
 */
void ImageViewer::backImage() {
  if (!save_button->isEnabled())
    return; //作業中
  now_index--;
  setImage(url_list.at(now_index));
  next_button->setEnabled(true);
  if (now_index == 0)
    back_button->setEnabled(false);
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:ImageLabelにセットされている画像をクリップボードにコピーする。
 */
void ImageViewer::copy() {
  if (!save_button->isEnabled())
    return; //作業中
  if (const QPixmap *pixmap = iml->pixmap())
    QApplication::clipboard()->setPixmap(*pixmap);
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:ImageLabelにセットされている画像を保存する。
 */
void ImageViewer::save() {
  QString media_link = url_list.at(now_index);

  QString tempname =
      media_link.split("/")
          .constLast(); // http://doc.qt.io/qt-5/qstring.html#split に「If sep
                        // does not match anywhere in the string, split()
                        // returns a single-element list containing this
                        // string.」とあるのでsplitが返すQStringListは空であることはない...はず
  QString templocation =
      QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
  QString filter(tr("画像") +
                 "("); // stream使っても良い(翻訳しやすいようにわざと分離する)
  QByteArrayList spfilters = QImageWriter::supportedImageFormats();

  for (int cnt = spfilters.size() - 1; cnt >= 0;
       filter.append(" *." + spfilters.at(cnt)), cnt--)
    ; //順序は逆になるけどすっきりするから別にいいや
  filter.append(")");
  QString filename = QFileDialog::getSaveFileName(
      this, tr("名前を付けて保存"), templocation + "/" + tempname, filter);
  if (filename.isEmpty() || !iml->pixmap()->save(filename))
    QMessageBox::critical(this, tr("画像の詳細 ") + APP_NAME,
                          tr("画像の保存に失敗しました。"));
}
