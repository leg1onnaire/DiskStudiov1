#ifndef CLONEDIALOG_H
#define CLONEDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QProcess>

class CloneDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CloneDialog(QWidget *parent = nullptr);

private slots:
    void startCloning();
    void updateProgress();
    void processFinished(int exitCode);

private:
    void randomizeUUIDs(const QString &targetDisk);

    QComboBox *sourceCombo;
    QComboBox *targetCombo;
    QPushButton *startButton;
    QProgressBar *progressBar;
    QLabel *statusLabel;
    QProcess *process;
};

#endif // CLONEDIALOG_H
