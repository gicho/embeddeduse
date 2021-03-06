// Copyright (C) 2018 Burkhard Stubert (DBA EmbeddedUse)

#include <QtDebug>
#include "Model.h"

//#define PROBLEM_FIXED

class Model::Impl : public QObject
{
    Q_OBJECT

public:
    Impl(Model *parent);
    ~Impl();
    QString infoText() const;
    void setInfoText(const QString &text);
    Model *m_iface;
    QString m_infoText;
};

Model::Impl::Impl(Model *parent) :
    QObject(parent),
    m_iface(parent),
    m_infoText{QStringLiteral("Waiting...")}
{
    qDebug() << __PRETTY_FUNCTION__;
#ifndef PROBLEM_FIXED
    setInfoText("Constructor: Oooops!!!");
#endif
}

Model::Impl::~Impl()
{
    qDebug() << __PRETTY_FUNCTION__;
#ifndef PROBLEM_FIXED
    setInfoText("Destructor: Oooops!!!");
#endif
}

QString Model::Impl::infoText() const
{
    return m_infoText;
}

void Model::Impl::setInfoText(const QString &text)
{
    if (m_infoText != text) {
        qDebug() << __PRETTY_FUNCTION__;
        m_infoText = text;
        emit m_iface->infoTextChanged();
    }
}

Model::Model(QObject *parent) :
    QObject{parent},
    m_impl{new Impl{this}}
{
    qDebug() << __PRETTY_FUNCTION__;
#ifdef PROBLEM_FIXED
    setInfoText("Constructor: All fine!!!");
#endif
}

Model::~Model()
{
    qDebug() << __PRETTY_FUNCTION__;
#ifdef PROBLEM_FIXED
    setInfoText("Destructor: All fine!!!");
#endif
}

QString Model::infoText() const
{
    return m_impl->infoText();
}

void Model::setInfoText(const QString &text)
{
    qDebug() << __PRETTY_FUNCTION__;
    m_impl->setInfoText(text);
}

#include "Model.moc"
