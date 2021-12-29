#ifndef VOLUMES_MODEL_H
#define VOLUMES_MODEL_H

#include "json_model.h"

class VolumesModel : public JSONModel
{
    Q_OBJECT
public:
    explicit VolumesModel(QObject *parent = nullptr);
    virtual ~VolumesModel();
    // receive a valid json with a list of volumes
    void load(const QString &json) override;

    // QAbstractItemModel methods
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    QString getVolumeId(const QModelIndex &index) const;
    int getNumIssues(const QModelIndex &index) const;
    QString getPublisher(const QModelIndex &index) const;
    QString getCoverURL(const QModelIndex &index) const;

signals:

public slots:

private:
    QList<QList<QString>> _data;

public:
    enum Column {
        SERIES = 0,
        YEAR,
        ISSUES,
        PUBLISHER,
        COVER_URL,
        DECK,
        ID
    };

    enum Role {
        SORT_ROLE = Qt::UserRole
    };
};

#endif // VOLUMES_MODEL_H
