// SPDX-FileCopyrightText: 2026 Marco Martin <mart@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QObject>
#include <qqmlregistration.h>

class QQuickItem;

class PagePoolActionHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit PagePoolActionHelper(QObject *parent = nullptr);

    Q_INVOKABLE QObject *stackForUseLayers(QObject *pageStack, bool useLayers) const;
    Q_INVOKABLE bool stackContainsPage(QObject *pageStack, QQuickItem *pageItem, bool useLayers) const;
    Q_INVOKABLE bool layerContainsPage(QObject *pageStack, QQuickItem *pageItem, bool useLayers) const;
    Q_INVOKABLE bool hasPushPop(QObject *stack) const;
    Q_INVOKABLE void clearLayers(QObject *pageStack) const;
    Q_INVOKABLE void popBaseOrClear(QObject *stack, QQuickItem *basePage, bool useLayers) const;
};
