#include "isowriterdialog.h"
#include "diskutils.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QDebug>

IsoWriterDialog::IsoWriterDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("ISO Yazdırıcı (Etcher Modu)");
    resize(500, 350);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);


    mainLayout->addWidget(new QLabel("1. Kaynak İmajı Seç (.iso, .img):"));
    QHBoxLayout *fileLayout = new QHBoxLayout();

    filePathInput = new QLineEdit(this);
    filePathInput->setPlaceholderText("Dosya seçilmedi...");
    filePathInput->setReadOnly(true);

    QPushButton *browseBtn = new QPushButton("Dosya Seç", this);
    connect(browseBtn, &QPushButton::clicked, this, &IsoWriterDialog::selectIsoFile);

    fileLayout->addWidget(filePathInput);
    fileLayout->addWidget(browseBtn);
    mainLayout->addLayout(fileLayout);


    QLabel *arrow = new QLabel("⬇️", this);
    arrow->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(arrow);


    mainLayout->addWidget(new QLabel("2. Hedef Diski Seç (USB/SD Kart):"));
    targetCombo = new QComboBox(this);
    targetCombo->addItems(DiskUtils::getAvailableDisks());
    mainLayout->addWidget(targetCombo);


    QLabel *warn = new QLabel("UYARI: Hedef diskteki TÜM VERİLER silinecektir!", this);
    warn->setStyleSheet("color: red; font-weight: bold; border: 1px solid red; padding: 5px;");
    warn->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(warn);


    progressBar = new QProgressBar(this);
    progressBar->setValue(0);
    progressBar->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(progressBar);

    statusLabel = new QLabel("Hazır", this);
    statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(statusLabel);


    flashButton = new QPushButton("⚡ FLASH! (Yazdır)", this);
    flashButton->setStyleSheet("background-color: #e67e22; color: white; font-weight: bold; font-size: 16px; padding: 10px;");
    mainLayout->addWidget(flashButton);

    connect(flashButton, &QPushButton::clicked, this, &IsoWriterDialog::startFlashing);


    process = new QProcess(this);
    connect(process, &QProcess::readyReadStandardError, this, &IsoWriterDialog::updateProgress);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &IsoWriterDialog::processFinished);
}

void IsoWriterDialog::selectIsoFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "ISO Dosyası Seç", QDir::homePath(), "Disk Images (*.iso *.img)");
    if (!fileName.isEmpty()) {
        filePathInput->setText(fileName);

        QFileInfo fi(fileName);
        m_totalSize = fi.size();
    }
}

void IsoWriterDialog::startFlashing()
{
    QString isoPath = filePathInput->text();
    QString targetDevice = targetCombo->currentText();

    if (isoPath.isEmpty()) {
        QMessageBox::warning(this, "Hata", "Lütfen bir ISO dosyası seçin!");
        return;
    }

    if (targetDevice.isEmpty() || targetDevice.contains("Disk Bulunamadı")) {
        QMessageBox::warning(this, "Hata", "Lütfen geçerli bir hedef disk seçin!");
        return;
    }


    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Son Onay",
        QString("HEDEF: %1\n\nBu diskin içindeki HER ŞEY silinecek ve\n%2\nburaya yazılacak.\n\nOnaylıyor musun?").arg(targetDevice).arg(isoPath),
        QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::No) return;


    flashButton->setEnabled(false);
    targetCombo->setEnabled(false);
    progressBar->setValue(0);
    statusLabel->setText("Disk hazırlanıyor (Unmounting)...");


    QProcess::execute("umount", QStringList() << targetDevice + "*");


    statusLabel->setText("Yazılıyor... Lütfen bekleyin.");


    QStringList args;
    args << "if=" + isoPath << "of=" + targetDevice << "bs=4M" << "status=progress" << "oflag=sync";

    process->start("dd", args);
}

void IsoWriterDialog::updateProgress()
{

    QString output = process->readAllStandardError();


    if (output.contains("bytes")) {

        QStringList parts = output.split(" ");
        if (parts.size() > 0) {

            quint64 bytesWritten = parts[0].toULongLong();

            if (m_totalSize > 0) {
                int percent = (int)((bytesWritten * 100) / m_totalSize);
                progressBar->setValue(percent);
                statusLabel->setText(QString("Yazılıyor: %%1").arg(percent));
            }
        }
    }
}

void IsoWriterDialog::processFinished(int exitCode)
{
    flashButton->setEnabled(true);
    targetCombo->setEnabled(true);

    if (exitCode == 0) {
        progressBar->setValue(100);
        statusLabel->setText("Tamamlandı!");
        QMessageBox::information(this, "Başarılı", "ISO yazdırma işlemi tamamlandı!\nDiski çıkarıp kullanabilirsiniz.");
        accept();
    } else {
        statusLabel->setText("Hata oluştu!");
        QMessageBox::critical(this, "Hata", "Yazdırma başarısız oldu.\n" + process->readAllStandardError());
    }
}
