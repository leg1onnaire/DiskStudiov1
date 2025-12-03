#include "mainwindow.h"
#include "diskutils.h"
#include "resizedialog.h"
#include "clonedialog.h"
#include "isowriterdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QLabel>
#include <QInputDialog>
#include <QMenu>
#include <QFileDialog>
#include <QThread>
#include <QProgressBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Disk Studio");
    resize(1100, 700);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);


    QHBoxLayout *topLayout = new QHBoxLayout();

    QLabel *lbl = new QLabel("Disk Seçin:", this);
    diskCombo = new QComboBox(this);
    diskCombo->setMinimumWidth(200);

    refreshButton = new QPushButton("⟳ Yenile", this);

    scanButton = new QPushButton("Seçili Diski Yükle", this);
    scanButton->setStyleSheet("background-color: #2e8b57; color: white; font-weight: bold;");

    QPushButton *cloneButton = new QPushButton("💿 Klonla", this);
    cloneButton->setStyleSheet("background-color: #8e44ad; color: white; font-weight: bold;");

    QPushButton *isoButton = new QPushButton("🔥 ISO Yazdır", this);
    isoButton->setStyleSheet("background-color: #d35400; color: white; font-weight: bold;");

    QPushButton *restoreButton = new QPushButton("♻️ USB Sıfırla (Restore)", this);
    restoreButton->setStyleSheet("background-color: #2980b9; color: white; font-weight: bold;");

    topLayout->addWidget(lbl);
    topLayout->addWidget(diskCombo);
    topLayout->addWidget(refreshButton);
    topLayout->addWidget(scanButton);
    topLayout->addWidget(cloneButton);
    topLayout->addWidget(isoButton);
    topLayout->addWidget(restoreButton);

    layout->addLayout(topLayout);


    layout->addWidget(new QLabel("Görsel Harita:"));
    visualizer = new DiskVisualizer(this);
    layout->addWidget(visualizer);


    layout->addWidget(new QLabel("Bölüm Listesi (Sağ Tık Menüsü Aktif):"));
    tableWidget = new QTableWidget(this);


    tableWidget->setColumnCount(8);
    QStringList headers;

    headers << "No" << "Aygıt" << "Boyut" << "Doluluk" << "Format" << "Bağlı (Mount)" << "Başlangıç" << "Bitiş";
    tableWidget->setHorizontalHeaderLabels(headers);

    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    layout->addWidget(tableWidget);


    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::refreshDiskList);
    connect(scanButton, &QPushButton::clicked, this, &MainWindow::scanDisk);
    connect(cloneButton, &QPushButton::clicked, this, &MainWindow::openCloneWizard);
    connect(isoButton, &QPushButton::clicked, this, &MainWindow::openIsoWizard);
    connect(restoreButton, &QPushButton::clicked, this, &MainWindow::restoreUsb);
    connect(tableWidget, &QTableWidget::customContextMenuRequested, this, &MainWindow::showContextMenu);


    refreshDiskList();
}

MainWindow::~MainWindow() {}

void MainWindow::refreshDiskList() {
    diskCombo->clear();
    QStringList disks = DiskUtils::getAvailableDisks();
    if (disks.isEmpty()) diskCombo->addItem("Disk Bulunamadı");
    else diskCombo->addItems(disks);
}

void MainWindow::scanDisk() {
    QString path = diskCombo->currentText();
    if (path.isEmpty() || path.contains("Disk Bulunamadı")) return;

    quint64 totalSec = 0;

    QVector<PartitionInfo> partitions = DiskUtils::readPartitions(path, &totalSec);

    visualizer->setPartitions(partitions, totalSec);

    tableWidget->setRowCount(0);
    for (const PartitionInfo &p : partitions) {
        int row = tableWidget->rowCount();
        tableWidget->insertRow(row);


        tableWidget->setItem(row, 0, new QTableWidgetItem(QString::number(p.id)));


        tableWidget->setItem(row, 1, new QTableWidgetItem(p.devicePath));


        tableWidget->setItem(row, 2, new QTableWidgetItem(QString::number(p.sizeGB, 'f', 2) + " GB"));


        if (p.isMounted && p.usedGB >= 0) {
            QProgressBar *bar = new QProgressBar();


            int percent = 0;
            if (p.sizeGB > 0) percent = (int)((p.usedGB / p.sizeGB) * 100.0);
            if (percent > 100) percent = 100;

            bar->setRange(0, 100);
            bar->setValue(percent);


            QString text = QString("%1% (%2 GB)").arg(percent).arg(QString::number(p.usedGB, 'f', 1));
            bar->setFormat(text);
            bar->setAlignment(Qt::AlignCenter);


            QString style = "QProgressBar { border: 1px solid grey; border-radius: 3px; text-align: center; color: black; }";
            if (percent < 60)
                style += "QProgressBar::chunk { background-color: #27ae60; }";
            else if (percent < 90)
                style += "QProgressBar::chunk { background-color: #f39c12; }";
            else
                style += "QProgressBar::chunk { background-color: #c0392b; }";

            bar->setStyleSheet(style);
            tableWidget->setCellWidget(row, 3, bar);
        } else {

            QTableWidgetItem *item = new QTableWidgetItem("N/A (Bağlı Değil)");
            item->setTextAlignment(Qt::AlignCenter);
            item->setForeground(QBrush(Qt::darkGray));
            tableWidget->setItem(row, 3, item);
        }


        tableWidget->setItem(row, 4, new QTableWidgetItem(p.fsType));


        QString mnt = p.mountPoint.isEmpty() ? "-" : p.mountPoint;
        tableWidget->setItem(row, 5, new QTableWidgetItem(mnt));


        tableWidget->setItem(row, 6, new QTableWidgetItem(QString::number(p.startLBA)));


        tableWidget->setItem(row, 7, new QTableWidgetItem(QString::number(p.endLBA)));
    }
}



