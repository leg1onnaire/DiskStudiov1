#ifndef DISKVISUALIZER_H
#define DISKVISUALIZER_H

#include <QWidget>
#include <QVector>
#include "diskutils.h"

class DiskVisualizer : public QWidget
{
    Q_OBJECT
public:
    explicit DiskVisualizer(QWidget *parent = nullptr);
    void setPartitions(const QVector<PartitionInfo> &parts, quint64 totalSectors);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<PartitionInfo> m_partitions;
    quint64 m_totalSectors;
};

#endif // DISKVISUALIZER_H
