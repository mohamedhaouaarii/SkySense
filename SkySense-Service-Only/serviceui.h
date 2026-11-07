#ifndef SERVICEUI_H
#define SERVICEUI_H
#include <functional>
#include <QString>
class QWidget;
QWidget *createSkySenseSidebar(std::function<void(const QString &)> navigate, const QString &initialSection = QStringLiteral("Services"));
void setupServiceContent(QWidget *root);
void setupServiceUi(QWidget *root);
bool exportServicesReportPdf(const QString &filePath, QString *error = nullptr);
#endif
