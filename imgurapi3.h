/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2016-05-27
 * Description : Implementation of v3 of the Imgur API
 *
 * Copyright (C) 2016 by Fabian Vogt <fabian at ritter dash vogt dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef IMGURAPI3_H
#define IMGURAPI3_H

#include <atomic>
#include <queue>

#include <QFile>
#include <QNetworkAccessManager>
#include <QString>
#include <QUrl>

#include "koauth2.h"

enum class ImgurAPI3ActionType {
    ACCT_INFO, /* Action: account Result: account */
    IMG_UPLOAD, /* Action: upload Result: image */
    ANON_IMG_UPLOAD, /* Action: upload Result: image */
};

struct ImgurAPI3Action {
    ImgurAPI3ActionType type;
    struct {
        QString imgpath;
        QString title;
        QString description;
    } upload;

    struct {
        QString username;
    } account;
};

struct ImgurAPI3Result {
    const ImgurAPI3Action *action;
    
    struct ImgurImage
    {
        QString    name;
        QString    title;
        QString    hash;
        QString    deletehash;
        QString    url;
        QString    description;
        qulonglong datetime;
        QString    type;
        bool       animated;
        uint       width;
        uint       height;
        uint       size;
        uint       views;
        qulonglong bandwidth;
    } image;
    
    struct ImgurAccount
    {
        
    };
};

/* Main class, handles the client side of the Imgur API v3. */
class ImgurAPI3 : public QObject
{
Q_OBJECT

public:
    ImgurAPI3(const QString &client_id, const QString &client_secret, QObject *parent=nullptr);
    ~ImgurAPI3();

    /* Use this to read/write the access and refresh tokens. */
    KOAuth2 &getAuth();
    
    void setPin(const QString &pin);

    unsigned int workQueueLength();
    void queueWork(const ImgurAPI3Action &action);
    void cancelAllWork();
    
    static QUrl urlForDeletehash(const QString &deletehash);

Q_SIGNALS:
    /* Called if authentication state changes. */
    void authorized(bool success, const QString &username);
    void authError(const QString &msg);
    
    /* Open url in a browser and let the user copy the pin.
     * Call setPin(pin) to authorize. */
    void requestPin(const QUrl &url);
    
    /* Emitted on progress changes. */
    void progress(unsigned int percent, const ImgurAPI3Action &action);
    void success(const ImgurAPI3Result &result);
    void error(const QString &msg, const ImgurAPI3Action &action);

    /* Emitted when the status changes. */
    void busy(bool b);
    
public Q_SLOTS:
    /* Connected to m_auth.authenticated(bool). */
    void oauthAuthorized(bool success, const QJsonObject &response);
    /* Connected to m_auth.requestPin(QUrl). */
    void oauthRequestPin(const QUrl &url);

    /* Connected to the current QNetworkReply. */
    void uploadProgress(qint64 sent, qint64 total);
    void replyFinished();

protected:
    void timerEvent(QTimerEvent *event) override;

private:
    /* Starts m_work_timer if m_work_queue not empty. */
    void startWorkTimer();
    /* Stops m_work_timer if running. */
    void stopWorkTimer();
    
    /* Start working on the first item of m_work_queue
     * by sending a request. */
    void doWork();
    
    /* Handler for OAuth 2 related requests. */
    KOAuth2 m_auth;
    
    /* Work queue. */
    std::queue<ImgurAPI3Action> m_work_queue;
    /* ID of timer triggering on idle (0ms). */
    int m_work_timer = 0;
    
    /* Current QNetworkReply */
    QNetworkReply *m_reply = nullptr;
    
    /* Current image being uploaded */
    QFile *m_image = nullptr;
    
    /* The QNetworkAccessManager used for connections */
    QNetworkAccessManager m_net;
};

#endif // IMGURAPI3_H
