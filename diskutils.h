#ifndef DISKUTILS_H
#define DISKUTILS_H

#include <QString>
#include <QStringList>
#include <QVector>
#include <QDir>
#include <QProcess>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

struct PartitionInfo {
    int id;
    quint64 startLBA;
    quint64 endLBA;
    double sizeGB;
    double usedGB;
    QString fsType;
    QString devicePath;
    QString mountPoint;
    bool isMounted;
};

class DiskUtils {
public:
    static QStringList getAvailableDisks() {
        QStringList disks;
        QDir dir("/sys/block");
        QStringList entries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString &entry : entries) {
            if (entry.startsWith("loop") || entry.startsWith("ram") || entry.startsWith("sr")) continue;
            disks.append("/dev/" + entry);
        }
        return disks;
    }

    static quint64 getDiskSizeBytes(const QString &diskPath) {
        QProcess process;
        // -b: Byte, -n: No Header, -d: Device Only (partisyonları gösterme), -o SIZE
        process.start("lsblk", QStringList() << "-b" << "-n" << "-d" << "-o" << "SIZE" << diskPath);
        process.waitForFinished();
        QString output = process.readAllStandardOutput().trimmed();
        if (output.isEmpty()) return 0;
        return output.toULongLong();
    }

    static QVector<PartitionInfo> readPartitions(const QString &diskPath, quint64 *totalSectors = nullptr) {
        QVector<PartitionInfo> partitions;


        QProcess sizeProc;
        sizeProc.start("lsblk", QStringList() << "-b" << "-n" << "-d" << "-o" << "SIZE" << diskPath);
        sizeProc.waitForFinished();
        QString sizeStr = sizeProc.readAllStandardOutput().trimmed();
        if (totalSectors != nullptr && !sizeStr.isEmpty()) {
            *totalSectors = sizeStr.toULongLong() / 512;
        }



        QString columns = "NAME,START,SIZE,FSTYPE,MOUNTPOINT,TYPE,FSUSED";
        QByteArray jsonOutput = runLsblkJson(diskPath, columns);

        if (jsonOutput.isEmpty()) {

            columns = "NAME,SIZE,FSTYPE,MOUNTPOINT,TYPE,FSUSED";
            jsonOutput = runLsblkJson(diskPath, columns);
        }

        if (jsonOutput.isEmpty()) return partitions;

        QJsonDocument doc = QJsonDocument::fromJson(jsonOutput);
        QJsonObject root = doc.object();
        QJsonArray blockdevices = root["blockdevices"].toArray();

        parseBlockDevices(blockdevices, partitions);

        return partitions;
    }

private:
    static QByteArray runLsblkJson(const QString &diskPath, const QString &columns) {
        QProcess proc;
        QStringList args;
        args << "-J" << "-b" << "-o" << columns << diskPath;

        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("LC_ALL", "C");
        proc.setProcessEnvironment(env);

        proc.start("lsblk", args);
        proc.waitForFinished();
        return proc.readAllStandardOutput();
    }

    static void parseBlockDevices(const QJsonArray &devices, QVector<PartitionInfo> &list) {
        for (const QJsonValue &val : devices) {
            QJsonObject obj = val.toObject();

            QString name = obj["name"].toString();
            QString type = obj["type"].toString();
            QString fs = obj["fstype"].toString();
            QString mount = obj["mountpoint"].toString();

            if (mount.isEmpty() && obj.contains("mountpoints")) {
                 QJsonArray mps = obj["mountpoints"].toArray();
                 if (!mps.isEmpty() && !mps[0].isNull()) mount = mps[0].toString();
            }


            if (obj.contains("children")) {
                parseBlockDevices(obj["children"].toArray(), list);
            }



            if (type == "disk") {
                bool hasChildren = obj.contains("children") && !obj["children"].toArray().isEmpty();


                if (hasChildren) continue;



            }

            PartitionInfo info;
            info.devicePath = name.startsWith("/") ? name : "/dev/" + name;

            QString idStr = name;
            idStr.remove(QRegExp("[^0-9]"));
            info.id = idStr.isEmpty() ? 1 : idStr.toInt();

            if (obj.contains("start")) info.startLBA = obj["start"].toVariant().toULongLong();
            else info.startLBA = 0;

            quint64 sizeBytes = obj["size"].toVariant().toULongLong();
            info.sizeGB = (double)sizeBytes / (1024.0 * 1024.0 * 1024.0);

            quint64 sectors = sizeBytes / 512;
            info.endLBA = info.startLBA + sectors - 1;


            info.fsType = fs.isEmpty() ? "Bilinmiyor (Raw)" : fs;
            info.mountPoint = mount;
            info.isMounted = !mount.isEmpty();

            if (!obj["fsused"].isNull()) {
                quint64 usedBytes = obj["fsused"].toVariant().toULongLong();
                info.usedGB = (double)usedBytes / (1024.0 * 1024.0 * 1024.0);
            } else {
                info.usedGB = -1;
            }

            list.append(info);
        }
    }
};

#endif // DISKUTILS_H
