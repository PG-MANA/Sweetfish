/*
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE
 *
 * VideoPlayer クラス
 * 動画をを再生する。Phonon依存している
 */
#include <QtWidgets>
#include "../Sweetfish.h"
#include "VideoPlayer.h"
#ifdef NO_PHONON
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QVideoWidget>
#else
#include <phonon/VideoPlayer>
#endif

VideoPlayer::VideoPlayer(TootData *tdata, QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f) {
  if (tdata == nullptr || !tdata->getMediaData().size()) return;
  media_data = tdata->getMediaData();
  QString media_url = media_data.getEntry(0).getRemoteUrl().isEmpty()
                          ? media_data.getEntry(0).getUrl()
                          : media_data.getEntry(0).getRemoteUrl();
  QVBoxLayout *main_layout = new QVBoxLayout(this);

#ifdef NO_PHONON
  video_player = new QMediaPlayer;
  QVideoWidget *video_widget = new QVideoWidget;
  video_player->setMedia(QUrl(media_url));  //先に読んでおく
  video_player->setVideoOutput(video_widget);
  main_layout->addWidget(video_widget);
#else
  video_player = new Phonon::VideoPlayer(Phonon::VideoCategory);
  video_player->load(QUrl(media_url));  //先に読んでおく
  connect(video_player, &Phonon::VideoPlayer::finished, video_player,
          static_cast<void (Phonon::VideoPlayer::*)(void)>(
              &Phonon::VideoPlayer::play));  // Loop
  main_layout->addWidget(video_player);
#endif

  //ボタン作成
  createButtons(main_layout);
  setWindowTitle(tr("動画 ") + APP_NAME);
  setAttribute(Qt::WA_DeleteOnClose);
}

/*
 * 引数:なし
 * 戻値:なし
 * 概要:表示と同時に再生する。
 */
void VideoPlayer::show() {
  QWidget::show();
  video_player->play();
}

/*
 * 引数:main_layout(格納するレイアウト)
 * 戻値:なし
 * 概要:ボタンをを作って設定する。ウィンドウ初期化用。
 */
void VideoPlayer::createButtons(QVBoxLayout *main_layout) {
  QHBoxLayout *button_layout = new QHBoxLayout;
  QPushButton *start = new QPushButton(tr("再生(&S)"));
  QPushButton *pause = new QPushButton(tr("一時停止(&P)"));
  QPushButton *back = new QPushButton(tr("最初に戻る(&B)"));  //最初に戻る
  QPushButton *close = new QPushButton(tr("閉じる(&C)"));
  start->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
  pause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
  back->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
  close->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
#ifdef NO_PHONON
  connect(start, &QPushButton::clicked, video_player, &QMediaPlayer::play);
  connect(pause, &QPushButton::clicked, video_player, &QMediaPlayer::pause);
  connect(back, &QPushButton::clicked, video_player, &QMediaPlayer::position);
#else
  connect(start, &QPushButton::clicked, video_player,
          static_cast<void (Phonon::VideoPlayer::*)(void)>(
              &Phonon::VideoPlayer::play));
  connect(pause, &QPushButton::clicked, video_player,
          &Phonon::VideoPlayer::pause);
  connect(back, &QPushButton::clicked, video_player,
          &Phonon::VideoPlayer::seek);
#endif
  connect(close, &QPushButton::clicked, this, &QWidget::close);
  button_layout->addWidget(start);
  button_layout->addWidget(pause);
  button_layout->addWidget(back);
  button_layout->addWidget(close);
  main_layout->addLayout(button_layout);
}
