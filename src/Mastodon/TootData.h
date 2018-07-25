/*
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE
 */
#pragma once
#include <QDateTime>
#include <QJsonObject>
#include <QString>

class TootAccountData {
public:
  TootAccountData(){};
  explicit TootAccountData(const QJsonObject &target);

  bool isEmpty() const { return id.isEmpty(); };
  bool isLocked() const { return locked; };

  // getter
  QByteArray getId() const { return id; };
  QString getUserName() const { return user_name; };
  QString getDisplayName() const { return display_name; };
  QString getAcct() const { return acct; };
  QString getAvatar() const { return avatar; };

private:
  QByteArray id;
  QString user_name; //スクリーンネーム(ABC)
  QString acct;      //インスタンス名を含むユーザ名
  QString display_name;
  QString avatar; //ユーザアイコン
  bool locked;
  // bool following;
};

class TootMediaDataEntry {
public:
  // getter
  QString getType() const { return type; };
  QString getUrl() const { return url; };
  QString getRemoteUrl() const { return remote_url; };
  QString getPreviewUrl() const { return preview_url; };
  QString getTextUrl() const { return text_url; };

private:
  QString type;
  QString url;
  QString remote_url;
  QString preview_url;
  QString text_url;

  friend class TootMediaData;
};

class TootMediaData {
public:
  TootMediaData(){};
  explicit TootMediaData(const QJsonArray &target);
  unsigned int size() const { return media_list.size(); };
  TootMediaDataEntry getEntry(unsigned int index) const {
    return media_list.at(index);
  };

private:
  QVector<TootMediaDataEntry> media_list;
};

class TootUrlData {
public:
  TootUrlData(){};
  void addUrlPair(const QString &display_url, const QString &full_url);
  unsigned int count() const { return data.count(); };
  unsigned int size() const { return count(); };
  QString getDisplayUrl(unsigned int index) const;
  QString getFullUrl(unsigned int index) const;

private:
  QList<QPair<QString, QString>> data;
};

class TootData {
public:
  TootData(){};
  explicit TootData(const QJsonObject &target,
                    const QByteArray &my_user_id = QByteArray());
  virtual ~TootData();

  bool isEmpty() const { return id.isEmpty(); };
  bool isBoosted() const;
  bool isFavourited() const;
  bool isTootOwner() const;

  // getter
  QByteArray getId() const { return id; };
  QString getApplicationName() const;
  QString getApplicationSite() const;
  const TootAccountData &getAccountData() const { return account; };
  const TootAccountData &getOriginalAccountData() const;
  TootData *getBoostedData() const { return reblog; };
  const TootUrlData &getUrlData() const { return url_list; };
  const TootMediaData &getMediaData() const { return media; };
  QDateTime getDateTime() const { return created_at; };
  QString getContent() const { return content; };

private:
  QByteArray id;
  QDateTime created_at;
  QString uri;
  QString url;
  QString content;
  QPair<QString, QString> application; // via
  unsigned flag;

  TootAccountData account;
  TootUrlData url_list;
  TootMediaData media;
  TootData *reblog = nullptr;

  void analyzeContent(QString content);
};

class TootNotificationData {
public:
  enum Event { //使いそうなやつだけ
    NoEvent,
    Mention,
    Boost,
    Favourite,
    // Unfavorite,
    // QuotedTweet,//引用ツイート
    Follow,
    // Unfollow,
    // ListCreated,
    // ListDestroyed,
    // ListUpdated,
    // ListMemberAdded,
    // ListMemberRemoved
  };
  TootNotificationData(){};
  explicit TootNotificationData(const QJsonObject &target);

  bool isEmpty() const { return (type == Event::NoEvent); };

  // getter
  Event getType() const { return type; };
  const TootAccountData &getAccount() const { return account; };
  const TootData &getStatus() const { return status; };

private:
  TootAccountData account;
  TootData status;
  Event type;
};
