// SPDX-FileCopyrightText: 2026 Marco Martin <mart@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "pagepoolactionhelper.h"

#include <QMetaObject>
#include <QQuickItem>

namespace
{
QObject *stackLayers(QObject *pageStack)
{
    if (!pageStack) {
        return nullptr;
    }
    return pageStack->property("layers").value<QObject *>();
}
}

PagePoolActionHelper::PagePoolActionHelper(QObject *parent)
    : QObject(parent)
{
}

QObject *PagePoolActionHelper::stackForUseLayers(QObject *pageStack, bool useLayers) const
{
    if (!useLayers) {
        return pageStack;
    }
    return stackLayers(pageStack);
}

bool PagePoolActionHelper::stackContainsPage(QObject *pageStack, QQuickItem *pageItem, bool useLayers) const
{
    if (!pageStack || !pageItem || useLayers) {
        return false;
    }

    QObject *columnView = pageStack->property("columnView").value<QObject *>();
    if (!columnView) {
        return false;
    }

    bool contains = false;
    QMetaObject::invokeMethod(columnView, "containsItem", Q_RETURN_ARG(bool, contains), Q_ARG(QQuickItem *, pageItem));
    return contains;
}

bool PagePoolActionHelper::layerContainsPage(QObject *pageStack, QQuickItem *pageItem, bool useLayers) const
{
    if (!pageStack || !pageItem || !useLayers) {
        return false;
    }
    QObject *layers = stackLayers(pageStack);
    if (!layers) {
        return false;
    }

    int depth = layers->property("depth").toInt();
    for (int i = 0; i < depth; ++i) {
        QObject *candidate = nullptr;
        QMetaObject::invokeMethod(layers, "get", Q_RETURN_ARG(QObject *, candidate), Q_ARG(int, i));
        if (candidate == pageItem) {
            return true;
        }
    }
    return false;
}

bool PagePoolActionHelper::hasPushPop(QObject *stack) const
{
    if (!stack) {
        return false;
    }
    const auto *metaObject = stack->metaObject();
    const bool hasPush = metaObject->indexOfMethod("push(QVariant,QVariant)") != -1 || metaObject->indexOfMethod("pushItem(QQuickItem*,QVariantMap)") != -1;
    const bool hasPop = metaObject->indexOfMethod("pop(QVariant)") != -1 || metaObject->indexOfMethod("popCurrentItem()") != -1;
    return hasPush && hasPop;
}

void PagePoolActionHelper::clearLayers(QObject *pageStack) const
{
    QObject *layers = stackLayers(pageStack);
    if (layers) {
        QMetaObject::invokeMethod(layers, "clear");
    }
}

void PagePoolActionHelper::popBaseOrClear(QObject *stack, QQuickItem *basePage, bool useLayers) const
{
    if (!stack) {
        return;
    }

    if (basePage) {
        if (!QMetaObject::invokeMethod(stack, "pop", Q_ARG(QVariant, QVariant::fromValue(basePage)))) {
            QMetaObject::invokeMethod(stack, "popToItem", Q_ARG(QQuickItem *, basePage));
        }
    } else if (!useLayers) {
        QMetaObject::invokeMethod(stack, "clear");
    }
}

#include "moc_pagepoolactionhelper.cpp"
