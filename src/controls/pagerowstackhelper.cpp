// SPDX-FileCopyrightText: 2026 Marco Martin <mart@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "pagerowstackhelper.h"

#include "loggingcategory.h"

#include <QMetaMethod>
#include <QMetaProperty>
#include <QQuickItem>
#include <QQmlContext>
#include <QQmlEngine>

PageRowStackHelper::PageRowStackHelper(QObject *parent)
    : QObject(parent)
{
}

PageRowStackHelper::~PageRowStackHelper()
{
    clearCache();
}

bool PageRowStackHelper::verifyPages(const QVariant &pages, const QVariant &properties, QObject *columnView)
{
    clearLastError();

    const QVariantList pagesList = pages.toList();
    bool pagesValid = isValidPage(pages, columnView);
    if (!pagesValid && !pagesList.isEmpty()) {
        pagesValid = true;
        for (const QVariant &page : pagesList) {
            if (!isValidPage(page, columnView)) {
                pagesValid = false;
                break;
            }
        }
    }

    if (!pagesValid) {
        return false;
    }

    if (!properties.isValid() || properties.isNull()) {
        return true;
    }

    const QVariantList propertiesList = properties.toList();
    if (!propertiesList.isEmpty()) {
        if (!pagesList.isEmpty() && pagesList.size() != propertiesList.size()) {
            return false;
        }
        for (const QVariant &property : propertiesList) {
            if (!property.canConvert<QVariantMap>()) {
                return false;
            }
        }
        return true;
    }

    return properties.canConvert<QVariantMap>();
}

QObject *PageRowStackHelper::initPage(const QVariant &page, const QVariant &properties, QObject *contextObject)
{
    clearLastError();

    QVariantMap initialProperties = propertiesMap(properties);
    QObject *pageObject = page.value<QObject *>();

    if (QQmlComponent *component = componentForPage(page, contextObject)) {
        if (!contextObject) {
            setLastError(QStringLiteral("Missing QML context object while creating page"));
            return nullptr;
        }
        QQmlContext *context = qmlContext(contextObject);
        if (!context) {
            setLastError(QStringLiteral("Failed to resolve QML context while creating page"));
            return nullptr;
        }

        pageObject = component->createWithInitialProperties(initialProperties, context);
        if (!pageObject || component->isError()) {
            setLastError(component->errorString());
            if (pageObject) {
                pageObject->deleteLater();
            }
            return nullptr;
        }
    } else if (pageObject) {
        const QMetaObject *metaObject = pageObject->metaObject();
        for (auto it = initialProperties.cbegin(); it != initialProperties.cend(); ++it) {
            if (metaObject->indexOfProperty(it.key().toUtf8().constData()) != -1) {
                pageObject->setProperty(it.key().toUtf8().constData(), it.value());
            }
        }
    }

    return pageObject;
}

void PageRowStackHelper::clearCache()
{
    for (auto it = m_componentCache.begin(); it != m_componentCache.end(); ++it) {
        if (it.value()) {
            it.value()->deleteLater();
        }
    }
    m_componentCache.clear();
}

QString PageRowStackHelper::lastError() const
{
    return m_lastError;
}

bool PageRowStackHelper::isValidPage(const QVariant &page, QObject *columnView) const
{
    if (QQmlComponent *component = page.value<QQmlComponent *>()) {
        Q_UNUSED(component);
        return true;
    }

    if (QQuickItem *item = toQuickItem(page)) {
        if (item->inherits("QQuickPage") && containsItem(columnView, item)) {
            return false;
        }
        return item->inherits("QQuickPage");
    }

    if (page.canConvert<QString>()) {
        return !page.toString().isEmpty();
    }

    if (page.canConvert<QUrl>()) {
        return !page.toUrl().toString().isEmpty();
    }

    return false;
}

bool PageRowStackHelper::containsItem(QObject *columnView, QQuickItem *item) const
{
    if (!columnView || !item) {
        return false;
    }

    bool contains = false;
    QMetaObject::invokeMethod(columnView, "containsItem", Q_RETURN_ARG(bool, contains), Q_ARG(QQuickItem *, item));
    return contains;
}

QQuickItem *PageRowStackHelper::toQuickItem(const QVariant &value) const
{
    if (!value.canConvert<QObject *>()) {
        return nullptr;
    }
    return qobject_cast<QQuickItem *>(value.value<QObject *>());
}

QQmlComponent *PageRowStackHelper::componentForPage(const QVariant &page, QObject *contextObject)
{
    if (QQmlComponent *component = page.value<QQmlComponent *>()) {
        return component;
    }

    if (!contextObject) {
        return nullptr;
    }

    QUrl pageUrl;
    if (page.canConvert<QString>()) {
        pageUrl = QUrl(page.toString());
    } else if (page.canConvert<QUrl>()) {
        pageUrl = page.toUrl();
    } else {
        return nullptr;
    }

    QQmlContext *context = qmlContext(contextObject);
    QQmlEngine *engine = qmlEngine(contextObject);
    if (!context || !engine) {
        return nullptr;
    }

    if (pageUrl.scheme().isEmpty()) {
        pageUrl = context->resolvedUrl(pageUrl);
    }

    const auto cacheIt = m_componentCache.constFind(pageUrl);
    if (cacheIt != m_componentCache.constEnd() && !cacheIt.value().isNull()) {
        return cacheIt.value();
    }

    auto *component = new QQmlComponent(engine, pageUrl, QQmlComponent::PreferSynchronous, this);
    m_componentCache.insert(pageUrl, component);
    return component;
}

QVariantMap PageRowStackHelper::propertiesMap(const QVariant &properties) const
{
    if (properties.canConvert<QVariantMap>()) {
        return properties.toMap();
    }
    return {};
}

void PageRowStackHelper::setLastError(const QString &error)
{
    if (m_lastError == error) {
        return;
    }
    m_lastError = error;
    Q_EMIT lastErrorChanged();
}

void PageRowStackHelper::clearLastError()
{
    if (m_lastError.isEmpty()) {
        return;
    }
    m_lastError.clear();
    Q_EMIT lastErrorChanged();
}

#include "moc_pagerowstackhelper.cpp"
