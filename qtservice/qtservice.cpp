/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of a Qt Solutions component.
**
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
****************************************************************************/

#include "qtservice.h"
#include "qtunixsocket.h"
#include "qtunixserversocket.h"

#include <QCoreApplication>
#include <QTimer>
#include <QVector>
#include <QMap>
#include <QSettings>
#include <QProcess>
#include <QStringList>
#include <QFile>
#include <QDir>
#include <QMutex>
#include <QTime>
#include <QDebug>

#include <pwd.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>
#include <sys/stat.h>

static QFile* f = 0;

static void qtServiceCloseDebugLog()
{
    if (!f)
        return;
    QString ps(QTime::currentTime().toString("HH:mm:ss.zzz ") + QLatin1String("--- DEBUG LOG CLOSED ---\n\n"));
    f->write(ps.toAscii());
    f->flush();
    f->close();
    delete f;
    f = 0;
}

static QString encodeName(const QString &name, bool allowUpper = false)
{
    QString n = name.toLower();
    QString legal = QLatin1String("abcdefghijklmnopqrstuvwxyz1234567890");
    if (allowUpper)
        legal += QLatin1String("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    int pos = 0;
    while (pos < n.size()) {
        if (legal.indexOf(n[pos]) == -1)
            n.remove(pos, 1);
        else
            ++pos;
    }
    return n;
}

static QString login()
{
    QString l;
    uid_t uid = getuid();
    passwd *pw = getpwuid(uid);
    if (pw)
        l = QString(pw->pw_name);
    return l;
}

static QString socketPath(const QString &serviceName)
{
    QString sn = encodeName(serviceName);
    return QString(QLatin1String("/var/tmp/") + sn + QLatin1String(".") + login());
}
static bool sendCmd(const QString &serviceName, const QString &cmd)
{
    bool retValue = false;
    QtUnixSocket sock;
    if (sock.connectTo(socketPath(serviceName))) {
        sock.write(QString(cmd+"\r\n").toLatin1().constData());
        sock.flush();
        sock.waitForReadyRead(-1);
        QString reply = sock.readAll();
        if (reply == QLatin1String("true"))
            retValue = true;
        sock.close();
    }
    return retValue;
}

static QString absPath(const QString &path)
{
    QString ret;
    if (path[0] != QChar('/')) { // Not an absolute path
        int slashpos;
        if ((slashpos = path.lastIndexOf('/')) != -1) { // Relative path
            QDir dir = QDir::current();
            dir.cd(path.left(slashpos));
            ret = dir.absolutePath();
        } else { // Need to search $PATH
            char *envPath = ::getenv("PATH");
            if (envPath) {
                QStringList envPaths = QString::fromLocal8Bit(envPath).split(':');
                for (int i = 0; i < envPaths.size(); ++i) {
                    if (QFile::exists(envPaths.at(i) + QLatin1String("/") + QString(path))) {
                        QDir dir(envPaths.at(i));
                        ret = dir.absolutePath();
                        break;
                    }
                }
            }
        }
    } else {
        QFileInfo fi(path);
        ret = fi.absolutePath();
    }
    return ret;
}

void qtServiceLogDebug(QtMsgType type, const char* msg)
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);
    QString s(QTime::currentTime().toString("HH:mm:ss.zzz "));
    s += QString("[%1] ").arg(
#if defined(Q_OS_WIN32)
                               GetCurrentProcessId());
#else
                               getpid());
#endif

    if (!f) {
#if defined(Q_OS_WIN32)
        f = new QFile("c:/service-debuglog.txt");
#else
        f = new QFile("/tmp/service-debuglog.txt");
#endif
        if (!f->open(QIODevice::WriteOnly | QIODevice::Append)) {
            delete f;
            f = 0;
            return;
        }
        QString ps(QLatin1String("\n") + s + QLatin1String("--- DEBUG LOG OPENED ---\n"));
        f->write(ps.toAscii());
    }

    switch (type) {
    case QtWarningMsg:
        s += QLatin1String("WARNING: ");
        break;
    case QtCriticalMsg:
        s += QLatin1String("CRITICAL: ");
        break;
    case QtFatalMsg:
        s+= QLatin1String("FATAL: ");
        break;
    case QtDebugMsg:
        s += QLatin1String("DEBUG: ");
        break;
    default:
        // Nothing
        break;
    }

    s += msg;
    s += QLatin1String("\n");

    f->write(s.toAscii());
    f->flush();

    if (type == QtFatalMsg) {
        qtServiceCloseDebugLog();
        exit(1);
    }
}