void MainWindow::showContextMenu(const QPoint &pos) {
    QModelIndex index = tableWidget->indexAt(pos);
    if (!index.isValid()) return;

    QMenu contextMenu(tr("Disk İşlemleri"), this);

    QAction *actMount = contextMenu.addAction("Bağla / Ayır (Mount/Unmount)");
    contextMenu.addSeparator();


    QAction *actRename = contextMenu.addAction("İsim Değiştir (Etiket)");

    QAction *actFormat = contextMenu.addAction("Formatla (Biçimlendir)");
    QAction *actResize = contextMenu.addAction("Boyutlandır (Resize)");
    QAction *actBackup = contextMenu.addAction("Yedek Al (.img)");
    contextMenu.addSeparator();
    QAction *actDelete = contextMenu.addAction("Bölümü SİL");

    connect(actMount, &QAction::triggered, this, &MainWindow::toggleMount);


    connect(actRename, &QAction::triggered, this, &MainWindow::renamePartition);

    connect(actFormat, &QAction::triggered, this, &MainWindow::formatPartition);
    connect(actResize, &QAction::triggered, this, &MainWindow::resizePartition);
    connect(actBackup, &QAction::triggered, this, &MainWindow::backupPartition);
    connect(actDelete, &QAction::triggered, this, &MainWindow::deletePartition);

    contextMenu.exec(tableWidget->viewport()->mapToGlobal(pos));
}
void MainWindow::restoreUsb() {
    QString diskPath = diskCombo->currentText();
    if (diskPath.isEmpty() || diskPath.contains("Disk Bulunamadı")) {
        QMessageBox::warning(this, "Hata", "Lütfen bir disk seçin!");
        return;
    }


    QStringList formats;
    formats << "FAT32 (Varsayılan - En Uyumlu)"
            << "NTFS (Windows & Büyük Dosyalar)"
            << "exFAT (Modern & Büyük Dosyalar)"
            << "ext4 (Sadece Linux)";

    bool ok;
    QString selection = QInputDialog::getItem(this, "Format Seçimi",
                                              "USB hangi formatta sıfırlansın?",
                                              formats, 0, false, &ok);
    if (!ok || selection.isEmpty()) return;


    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "USB Sıfırlama",
        QString("HEDEF: %1\n\nBu işlem diskin BÖLÜM TABLOSUNU SİLECEK ve\n%2 olarak formatlayacaktır.\n\nOnaylıyor musunuz?").arg(diskPath).arg(selection),
        QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::No) return;

    QMessageBox::information(this, "Bilgi", "İşlem başlıyor. Lütfen bekleyin...");

    QProcess process;


    process.execute("umount", QStringList() << diskPath + "*");


    process.start("parted", QStringList() << "--script" << diskPath << "mklabel" << "msdos");
    process.waitForFinished();



    QString partType = "fat32"; // Varsayılan
    if (selection.contains("ext4")) partType = "ext4";
    else if (selection.contains("NTFS")) partType = "ntfs";


    process.start("parted", QStringList() << "--script" << diskPath << "mkpart" << "primary" << partType << "1MiB" << "100%");
    process.waitForFinished();


    QThread::sleep(1);


    QString partPath;
    if (diskPath.contains("nvme") || diskPath.contains("mmcblk"))
        partPath = diskPath + "p1";
    else
        partPath = diskPath + "1";


    QString program;
    QStringList args;

    if (selection.contains("FAT32")) {
        program = "mkfs.vfat";
        args << "-F" << "32" << "-n" << "USB_DISK" << partPath;
    }
    else if (selection.contains("NTFS")) {
        program = "mkfs.ntfs";

        args << "-f" << "-L" << "USB_DISK" << partPath;
    }
    else if (selection.contains("exFAT")) {
        program = "mkfs.exfat";
        args << "-n" << "USB_DISK" << partPath;
    }
    else if (selection.contains("ext4")) {
        program = "mkfs.ext4";
        args << "-F" << "-L" << "USB_DISK" << partPath;
    }

    process.start(program, args);
    process.waitForFinished();

    if (process.exitCode() == 0) {
        QMessageBox::information(this, "Başarılı", "USB Bellek sıfırlandı: " + selection);
        scanDisk();
    } else {
        QString err = process.readAllStandardError();
        if (err.isEmpty()) err = "Bilinmeyen hata (Araç kurulu olmayabilir: exfat-utils, ntfs-3g)";
        QMessageBox::critical(this, "Hata", "Formatlanamadı:\n" + err);
    }
}

