#ifndef WIDGETMANAGER_H
#define WIDGETMANAGER_H

#include <QObject>
#include <QVariantList>

/**
 * @brief WidgetManager (小组件排版业务管家)
 *
 * 架构设计：
 * 负责管理下拉控制中心内小组件 (Widgets) 的布局数据。
 * 采用 JSON 格式进行本地持久化 (读写配置文件)，
 * 前端通过 QVariantList 获取列表，并支持动态更新与保存。
 */
class WidgetManager : public QObject
{
    Q_OBJECT
    // 暴露给前端 QML 的小组件列表数据
    Q_PROPERTY(QVariantList widgets READ widgets NOTIFY widgetsChanged)

public:
    explicit WidgetManager(QObject *parent = nullptr);

    /**
     * @brief 获取当前已保存的小组件列表
     */
    QVariantList widgets() const;

    /**
     * @brief 保存前端拖拽调整后的小组件布局
     * @param newWidgets 包含组件类型、坐标、宽高信息的列表
     */
    Q_INVOKABLE void saveLayout(const QVariantList &newWidgets);

signals:
    // 小组件列表发生变更时触发，通知 UI 刷新
    void widgetsChanged();

private:
    QVariantList m_widgets; // 当前小组件排版数据的本地缓存
};

#endif // WIDGETMANAGER_H
