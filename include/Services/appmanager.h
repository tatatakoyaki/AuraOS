#ifndef APPMANAGER_H
#define APPMANAGER_H

#include <QObject>
#include <QVariantList>
#include <QFileSystemWatcher>

/**
 * @brief AppManager (应用生命周期与桌面布局管家)
 *
 * 架构设计：
 * 1. 动态发现：通过 QFileSystemWatcher 实时监控应用安装目录，实现第三方应用的热插拔。
 * 2. 状态持久化：将桌面图标顺序持久化至本地 JSON 文件，保障重启后的布局一致性。
 * 3. 数据桥接：通过 QVariantList 与 QML 引擎进行数据绑定与列表渲染。
 */
class AppManager : public QObject
{
    Q_OBJECT
    // 暴露给 QML 的已安装应用列表
    Q_PROPERTY(QVariantList installedApps READ installedApps NOTIFY appsChanged)

public:
    explicit AppManager(QObject *parent = nullptr);

    /**
     * @brief 获取当前已安装的应用列表
     */
    QVariantList installedApps() const;

    /**
     * @brief 主动刷新应用列表（当检测到目录变动时触发）
     */
    Q_INVOKABLE void refreshApps();

    /**
     * @brief 供 QML 拖拽时调用的应用位置交换接口
     */
    Q_INVOKABLE void moveApp(int from, int to);

    /**
     * @brief 将 QML 传来的最终应用顺序同步落库
     */
    Q_INVOKABLE void syncLayout(const QStringList &appNames);

    /**
     * @brief 杀掉指定的后台应用进程
     */
    Q_INVOKABLE void killApp(const QString &appName);

signals:
    // 应用列表发生增删或顺序变化时触发，通知 UI 刷新
    void appsChanged();

private:
    QVariantList m_apps;               // 当前内存中的应用列表缓存
    QString m_appDir;                  // 应用存放的物理根目录
    QFileSystemWatcher *m_watcher;     // 文件系统监控器，用于实现热插拔

    /**
     * @brief 获取本地布局配置文件的绝对路径
     */
    QString getConfigPath();
};

#endif // APPMANAGER_H