void MainWindow::toggleMount() {
    int row = tableWidget->currentRow();
    if (row < 0) return;

    QString devPath = tableWidget->item(row, 1)->text();
    QString currentMount = tableWidget->item(row, 5)->text();

    QProcess process;

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LC_ALL", "C");
    process.setProcessEnvironment(env);

    if (currentMount == "-" || currentMount.isEmpty()) {


        process.start("udisksctl", QStringList() << "mount" << "-b" << devPath);
    } else {

        process.start("umount", QStringList() << devPath);
    }

    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    QString error = process.readAllStandardError();

    if (process.exitCode() == 0) {


        if (!output.isEmpty()) QMessageBox::information(this, "Bilgi", output);
        scanDisk();
    } else {

        QMessageBox::critical(this, "Mount Hatası",
            "Disk bağlanamadı!\n\nHata Çıktısı:\n" + error +
            "\n\nOlası Sebepler:\n1. Disk zaten bağlı.\n2. Dosya sistemi bozuk (exFAT/NTFS).\n3. Windows 'Hızlı Başlat' yüzünden diski kilitledi.");
    }
}

void MainWindow::backupPartition() {
    int row = tableWidget->currentRow();
    QString devPath = tableWidget->item(row, 1)->text();
    QString fileName = QFileDialog::getSaveFileName(this, "Yedek Kaydet", QDir::homePath(), "Image (*.img)");
    if (fileName.isEmpty()) return;

    QMessageBox::information(this, "Bilgi", "Yedekleme başladı... Bekleyiniz.");
    QProcess process;
    process.start("dd", QStringList() << "if=" + devPath << "of=" + fileName << "bs=4M" << "status=none");
    process.waitForFinished(-1);

    if(process.exitCode() == 0) QMessageBox::information(this, "Tamam", "Yedek alındı.");
    else QMessageBox::critical(this, "Hata", process.readAllStandardError());
}

void MainWindow::deletePartition() {
    int row = tableWidget->currentRow();
    QString partNum = tableWidget->item(row, 0)->text();
    QString diskPath = diskCombo->currentText();
    QString mountStatus = tableWidget->item(row, 5)->text();

    if (mountStatus != "-" && !mountStatus.isEmpty()) {
        QMessageBox::warning(this, "Hata", "Disk BAĞLI! Önce ayırın.");
        return;
    }
    if (QMessageBox::question(this, "Sil", "Emin misiniz?", QMessageBox::Yes|QMessageBox::No) == QMessageBox::No) return;

    QProcess::execute("parted", QStringList() << "--script" << diskPath << "rm" << partNum);
    scanDisk();
}

void MainWindow::formatPartition() {
    int row = tableWidget->currentRow();

    if (row < 0) return;

    QString devicePath = tableWidget->item(row, 1)->text();
    QString mountStatus = tableWidget->item(row, 5)->text(); // Sütun 5

    if (mountStatus != "-" && !mountStatus.isEmpty()) {
        QMessageBox::warning(this, "Hata", "Disk şu an BAĞLI! Önce ayırın (Unmount/Bağla-Ayır).");
        return;
    }


    QStringList formats;
    formats << "fat32" << "ntfs" << "ext4" << "exfat";

    bool ok;
    QString fs = QInputDialog::getItem(this, "Formatla", "Dosya Sistemi Seçin:", formats, 0, false, &ok);

    if (ok && !fs.isEmpty()) {

        if (QMessageBox::question(this, "Onay", devicePath + " aygıtı " + fs + " olarak formatlanacak.\nVeriler SİLİNECEK. Emin misiniz?", QMessageBox::Yes|QMessageBox::No) == QMessageBox::No)
            return;

        QMessageBox::information(this, "Bilgi", "Formatlama işlemi başladı...");

        QString program;
        QStringList args;


        if (fs == "ext4") {
            program = "mkfs.ext4";
            args << "-F" << "-L" << "DISK_EXT4" << devicePath;
        }
        else if (fs == "ntfs") {
            program = "mkfs.ntfs";
            args << "-f" << "-L" << "DISK_NTFS" << devicePath;
        }
        else if (fs == "fat32") {
            program = "mkfs.vfat";
            args << "-F" << "32" << "-n" << "DISK_FAT" << devicePath;
        }
        else if (fs == "exfat") {

            program = "mkfs.exfat";
            args << "-n" << "DISK_EXFAT" << devicePath;
        }

        QProcess process;
        process.start(program, args);
        process.waitForFinished();

        if (process.exitCode() == 0) {
            QMessageBox::information(this, "Başarılı", "Format tamamlandı: " + fs);
            scanDisk(); // Listeyi yenile
        } else {
            QString err = process.readAllStandardError();
            if (err.isEmpty()) err = "Bilinmeyen hata. (Gerekli paketler kurulu mu? exfat-fuse / ntfs-3g)";
            QMessageBox::critical(this, "Hata", "Formatlanamadı:\n" + err);
        }
    }
}