QtServiceController::QtServiceController(const QString &name)
 : d_ptr(new QtServiceControllerPrivate())
{
    Q_D(QtServiceController);
    d->q_ptr = this;
    d->serviceName = name;
}

QtServiceController::~QtServiceController()
{
    delete d_ptr;
}

QString QtServiceController::serviceName() const
{
    Q_D(const QtServiceController);
    return d->serviceName;
}

bool QtServiceController::install(const QString &serviceFilePath, const QString &account,
                const QString &password)
{
    QStringList arguments;
    arguments << QLatin1String("-i");
    arguments << account;
    arguments << password;
    return (QProcess::execute(serviceFilePath, arguments) == 0);
}

bool QtServiceController::start()
{
    return start(QStringList());
}

void QtServiceBase::logMessage(const QString &message, QtServiceBase::MessageType type,
                            int, uint, const QByteArray &)
{
    if (!d_ptr->sysd)
        return;
    int st;
    switch(type) {
        case QtServiceBase::Error:
            st = LOG_ERR;
            break;
        case QtServiceBase::Warning:
            st = LOG_WARNING;
            break;
        default:
            st = LOG_INFO;
    }
    if (!d_ptr->sysd->ident) {
        QString tmp = encodeName(serviceName(), TRUE);
        int len = tmp.toLocal8Bit().size();
        d_ptr->sysd->ident = new char[len+1];
        d_ptr->sysd->ident[len] = '\0';
        ::memcpy(d_ptr->sysd->ident, tmp.toLocal8Bit().constData(), len);
    }
    openlog(d_ptr->sysd->ident, LOG_PID, LOG_DAEMON);
    foreach(QString line, message.split('\n'))
        syslog(st, "%s", line.toLocal8Bit().constData());
    closelog();
}

void QtServiceBase::setServiceFlags(QtServiceBase::ServiceFlags flags)
{
    if (d_ptr->serviceFlags == flags)
        return;
    d_ptr->serviceFlags = flags;
    if (d_ptr->sysd)
        d_ptr->sysd->serviceFlags = flags;
}

QtServiceBase *QtServiceBasePrivate::instance = 0;

QtServiceBasePrivate::QtServiceBasePrivate(const QString &name)
    : startupType(QtServiceController::ManualStartup), serviceFlags(0), controller(name)
{
}

QtServiceBasePrivate::~QtServiceBasePrivate()
{
}

void QtServiceBasePrivate::startService()
{
    q_ptr->start();
}

int QtServiceBasePrivate::run(bool asService, const QStringList &argList)
{
    int argc = argList.size();
    QVector<char *> argv(argc);
    QList<QByteArray> argvData;
    for (int i = 0; i < argc; ++i)
        argvData.append(argList.at(i).toLocal8Bit());
    for (int i = 0; i < argc; ++i)
        argv[i] = argvData[i].data();

    if (asService && !sysInit())
        return -1;

    q_ptr->createApplication(argc, argv.data());
    QCoreApplication *app = QCoreApplication::instance();
    if (!app)
        return -1;

    if (asService)
        sysSetPath();

    QtServiceStarter starter(this);
    QTimer::singleShot(0, &starter, SLOT(slotStart()));
    int res = q_ptr->executeApplication();
    delete app;

    if (asService)
        sysCleanup();
    return res;
}

bool QtServiceBasePrivate::sysInit()
{
    sysd = new QtServiceSysPrivate;
    sysd->serviceFlags = serviceFlags;
    // Restrict permissions on files that are created by the service
    ::umask(027);

    return true;
}

void QtServiceBasePrivate::sysSetPath()
{
    if (sysd)
        sysd->setPath(socketPath(controller.serviceName()));
}

void QtServiceBasePrivate::sysCleanup()
{
    if (sysd) {
        sysd->close();
        delete sysd;
        sysd = 0;
    }
}

bool QtServiceBasePrivate::start()
{
    if (sendCmd(controller.serviceName(), "alive")) {
        // Already running
        return false;
    }
    // Could just call controller.start() here, but that would fail if
    // we're not installed. We do not want to strictly require installation.
    ::setenv("QTSERVICE_RUN", "1", 1);  // Tell the detached process it's it
    return QProcess::startDetached(filePath(), args.mid(1), "/");
}

