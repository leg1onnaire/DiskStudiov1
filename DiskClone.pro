QT       += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# --- YENİ EKLENEN AYARLAR ---
# $$PWD = Projenin bulunduğu klasör demektir.
# Tüm çıktıları proje klasörünün içindeki "build" klasörüne yönlendiriyoruz.

# Çalıştırılabilir dosya (.exe veya Linux binary) buraya gelecek:
DESTDIR = $$PWD/build

# Ara derleme dosyaları (.o) buraya:
OBJECTS_DIR = $$PWD/build/obj

# Qt Meta-Object dosyaları (moc_*.cpp) buraya:
MOC_DIR = $$PWD/build/moc

# Kaynak dosyaları (qrc) buraya:
RCC_DIR = $$PWD/build/rcc

# Arayüz başlıkları (ui_*.h) buraya:
UI_DIR = $$PWD/build/ui
# -----------------------------

SOURCES += \
    clonedialog.cpp \
    diskvisualizer.cpp \
    isowriterdialog.cpp \
    main.cpp \
    mainwindow.cpp \
    resizedialog.cpp

HEADERS += \
    clonedialog.h \
    diskvisualizer.h \
    isowriterdialog.h \
    mainwindow.h \
    diskutils.h \
    resizedialog.h

# Uygulama adı
TARGET = DiskClone
TEMPLATE = app
