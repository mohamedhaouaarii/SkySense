#ifndef SERVICEUI_H
#define SERVICEUI_H
#include <QString>
class QWidget;

// Page "Services" integree : meme fenetre, meme en-tete et meme barre laterale que les autres gestions.
void setupServiceContent(QWidget *root);
bool exportServicesReportPdf(const QString &filePath, QString *error = nullptr);
#endif