QString QtServiceBasePrivate::filePath() const
{
    QString ret;
    if (args.isEmpty())
        return ret;
    QFileInfo fi(args[0]);
    QDir dir(absPath(args[0]));
    return dir.absoluteFilePath(fi.fileName());
}

bool QtServiceBasePrivate::install(const QString &account, const QString &password)
{
    Q_UNUSED(account)
    Q_UNUSED(password)
    QSettings settings(QSettings::SystemScope, "QtSoftware");

    settings.beginGroup("services");
    settings.beginGroup(controller.serviceName());

    settings.setValue("path", filePath());
    settings.setValue("description", serviceDescription);
    settings.setValue("automaticStartup", startupType);

    settings.endGroup();
    settings.endGroup();
    settings.sync();

    QSettings::Status ret = settings.status();
    if (ret == QSettings::AccessError) {
        fprintf(stderr, "Cannot install \"%s\". Cannot write to: %s. Check permissions.\n",
                controller.serviceName().toLatin1().constData(),
                settings.fileName().toLatin1().constData());
    }
    return (ret == QSettings::NoError);
}

QtServiceBase::QtServiceBase(int argc, char **argv, const QString &name)
{
#if defined(QTSERVICE_DEBUG)
    qInstallMsgHandler(qtServiceLogDebug);
    qAddPostRoutine(qtServiceCloseDebugLog);
#endif

    Q_ASSERT(!QtServiceBasePrivate::instance);
    QtServiceBasePrivate::instance = this;

    QString nm(name);
    if (nm.length() > 255) {
	qWarning("QtService: 'name' is longer than 255 characters.");
	nm.truncate(255);
    }
    if (nm.contains('\\')) {
	qWarning("QtService: 'name' contains backslashes '\\'.");
	nm.replace((QChar)'\\', (QChar)'\0');
    }

    d_ptr = new QtServiceBasePrivate(nm);
    d_ptr->q_ptr = this;

    d_ptr->serviceFlags = 0;
    d_ptr->sysd = 0;
    for (int i = 0; i < argc; ++i)
        d_ptr->args.append(QString::fromLocal8Bit(argv[i]));
}

QtServiceBase::~QtServiceBase()
{
    delete d_ptr;
    QtServiceBasePrivate::instance = 0;
}


QString QtServiceBase::serviceName() const
{
    return d_ptr->controller.serviceName();
}

QString QtServiceBase::serviceDescription() const
{
    return d_ptr->serviceDescription;
}

void QtServiceBase::setServiceDescription(const QString &description)
{
    d_ptr->serviceDescription = description;
}

QtServiceController::StartupType QtServiceBase::startupType() const
{
    return d_ptr->startupType;
}

QString QtServiceController::serviceDescription() const
{
    QSettings settings(QSettings::SystemScope, "QtSoftware");
    settings.beginGroup("services");
    settings.beginGroup(serviceName());

    QString desc = settings.value("description").toString();

    settings.endGroup();
    settings.endGroup();

    return desc;
}

QtServiceController::StartupType QtServiceController::startupType() const
{
    QSettings settings(QSettings::SystemScope, "QtSoftware");
    settings.beginGroup("services");
    settings.beginGroup(serviceName());

    StartupType startupType = (StartupType)settings.value("startupType").toInt();

    settings.endGroup();
    settings.endGroup();

    return startupType;
}

QString QtServiceController::serviceFilePath() const
{
    QSettings settings(QSettings::SystemScope, "QtSoftware");
    settings.beginGroup("services");
    settings.beginGroup(serviceName());

    QString path = settings.value("path").toString();

    settings.endGroup();
    settings.endGroup();

    return path;
}

bool QtServiceController::uninstall()
{
    QSettings settings(QSettings::SystemScope, "QtSoftware");
    settings.beginGroup("services");

    settings.remove(serviceName());

    settings.endGroup();
    settings.sync();

    QSettings::Status ret = settings.status();
    if (ret == QSettings::AccessError) {
        fprintf(stderr, "Cannot uninstall \"%s\". Cannot write to: %s. Check permissions.\n",
                serviceName().toLatin1().constData(),
                settings.fileName().toLatin1().constData());
    }
    return (ret == QSettings::NoError);
}


