#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QScrollBar>
#include <QString>
#include <QTimer>
#include <QVector>
#include <QFileDialog>
#include <QTextStream>
#include <cmath>

#include <configwindow.h>
#include <midifile/MidiFile.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void clear_status_color(QString message);
    void midi_player();
    void recived_data();
    void set_can_connect();

    void on_connect_button_clicked();
    void on_refresh_button_clicked();
    void on_config_button_clicked();
    void on_play_button_clicked();
    void on_stop_button_clicked();
    void on_select_file_button_clicked();

private:
    void set_widget_visibility(bool player, bool rest);
    void display_message(QString message, bool good);
    void disconnect_floppy();

    Ui::MainWindow *ui;
    QSerialPort *serial;
    QTimer *oneshot;
    ConfigWindow *configwindow;

    smf::MidiFile midifile;
    uint current_event = 0;
    QString file_name;

    bool playing_song = false;
    bool can_connect = true;
    bool connected = false;
};
#endif // MAINWINDOW_H
