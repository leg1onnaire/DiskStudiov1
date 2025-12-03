#ifndef RESIZEDIALOG_H
#define RESIZEDIALOG_H

#include <QDialog>
#include <QSlider>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

class ResizeDialog : public QDialog
{
    Q_OBJECT

public:

    explicit ResizeDialog(const QString &partName, double currentSizeGB, double maxSizeGB, QWidget *parent = nullptr);


    double getNewSizeGB() const;

private slots:

    void onSliderChanged(int value);
    void onSpinBoxChanged(double value);

private:
    QLabel *infoLabel;
    QSlider *sizeSlider;
    QDoubleSpinBox *sizeSpinBox;
    QLabel *minLabel;
    QLabel *maxLabel;

    double m_maxSizeGB;
};

#endif // RESIZEDIALOG_H