bool QtServiceController::start(const QStringList &arguments)
{
    if (!isInstalled())
        return false;
    if (isRunning())
        return false;
    return QProcess::startDetached(serviceFilePath(), arguments);
}

bool QtServiceController::stop()
{
    return sendCmd(serviceName(), QLatin1String("terminate"));
}

bool QtServiceController::pause()
{
    return sendCmd(serviceName(), QLatin1String("pause"));
}

bool QtServiceController::resume()
{
    return sendCmd(serviceName(), QLatin1String("resume"));
}

bool QtServiceController::sendCommand(int code)
{
    return sendCmd(serviceName(), QString(QLatin1String("num:") + QString::number(code)));
}

bool QtServiceController::isInstalled() const
{
    QSettings settings(QSettings::SystemScope, "QtSoftware");
    settings.beginGroup("services");

    QStringList list = settings.childGroups();

    settings.endGroup();

    QStringListIterator it(list);
    while (it.hasNext()) {
        if (it.next() == serviceName())
            return true;
    }

    return false;
}

bool QtServiceController::isRunning() const
{
    QtUnixSocket sock;
    if (sock.connectTo(socketPath(serviceName())))
        return true;
    return false;
}

void QtServiceBase::setStartupType(QtServiceController::StartupType type)
{
    d_ptr->startupType = type;
}

QtServiceBase::ServiceFlags QtServiceBase::serviceFlags() const
{
    return d_ptr->serviceFlags;
}

int QtServiceBase::exec()
{
    if (d_ptr->args.size() > 1) {
        QString a =  d_ptr->args.at(1);
        if (a == QLatin1String("-i") || a == QLatin1String("-install")) {
            if (!d_ptr->controller.isInstalled()) {
                QString account;
                QString password;
                if (d_ptr->args.size() > 2)
                    account = d_ptr->args.at(2);
                if (d_ptr->args.size() > 3)
                    password = d_ptr->args.at(3);
                if (!d_ptr->install(account, password)) {
                    fprintf(stderr, "The service %s could not be installed\n", serviceName().toLatin1().constData());
                    return -1;
                } else {
                    printf("The service %s has been installed under: %s\n",
                        serviceName().toLatin1().constData(), d_ptr->filePath().toLatin1().constData());
                }
            } else {
                fprintf(stderr, "The service %s is already installed\n", serviceName().toLatin1().constData());
            }
            return 0;
        } else if (a == QLatin1String("-u") || a == QLatin1String("-uninstall")) {
            if (d_ptr->controller.isInstalled()) {
                if (!d_ptr->controller.uninstall()) {
                    fprintf(stderr, "The service %s could not be uninstalled\n", serviceName().toLatin1().constData());
                    return -1;
                } else {
                    printf("The service %s has been uninstalled.\n",
                        serviceName().toLatin1().constData());
                }
            } else {
                fprintf(stderr, "The service %s is not installed\n", serviceName().toLatin1().constData());
            }
            return 0;
        } else if (a == QLatin1String("-v") || a == QLatin1String("-version")) {
            printf("The service\n"
                "\t%s\n\t%s\n\n", serviceName().toLatin1().constData(), d_ptr->args.at(0).toLatin1().constData());
            printf("is %s", (d_ptr->controller.isInstalled() ? "installed" : "not installed"));
            printf(" and %s\n\n", (d_ptr->controller.isRunning() ? "running" : "not running"));
            return 0;
        } else if (a == QLatin1String("-e") || a == QLatin1String("-exec")) {
            d_ptr->args.removeAt(1);
            int ec = d_ptr->run(false, d_ptr->args);
            if (ec == -1)
                qErrnoWarning("The service could not be executed.");
            return ec;
        } else if (a == QLatin1String("-t") || a == QLatin1String("-terminate")) {
            if (!d_ptr->controller.stop())
                qErrnoWarning("The service could not be stopped.");
            return 0;
        } else if (a == QLatin1String("-p") || a == QLatin1String("-pause")) {
            d_ptr->controller.pause();
            return 0;
        } else if (a == QLatin1String("-r") || a == QLatin1String("-resume")) {
            d_ptr->controller.resume();
            return 0;
        } else if (a == QLatin1String("-c") || a == QLatin1String("-command")) {
            int code = 0;
            if (d_ptr->args.size() > 2)
                code = d_ptr->args.at(2).toInt();
            d_ptr->controller.sendCommand(code);
            return 0;
        } else  if (a == QLatin1String("-h") || a == QLatin1String("-help")) {
            printf("\n%s -[i|u|e|s|v|h]\n"
                   "\t-i(nstall) [account] [password]\t: Install the service, optionally using given account and password\n"
                   "\t-u(ninstall)\t: Uninstall the service.\n"
                   "\t-e(xec)\t\t: Run as a regular application. Useful for debugging.\n"
                   "\t-t(erminate)\t: Stop the service.\n"
                   "\t-c(ommand) num\t: Send command code num to the service.\n"
                   "\t-v(ersion)\t: Print version and status information.\n"
                   "\t-h(elp)   \t: Show this help\n"
                   "\tNo arguments\t: Start the service.\n",
                   d_ptr->args.at(0).toLatin1().constData());
            return 0;
        }
    }
#if defined(Q_OS_UNIX)
    if (::getenv("QTSERVICE_RUN")) {
        // Means we're the detached, real service process.
        int ec = d_ptr->run(true, d_ptr->args);
        if (ec == -1)
            qErrnoWarning("The service failed to run.");
        return ec;
    }
#endif
    if (!d_ptr->start()) {
        fprintf(stderr, "The service %s could not start\n", serviceName().toLatin1().constData());
        return -4;
    }
    return 0;
}

