#include "resizedialog.h"
#include <QHBoxLayout>

ResizeDialog::ResizeDialog(const QString &partName, double currentSizeGB, double maxSizeGB, QWidget *parent)
    : QDialog(parent), m_maxSizeGB(maxSizeGB)
{
    setWindowTitle("Boyutlandır: " + partName);
    resize(400, 250);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);


    infoLabel = new QLabel(QString("Bölüm: %1\nMevcut Boyut: %2 GB").arg(partName).arg(currentSizeGB), this);
    infoLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    infoLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(infoLabel);



    sizeSlider = new QSlider(Qt::Horizontal, this);
    sizeSlider->setRange(100, (int)(maxSizeGB * 1024));
    sizeSlider->setValue((int)(currentSizeGB * 1024));


    sizeSlider->setStyleSheet(
        "QSlider::groove:horizontal { border: 1px solid #999999; height: 20px; background: white; margin: 2px 0; }"
        "QSlider::handle:horizontal { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f); border: 1px solid #5c5c5c; width: 20px; margin: -2px 0; border-radius: 3px; }"
        "QSlider::sub-page:horizontal { background: #f6e58d; border: 1px solid #777; height: 20px; border-radius: 2px; }" // Sarı Dolgu (GParted rengi)
    );
    mainLayout->addWidget(sizeSlider);


    QHBoxLayout *limitsLayout = new QHBoxLayout();
    minLabel = new QLabel("Min: 100 MB", this);
    maxLabel = new QLabel(QString("Maks: %1 GB").arg(maxSizeGB, 0, 'f', 2), this);
    limitsLayout->addWidget(minLabel);
    limitsLayout->addStretch();
    limitsLayout->addWidget(maxLabel);
    mainLayout->addLayout(limitsLayout);


    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->addWidget(new QLabel("Yeni Boyut (GB):"));

    sizeSpinBox = new QDoubleSpinBox(this);
    sizeSpinBox->setRange(0.1, maxSizeGB);
    sizeSpinBox->setValue(currentSizeGB);
    sizeSpinBox->setSuffix(" GB");
    sizeSpinBox->setSingleStep(0.5);
    inputLayout->addWidget(sizeSpinBox);
    mainLayout->addLayout(inputLayout);


    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *cancelBtn = new QPushButton("İptal", this);
    QPushButton *okBtn = new QPushButton("Boyutlandır", this);


    okBtn->setStyleSheet("background-color: #e67e22; color: white; font-weight: bold; padding: 6px;");

    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(okBtn);
    mainLayout->addLayout(btnLayout);


    connect(sizeSlider, &QSlider::valueChanged, this, &ResizeDialog::onSliderChanged);
    connect(sizeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ResizeDialog::onSpinBoxChanged);

    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

double ResizeDialog::getNewSizeGB() const {
    return sizeSpinBox->value();
}

void ResizeDialog::onSliderChanged(int value) {

    double valGB = (double)value / 1024.0;

    sizeSpinBox->blockSignals(true);
    sizeSpinBox->setValue(valGB);
    sizeSpinBox->blockSignals(false);
}

void ResizeDialog::onSpinBoxChanged(double value) {

    int valMB = (int)(value * 1024);
    sizeSlider->blockSignals(true);
    sizeSlider->setValue(valMB);
    sizeSlider->blockSignals(false);
}
