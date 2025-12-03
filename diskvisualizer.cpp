#include "diskvisualizer.h"
#include <QPainter>

DiskVisualizer::DiskVisualizer(QWidget *parent) : QWidget(parent)
{

    setMinimumHeight(70);
    m_totalSectors = 0;
}

void DiskVisualizer::setPartitions(const QVector<PartitionInfo> &parts, quint64 totalSectors)
{
    m_partitions = parts;
    m_totalSectors = totalSectors;
    update();
}

void DiskVisualizer::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);


    painter.setRenderHint(QPainter::Antialiasing);



    painter.fillRect(rect(), QColor(224, 224, 224));

    if (m_totalSectors == 0) {
        painter.setPen(Qt::darkGray);
        painter.drawText(rect(), Qt::AlignCenter, "Disk Verisi Yok veya Seçilmedi");
        return;
    }

    double widgetWidth = width();


    QPen borderPen(Qt::white);
    borderPen.setWidth(2);
    painter.setPen(borderPen);

    for (const PartitionInfo &p : m_partitions) {


        double partSizeSec = (double)(p.endLBA - p.startLBA);
        double ratio = partSizeSec / (double)m_totalSectors;
        double startRatio = (double)p.startLBA / (double)m_totalSectors;

        int xPos = (int)(startRatio * widgetWidth);
        int pWidth = (int)(ratio * widgetWidth);


        if (pWidth < 4) pWidth = 4;


        QRect partRect(xPos, 5, pWidth, height() - 25);


        QColor themeColor;
        if (p.fsType.contains("fat")) themeColor = QColor(46, 204, 113);      // Yeşil (FAT)
        else if (p.fsType.contains("ntfs")) themeColor = QColor(52, 152, 219); // Mavi (NTFS)
        else if (p.fsType.contains("ext")) themeColor = QColor(155, 89, 182);  // Mor (EXT4)
        else if (p.fsType.contains("swap")) themeColor = QColor(231, 76, 60);  // Kırmızı (Swap)
        else themeColor = QColor(149, 165, 166);                               // Gri (Bilinmeyen)



        if (p.isMounted && p.usedGB >= 0 && p.sizeGB > 0) {




            painter.setBrush(themeColor.lighter(170));
            painter.drawRect(partRect);


            double usageRatio = p.usedGB / p.sizeGB;
            if (usageRatio > 1.0) usageRatio = 1.0;


            int usedWidth = (int)(partRect.width() * usageRatio);
            if (usedWidth < 2 && usageRatio > 0) usedWidth = 2;

            QRect usedRect(partRect.x(), partRect.y(), usedWidth, partRect.height());
            painter.setBrush(themeColor);


            painter.setPen(Qt::NoPen);
            painter.drawRect(usedRect);


            painter.setPen(borderPen);
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(partRect);

        } else {


            painter.setBrush(themeColor);
            painter.drawRect(partRect);
        }



        if (pWidth > 30) {
            painter.setPen(Qt::black);


            painter.drawText(partRect, Qt::AlignCenter, QString::number(p.id));


            if (pWidth > 60) {
                QString sizeText = QString::number(p.sizeGB, 'f', 1) + " GB";

                QRect textRect(xPos, height() - 20, pWidth, 15);
                painter.setPen(Qt::darkGray);
                QFont font = painter.font();
                font.setPointSize(8);
                painter.setFont(font);
                painter.drawText(textRect, Qt::AlignCenter, sizeText);
            }
        }


        painter.setPen(borderPen);
    }
}
