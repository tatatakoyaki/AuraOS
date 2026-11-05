#include "Services/appmanager.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

AppManager::AppManager(QObject *parent) : QObject{parent}
{
    m_appDir = QDir::homePath() + "/SmartIVI/apps";
    QDir().mkpath(m_appDir);

    // 实例化文件监控器，实现应用的热插拔（安装/卸载自动刷新）
    m_watcher = new QFileSystemWatcher(this);
    m_watcher->addPath(m_appDir);

    // 目录变动时触发重扫
    connect(m_watcher, &QFileSystemWatcher::directoryChanged, this, &AppManager::refreshApps);

    // 首次扫描并加载布局
    refreshApps();
}

QVariantList AppManager::installedApps() const { return m_apps; }

/**
 * @brief 获取持久化 JSON 配置文件的绝对路径
 */
QString AppManager::getConfigPath() {
    return QDir::homePath() + "/SmartIVI/layout.json";
}

/**
 * @brief 扫描应用目录并刷新应用列表
 *
 * 执行流程：
 * 1. 注入不可卸载的内置系统应用。
 * 2. 扫描外部目录，解析第三方应用的 manifest.json。
 * 3. 读取本地持久化的 JSON，恢复用户的拖拽排序。
 */
void AppManager::refreshApps()
{
    m_apps.clear();
    QDir dir(m_appDir);

    // ==========================================
    // 1. 注入内置系统应用（固定基础数据，不可被卸载）
    // ==========================================
    QVariantMap sysAc;
    sysAc["name"] = "空调";
    sysAc["icon"] = "❄️";
    sysAc["qmlPath"] = "system://ac";
    m_apps.append(sysAc);

    QVariantMap sysMedia;
    sysMedia["name"] = "音乐";
    sysMedia["icon"] = "🎵";
    sysMedia["qmlPath"] = "system://media";
    m_apps.append(sysMedia);

    QVariantMap sysSettings;
    sysSettings["name"] = "设置";
    sysSettings["icon"] = "⚙️";
    sysSettings["qmlPath"] = "system://settings";
    m_apps.append(sysSettings);

    // ==========================================
    // 2. 扫描外部目录，动态加载第三方应用
    // ==========================================
    QStringList appFolders = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &folderName : appFolders) {
        QString manifestPath = m_appDir + "/" + folderName + "/manifest.json";
        QFile file(manifestPath);

        if (file.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            QJsonObject obj = doc.object();

            QVariantMap appInfo;
            appInfo["name"] = obj["name"].toString();
            appInfo["icon"] = obj["icon"].toString();

            QString entryFile = obj["entry"].toString();
            if(entryFile.isEmpty()) entryFile = "Main.qml";
            appInfo["qmlPath"] = "file://" + m_appDir + "/" + folderName + "/" + entryFile;

            m_apps.append(appInfo);
            file.close();
        }
    }

    // ==========================================
    // 3. 读取本地 JSON 配置，恢复用户的自定义排序
    // ==========================================
    QFile configFile(getConfigPath());
    if (configFile.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(configFile.readAll());
        QJsonArray savedArray = doc.array();

        QVariantList sortedApps;
        QStringList savedNames;

        // 提取历史记录中的 App 名称顺序
        for (int i = 0; i < savedArray.size(); ++i) {
            savedNames.append(savedArray[i].toString());
        }

        // 核心排序算法：按照历史顺序，从当前扫描结果中查找并重新排列
        for (const QString &name : savedNames) {
            for (const QVariant &app : m_apps) {
                if (app.toMap()["name"].toString() == name) {
                    sortedApps.append(app);
                    break;
                }
            }
        }

        // 处理新增的 App（例如热安装的 App，不在历史记录中，直接追加到末尾）
        for (const QVariant &app : m_apps) {
            if (!savedNames.contains(app.toMap()["name"].toString())) {
                sortedApps.append(app);
            }
        }

        if (sortedApps.size() == m_apps.size()) {
            m_apps = sortedApps;
        }
        configFile.close();
    }

    qDebug() << "[AppManager] Scanned" << m_apps.size() << "apps. Layout restored.";
    emit appsChanged();
}

/**
 * @brief 内存层面移动应用顺序（供高频拖拽调用）
 *
 * 注意：特意去掉了 emit appsChanged()，防止拖拽过程中的高频重建
 * 导致 QML 端 MouseArea 丢失焦点或点击失效。
 */
void AppManager::moveApp(int from, int to)
{
    if (from < 0 || from >= m_apps.size() || to < 0 || to >= m_apps.size() || from == to) {
        return;
    }
    m_apps.move(from, to);
}

/**
 * @brief 静默持久化最终的布局顺序到本地 JSON 文件
 *
 * 由前端在拖拽松手后统一调用，避免高频 IO 操作。
 * @param appNames 最终排序后的应用名称列表
 */
void AppManager::syncLayout(const QStringList &appNames)
{
    if (appNames.size() != m_apps.size()) return;

    QVariantList newApps;
    // 根据前端传来的名称顺序重新组装内部数组
    for (const QString &name : appNames) {
        for (const QVariant &app : m_apps) {
            if (app.toMap()["name"].toString() == name) {
                newApps.append(app);
                break;
            }
        }
    }

    if (newApps.size() == m_apps.size()) {
        m_apps = newApps;

        // 序列化为 JSON 数组进行持久化
        QJsonArray arrayToSave;
        for (const QVariant &app : m_apps) {
            arrayToSave.append(app.toMap()["name"].toString());
        }

        QFile file(getConfigPath());
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            file.write(QJsonDocument(arrayToSave).toJson());
            file.close();
            qDebug() << "[AppManager] Layout saved to JSON successfully.";
        } else {
            qWarning() << "[AppManager] Failed to save layout to JSON!";
        }
    }
}

/**
 * @brief 清理指定应用的后台进程
 *
 * 真实车机环境：通过 Linux 系统调用 (kill/signal) 或给 Android 虚拟机发指令，
 * 强制销毁对应的 App 进程。
 */
void AppManager::killApp(const QString &appName)
{
    qDebug() << "[AppManager] Kill process request received for:" << appName;
}
