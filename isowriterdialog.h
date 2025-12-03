#ifndef ISOWRITERDIALOG_H
#define ISOWRITERDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QProgressBar>
#include <QPushButton>
#include <QProcess>
#include <QLabel>

class IsoWriterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IsoWriterDialog(QWidget *parent = nullptr);

private slots:
    void selectIsoFile();
    void startFlashing();
    void updateProgress();
    void processFinished(int exitCode);

private:
    QLineEdit *filePathInput;
    QComboBox *targetCombo;
    QProgressBar *progressBar;
    QLabel *statusLabel;
    QPushButton *flashButton;
    QProcess *process;

    quint64 m_totalSize;
};

#endif // ISOWRITERDIALOG_H
