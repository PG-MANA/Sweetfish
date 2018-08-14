/*
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE
 *
 * VideoPlayer クラス
 * 動画をを再生する。Phonon依存している
 */
#include "../Mastodon/Mastodon.h"
#include "../Mastodon/TootData.h"
#include "../Network/Network.h"
#include <QWidget>

#ifdef USE_MULTIMEDIA
class QMediaPlayer;
#else
namespace Phonon {
class VideoPlayer;
}
#endif

class QVBoxLayout;
class QPushButton;

class VideoPlayer : public QWidget {
  Q_OBJECT
public:
  explicit VideoPlayer(TootData *twdata, QWidget *parent = Q_NULLPTR,
                       Qt::WindowFlags f = Qt::WindowFlags());

public slots:
  void show();
  void save(){}; //未実装
private:
  void createButtons(QVBoxLayout *main_layout);
  TootMediaData media_data;
#ifdef USE_MULTIMEDIA
  QMediaPlayer *video_player;
#else
  Phonon::VideoPlayer *video_player;
#endif
};
