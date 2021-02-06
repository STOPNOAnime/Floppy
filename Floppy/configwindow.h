#ifndef CONFIGWINDOW_H
#define CONFIGWINDOW_H

#include <QDialog>
#include <QScrollBar>
#include <QString>
#include <QVector>
#include <QStatusBar>

namespace Ui {
class ConfigWindow;
}

class ConfigWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigWindow(QWidget *parent = nullptr);
    ~ConfigWindow();
    void analyze_track_assigment();
    void initialize_track_assigment();
    QVector<unsigned int> track_assigment[16];
    uint floppy_number = 0;
    uint track_number = 0;

private slots:
    void on_close_button_clicked();
    void on_default_button_clicked();
    void on_apply_button_clicked();
    void on_clear_button_clicked();

private:
    void apply_track_assigment();

    Ui::ConfigWindow *ui;
    QStatusBar *bar;
};

#endif // CONFIGWINDOW_H
