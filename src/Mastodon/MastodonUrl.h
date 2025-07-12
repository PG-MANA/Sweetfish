/*
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE
 */
#pragma once

#include <QString>

// classではないほうがいいかと...
namespace MastodonUrl {
static const QString scheme("https://");
// 認証関係
static const QString register_app("/api/v1/apps");
static const QString authorize("/oauth/authorize/?");
static const QString token("/oauth/token");
// TimeLine関係
static const QString home_timeline("/api/v1/timelines/home");
static const QString list_timeline("/api/v1/timelines/list/");

// ストリーム関係
//  static const QString local_stream("/api/v1/streaming/public/local");
static const QString user_stream("/api/v1/streaming/user");
static const QString list_stream("/api/v1/streaming/list?list=");

// トゥート関係
static const QString statuses("/api/v1/statuses");
static const QString reblog("/reblog");       // statusesとidを組み合わせる。
static const QString favourite("/favourite"); // statusesとidを組み合わせる。
// お気に入り関係

// アップロード関係
static const QString media_upload("/api/v1/media");

// リスト関係
static const QString lists("/api/v1/lists");
// ユーザ関係
static const QString current_account("/api/v1/accounts/verify_credentials");
static const QString accounts("/api/v1/accounts/");
static const QString accounts_status("/statuses");
static const QString accounts_relationship("/api/v1/accounts/relationships");
static const QString accounts_follow("/follow");
static const QString accounts_unfollow("/unfollow");
static const QString accounts_block("/block");
static const QString accounts_unblock("/unblock");

} // namespace MastodonUrl
