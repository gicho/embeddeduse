#ifndef APPLICATIONMANAGER_H
#define APPLICATIONMANAGER_H

#include <QAbstractListModel>
#include <QByteArray>
#include <QHash>
#include <QModelIndex>
#include <QString>
#include <QVariant>
#include <QVector>
class QProcess;
class QQuickItem;

class ApplicationManager : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role {
        ROLE_COLOR = Qt::UserRole,
        ROLE_RUNNING,
        ROLE_PROCESS_ID,
        ROLE_HOME,
        ROLE_APPLICATION_ITEM
    };
    explicit ApplicationManager(QObject *parent = nullptr);
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent = QModelIndex{}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

public slots:
    bool startApplication(int row);
    // The ownership of item stays with the QML engine.
    void insertApplicationItem(int processId, QQuickItem *item);

private:
    struct AppInfo {
        QString m_color;
        QProcess *m_process;
        QQuickItem *m_item;
    };
    int indexOfProcess(int processId) const;
    QVector<AppInfo> m_appInfoColl;
};

#endif // APPLICATIONMANAGER_H