void MainWindow::resizePartition() {
    int row = tableWidget->currentRow();
    QString partNum = tableWidget->item(row, 0)->text();


    QString sizeStr = tableWidget->item(row, 2)->text().split(" ")[0];
    double currentSizeGB = sizeStr.toDouble();


    QString startSectorStr = tableWidget->item(row, 6)->text();
    quint64 startSector = startSectorStr.toULongLong();

    QString diskPath = diskCombo->currentText();

    ResizeDialog dlg(diskPath + "p" + partNum, currentSizeGB, currentSizeGB, this);
    if (dlg.exec() == QDialog::Accepted) {
        double newSizeGB = dlg.getNewSizeGB();
        quint64 newSizeSectors = (quint64)(newSizeGB * 1024.0 * 1024.0 * 1024.0 / 512.0);
        quint64 newEndSector = startSector + newSizeSectors - 1;
        QString endSectorParam = QString::number(newEndSector) + "s";

        QProcess process;
        process.start("parted", QStringList() << "--script" << diskPath << "resizepart" << partNum << endSectorParam);
        process.waitForFinished();

        if (process.exitCode() == 0) {
             QMessageBox::information(this, "Başarılı", "Bölüm boyutlandırıldı.");
             scanDisk();
        } else {
             QMessageBox::critical(this, "Hata", process.readAllStandardError());
        }
    }
}

void MainWindow::openCloneWizard() {
    CloneDialog dlg(this);
    dlg.exec();
    refreshDiskList();
}

void MainWindow::openIsoWizard() {
    IsoWriterDialog dlg(this);
    dlg.exec();
    refreshDiskList();
}


void MainWindow::renamePartition() {
    int row = tableWidget->currentRow();
    QString devicePath = tableWidget->item(row, 1)->text();
    QString fsType = tableWidget->item(row, 4)->text();
    QString mountStatus = tableWidget->item(row, 5)->text();


    if (mountStatus != "-" && !mountStatus.isEmpty()) {
        QMessageBox::warning(this, "Hata", "İsim değiştirmek için önce diski AYIRIN (Unmount).");
        return;
    }

    // 2. Yeni İsim İste
    bool ok;
    QString newLabel = QInputDialog::getText(this, "İsim Değiştir",
                                             "Yeni Disk İsmi (Etiket):", QLineEdit::Normal,
                                             "YENI_DISK", &ok);
    if (!ok || newLabel.isEmpty()) return;


    newLabel = newLabel.replace(" ", "_");

    QProcess process;
    QString program;
    QStringList args;


    if (fsType.contains("exfat")) {

        program = "exfatlabel";
        args << devicePath << newLabel;
    }
    else if (fsType.contains("ntfs")) {

        program = "ntfslabel";
        args << devicePath << newLabel;
    }
    else if (fsType.contains("fat") || fsType.contains("vfat")) {

        program = "fatlabel";
        args << devicePath << newLabel;
    }
    else if (fsType.contains("ext")) {

        program = "e2label";
        args << devicePath << newLabel;
    }
    else {
        QMessageBox::warning(this, "Hata", "Bu dosya sistemi için isim değiştirme desteklenmiyor: " + fsType);
        return;
    }


    process.start(program, args);
    process.waitForFinished();

    if (process.exitCode() == 0) {
        QMessageBox::information(this, "Başarılı", "Disk ismi değiştirildi: " + newLabel);
        scanDisk();
    } else {
        QString err = process.readAllStandardError();
        if (err.isEmpty()) err = "Araç bulunamadı (fatlabel/exfatlabel).";
        QMessageBox::critical(this, "Hata", "İsim değiştirilemedi:\n" + err);
    }
}