QtServiceBase *QtServiceBase::instance()
{
    return QtServiceBasePrivate::instance;
}

void QtServiceBase::stop()
{
}

void QtServiceBase::pause()
{
}

void QtServiceBase::resume()
{
}

void QtServiceBase::processCommand(int /*code*/)
{
}

QtServiceSysPrivate::QtServiceSysPrivate()
    : QtUnixServerSocket(), ident(0), serviceFlags(0)
{
}

QtServiceSysPrivate::~QtServiceSysPrivate()
{
    if (ident)
        delete[] ident;
}

void QtServiceSysPrivate::incomingConnection(int socketDescriptor)
{
    QTcpSocket *s = new QTcpSocket(this);
    s->setSocketDescriptor(socketDescriptor);
    connect(s, SIGNAL(readyRead()), this, SLOT(slotReady()));
    connect(s, SIGNAL(disconnected()), this, SLOT(slotClosed()));
}

void QtServiceSysPrivate::slotReady()
{
    QTcpSocket *s = (QTcpSocket *)sender();
    cache[s] += QString(s->readAll());
    QString cmd = getCommand(s);
    while (!cmd.isEmpty()) {
        bool retValue = false;
        if (cmd == QLatin1String("terminate")) {
            if (!(serviceFlags & QtServiceBase::CannotBeStopped)) {
                QtServiceBase::instance()->stop();
                QCoreApplication::instance()->quit();
                retValue = true;
            }
        } else if (cmd == QLatin1String("pause")) {
            if (serviceFlags & QtServiceBase::CanBeSuspended) {
                QtServiceBase::instance()->pause();
                retValue = true;
            }
        } else if (cmd == QLatin1String("resume")) {
            if (serviceFlags & QtServiceBase::CanBeSuspended) {
                QtServiceBase::instance()->resume();
                retValue = true;
            }
        } else if (cmd == QLatin1String("alive")) {
            retValue = true;
        } else if (cmd.length() > 4 && cmd.left(4) == QLatin1String("num:")) {
            cmd = cmd.mid(4);
            QtServiceBase::instance()->processCommand(cmd.toInt());
            retValue = true;
        }
        QString retString;
        if (retValue)
            retString = QLatin1String("true");
        else
            retString = QLatin1String("false");
        s->write(retString.toLatin1().constData());
        s->flush();
        cmd = getCommand(s);
    }
}

void QtServiceSysPrivate::slotClosed()
{
    QTcpSocket *s = (QTcpSocket *)sender();
    s->deleteLater();
}

QString QtServiceSysPrivate::getCommand(const QTcpSocket *socket)
{
    int pos = cache[socket].indexOf("\r\n");
    if (pos >= 0) {
        QString ret = cache[socket].left(pos);
        cache[socket].remove(0, pos+2);
        return ret;
    }
    return "";
}

#include "qtservice.moc"