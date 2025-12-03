#include "clonedialog.h"
#include "diskutils.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDebug>
#include <QThread>
#include <QApplication>

CloneDialog::CloneDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Disk Klonlama Sihirbazı");
    resize(550, 350);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);


    mainLayout->addWidget(new QLabel("KAYNAK Disk (Kopyalanacak):"));
    sourceCombo = new QComboBox(this);
    sourceCombo->addItems(DiskUtils::getAvailableDisks());
    mainLayout->addWidget(sourceCombo);


    QLabel *arrow = new QLabel("⬇️ Kopyalanıyor ⬇️", this);
    arrow->setAlignment(Qt::AlignCenter);
    arrow->setStyleSheet("font-weight: bold; color: #2980b9; margin: 10px;");
    mainLayout->addWidget(arrow);


    mainLayout->addWidget(new QLabel("HEDEF Disk (Silinip Üzerine Yazılacak):"));
    targetCombo = new QComboBox(this);
    targetCombo->addItems(DiskUtils::getAvailableDisks());
    mainLayout->addWidget(targetCombo);


    QLabel *warning = new QLabel("DİKKAT: Hedef diskteki TÜM VERİLER silinecektir!", this);
    warning->setStyleSheet("color: red; font-weight: bold; border: 2px solid red; padding: 10px; border-radius: 5px;");
    warning->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(warning);


    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 0);
    progressBar->setVisible(false);
    mainLayout->addWidget(progressBar);

    statusLabel = new QLabel("", this);
    statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(statusLabel);


    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *cancelBtn = new QPushButton("İptal", this);
    startButton = new QPushButton("Klonlamayı Başlat", this);
    startButton->setStyleSheet("background-color: #c0392b; color: white; font-weight: bold; padding: 10px;");

    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(startButton);
    mainLayout->addLayout(btnLayout);


    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(startButton, &QPushButton::clicked, this, &CloneDialog::startCloning);


    process = new QProcess(this);

    connect(process, &QProcess::readyReadStandardError, this, &CloneDialog::updateProgress);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &CloneDialog::processFinished);
}

void CloneDialog::startCloning()
{
    QString source = sourceCombo->currentText();
    QString target = targetCombo->currentText();


    if (source == target) {
        QMessageBox::warning(this, "Hata", "Kaynak ve Hedef disk AYNI olamaz!");
        return;
    }


    quint64 srcSectors = 0;
    quint64 tgtSectors = 0;


    DiskUtils::readPartitions(source, &srcSectors);
    DiskUtils::readPartitions(target, &tgtSectors);

    if (srcSectors > tgtSectors) {
        double srcGB = (double)srcSectors * 512.0 / (1024.0*1024.0*1024.0);
        double tgtGB = (double)tgtSectors * 512.0 / (1024.0*1024.0*1024.0);

        QMessageBox::critical(this, "Boyut Hatası",
            QString("Hedef disk, Kaynak diskten KÜÇÜK!\n\n"
                    "Kaynak: %1 GB\n"
                    "Hedef: %2 GB\n\n"
                    "Veri sığmayacağı için işlem iptal edildi.")
            .arg(QString::number(srcGB, 'f', 2))
            .arg(QString::number(tgtGB, 'f', 2)));
        return;
    }


    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Son Onay",
                                  QString("KAYNAK: %1\nHEDEF: %2\n\n"
                                          "%2 içindeki HER ŞEY silinecek ve %1 buraya kopyalanacak.\n\n"
                                          "Bu işlem geri alınamaz. Onaylıyor musunuz?")
                                          .arg(source).arg(target),
                                  QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::No) return;


    sourceCombo->setEnabled(false);
    targetCombo->setEnabled(false);
    startButton->setEnabled(false);
    progressBar->setVisible(true);
    statusLabel->setText("Klonlanıyor... Lütfen bekleyin (Disk boyutuna göre saatler sürebilir)");


    QProcess::execute("umount", QStringList() << target + "*");


    QStringList args;
    args << "if=" + source << "of=" + target << "bs=4M" << "status=progress" << "oflag=sync";

    process->start("dd", args);
}

void CloneDialog::updateProgress()
{

    QString output = process->readAllStandardError();
    if (output.contains("bytes")) {
        QStringList lines = output.split("\r");
        if (!lines.isEmpty()) {
            statusLabel->setText(lines.last().trimmed());
        }
    }
}

void CloneDialog::processFinished(int exitCode)
{
    if (exitCode == 0) {

        QString target = targetCombo->currentText();
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Klonlama Tamamlandı",
            "Klonlama başarılı!\n\n"
            "Eğer bu iki diski AYNI ANDA bu bilgisayara takacaksanız,\n"
            "çakışma olmaması için hedef diskin UUID'lerini (Kimliklerini) değiştirmem gerekir.\n\n"
            "UUID'ler değiştirilsin mi? (Önerilen: Evet)",
            QMessageBox::Yes|QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            randomizeUUIDs(target);
        }


        QMessageBox::information(this, "Bitti", "İşlem başarıyla tamamlandı. Yeni diski kullanabilirsiniz.");
        accept();
    } else {
        QMessageBox::critical(this, "Hata", "Klonlama başarısız oldu.\n" + process->readAllStandardError());


        sourceCombo->setEnabled(true);
        targetCombo->setEnabled(true);
        startButton->setEnabled(true);
        progressBar->setVisible(false);
        statusLabel->setText("Hata oluştu.");
    }
}


void CloneDialog::randomizeUUIDs(const QString &targetDisk) {
    statusLabel->setText("Partition tablosu yenileniyor...");
    QApplication::processEvents();


    QProcess::execute("partprobe", QStringList() << targetDisk);
    QThread::sleep(2);

    statusLabel->setText("Bölümler taranıyor...");
    QApplication::processEvents();


    QVector<PartitionInfo> parts = DiskUtils::readPartitions(targetDisk);
    int changeCount = 0;

    for (const PartitionInfo &p : parts) {
        QProcess proc;
        QString msg = QString("UUID değiştiriliyor: %1").arg(p.devicePath);
        statusLabel->setText(msg);
        QApplication::processEvents();

        if (p.fsType.contains("ext")) {
            // Ext2/3/4 -> tune2fs
            proc.start("tune2fs", QStringList() << "-U" << "random" << p.devicePath);
            proc.waitForFinished();
            changeCount++;
        }
        else if (p.fsType.contains("swap")) {

            proc.start("mkswap", QStringList() << p.devicePath);
            proc.waitForFinished();
            changeCount++;
        }
        else if (p.fsType.contains("xfs")) {

             proc.start("xfs_admin", QStringList() << "-U" << "generate" << p.devicePath);
             proc.waitForFinished();
             changeCount++;
        }


    }

    statusLabel->setText("UUID işlemi tamamlandı.");
}
