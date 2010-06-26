#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include <QDialog>

namespace Ui {
    class ConnectionDialog;
}

class ConnectionDialog : public QDialog {
    Q_OBJECT

public:
    ConnectionDialog(QWidget *parent);
    ~ConnectionDialog();

private:
    Ui::ConnectionDialog *ui;

private slots:
    void on_connectButton_clicked();
};

#endif // CONNECTIONDIALOG_H
