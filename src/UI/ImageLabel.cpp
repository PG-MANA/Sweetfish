/*
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE
 *
 * ImageLabel クラス
 * これを応用すれば画像や動画のラベルも作れる。
 */
#include "ImageLabel.h"
#include "TootContent.h"
#include <QHash>
#include <QMouseEvent>
#include <QNetworkReply>

QHash<QString, QPixmap> ImageLabel::images;

ImageLabel::ImageLabel(const unsigned int init_sizex,
                       const unsigned int init_sizey,
                       const unsigned int init_index, QString init_url,
                       QWidget *parent, Qt::WindowFlags f)
    : QLabel(parent, f), index(init_index), sizex(init_sizex),
      sizey(init_sizey), url(init_url) {}

/*
 * 引数:target_url(画像の名前)
 * 戻値:キャッシュから設定できた場合true、できなかった場合はfalse
 * 概要:渡されたURLでキャッシュを探し見つかった場合はそこから設定する。
 */
bool ImageLabel::setPixmapByName(const QString &target_url) {
  url = target_url;
  const QHash<QString, QPixmap>::iterator image = images.find(url);
  if (image == images.end()) {
    if (images.size() > 256)
      images.clear(); // ここらで一回全部消してメモリの使用を減らす
    return false;
  }
  setPixmap(image.value());
  return true;
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:QNetworkReplyのfinishedによって呼ばれる。画像の設定、キャッシュへ登録を行う。
 */
void ImageLabel::setPixmapByNetwork() {
  auto *rep = qobject_cast<QNetworkReply *>(sender());
  QPixmap p;

  rep->deleteLater(); // returnしたあとに削除される
  if (rep->error() != QNetworkReply::NoError || !p.loadFromData(rep->readAll()))
    return;
  if (sizex && sizey) {
    p = p.scaled(sizex, sizey, Qt::KeepAspectRatio); // 縮小
    images[url] = p;
  }
  setPixmap(p);
}

/*
 * 引数:new_sizex, new_sizey(新しいsize)
 * 戻値:なし
 * 概要:縮小サイズを更新するときに使う。
 */
void ImageLabel::setSize(unsigned int new_sizex, unsigned int new_sizey) {
  sizex = new_sizex;
  sizey = new_sizey;
}

/*
 * 引数:ref_sizex, ref_sizey(結果受け取り用)
 * 戻値:なし
 * 概要:縮小サイズを取得するときに使う。
 */
void ImageLabel::getSize(unsigned int &ref_sizex,
                         unsigned int &ref_sizey) const {
  ref_sizex = sizex;
  ref_sizey = sizey;
}

/*
 * 引数:なし
 * 戻値:index(画像が何番目か)
 * 概要:Layoutなどで画像が何番目かを返す。
 */
unsigned int ImageLabel::getIndex() const { return index; }

/*
 * 引数:index(画像が何番目か)
 * 戻値:なし
 * 概要:Layoutなどで画像が何番目かを変更する。
 */
void ImageLabel::setIndex(unsigned int i) { index = i; }

/*
 * 引数:event
 * 戻値:なし
 * 概要:クリックされたときにシグナルを発信する。
 * 備考:ダブルクリックはmouseDoubleClickEventになる。
 */
void ImageLabel::mousePressEvent(QMouseEvent *event) {
  switch (event->button()) {
  case Qt::LeftButton:
    emit clicked(index);
    break;
  case Qt::RightButton:
    emit rightClicked(index);
    break;
  default:
    event->ignore();
    return;
  }
  event->accept();
}
