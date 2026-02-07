#include "bnrd.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QSettings>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusError>

Bnrd::Bnrd(QObject *parent) : QObject(parent) {
  initDirectories();

  server = new BnrServer(this);
}

void Bnrd::initDirectories() {
  QSettings settings("Stella", "Stella");

  QString defaultBackupDir;

#ifdef Q_OS_LINUX

  defaultBackupDir = QDir()
                         .home()
                         .filePath("Documents")
                         .append(QDir::separator())
                         .append("Stella")
                         .append(QDir::separator())
                         .append("backups");

  QDir().mkpath(defaultBackupDir);
#endif

  m_backupDir = settings.value("Bnr/backupDirectory", defaultBackupDir).toString();
}

void Bnrd::changeBackupDir(const QString &backupDir) {
  QString docsDir = QDir().home().filePath("Documents");

  if (backupDir != m_backupDir && backupDir != docsDir) {

    QString destDir = QDir(backupDir)
                          .filePath("Stella")
                          .append(QDir::separator())
                          .append("backups")
                          .append("%1");
    QDir().mkpath(destDir);

    QStringList exts = { "*" };
    QDirIterator it(m_backupDir, exts, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
      const QString fileName = it.next();
      QFileInfo info(fileName);

      qDebug() << "file source" << info.absoluteFilePath();

      int start = fileName.indexOf("backups");
      // qDebug() << "start index" << start;
      QString suffix = fileName.mid((start + 7), fileName.length() - (start + 7));
      QString dest = destDir.arg(suffix);

      qDebug() << "file dest" << dest;
      QFile file(fileName);
      QFileInfo newInfo(dest);
      QDir().mkpath(newInfo.absoluteDir().absolutePath());

      if (info.fileName() != "." && info.fileName() != "..") {
        file.copy(dest);
      }
    }
    QDir(m_backupDir).removeRecursively();
  }
}

int main(int argc, char **argv) {
  QCoreApplication app(argc, argv);
  app.setOrganizationName("Stella");
  app.setOrganizationDomain("io.github.stella");
  app.setApplicationName("Stella");

  if (!QDBusConnection::sessionBus().isConnected()) {
    fprintf(stderr,
            "Cannot connect to the D-Bus session bus.\n"
            "To start it, run:\n"
            "\teval `dbus-launch --auto-syntax`\n");
    return 1;
  }

  if (!QDBusConnection::sessionBus().registerService(SERVICE_NAME)) {
    fprintf(stderr, "%s\n", qPrintable(QDBusConnection::sessionBus().lastError().message()));
    exit(1);
  }

  Bnrd bnrd;
  QDBusConnection::sessionBus().registerObject("/", &bnrd, QDBusConnection::ExportAllSlots);

  return app.exec();
}
