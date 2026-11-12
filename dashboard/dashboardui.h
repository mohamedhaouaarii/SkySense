#pragma once
#include <QWidget>
#include <functional>
#include <memory>

class DashboardPage final : public QWidget {
public:
    explicit DashboardPage(std::function<void(const QString &)> navigate, QWidget *parent = nullptr);
    ~DashboardPage() override;
    void refresh();
protected:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
private:
    struct Content;
    std::unique_ptr<Content> content;
};
