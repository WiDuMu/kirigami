// SPDX-FileCopyrightText: 2026 Marco Martin <mart@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QObject>
#include <QPointer>
#include <QQmlComponent>
#include <QUrl>
#include <qqmlregistration.h>

class QQuickItem;

class PageRowStackHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit PageRowStackHelper(QObject *parent = nullptr);
    ~PageRowStackHelper() override;

    Q_INVOKABLE bool verifyPages(const QVariant &pages, const QVariant &properties, QObject *columnView);
    Q_INVOKABLE QObject *initPage(const QVariant &page, const QVariant &properties, QObject *contextObject);
    Q_INVOKABLE void clearCache();

    QString lastError() const;

Q_SIGNALS:
    void lastErrorChanged();

private:
    bool isValidPage(const QVariant &page, QObject *columnView) const;
    bool containsItem(QObject *columnView, QQuickItem *item) const;
    QQuickItem *toQuickItem(const QVariant &value) const;
    QQmlComponent *componentForPage(const QVariant &page, QObject *contextObject);
    QVariantMap propertiesMap(const QVariant &properties) const;
    void setLastError(const QString &error);
    void clearLastError();

    mutable QHash<QUrl, QPointer<QQmlComponent>> m_componentCache;
    QString m_lastError;
};
