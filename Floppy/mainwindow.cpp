#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    set_widget_visibility(false,false);
    ui->midi_info_text->setReadOnly(true);
    ui->statusbar->setSizeGripEnabled(false);
    setWindowFlags(Qt::Window | Qt::MSWindowsFixedSizeDialogHint);

    connect(ui->statusbar, &QStatusBar::messageChanged, this, &MainWindow::clear_status_color);

    serial = new QSerialPort(this);
    serial->setBaudRate(115200);
    connect(serial, &QSerialPort::readyRead, this, &MainWindow::recived_data);
    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()){
        ui->serial_port_list->addItem(serialPortInfo.portName());
    }

    oneshot = new QTimer(this);
    oneshot->setSingleShot(true);
    connect(oneshot, SIGNAL(timeout()), SLOT(midi_player()));

    configwindow = new ConfigWindow();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::clear_status_color(QString message){
    if(message.isEmpty()) ui->statusbar->setStyleSheet("background-color: rgba(0,0,0,0);");
}

void MainWindow::set_widget_visibility(bool player, bool rest){
    ui->play_button->setEnabled(player);
    ui->stop_button->setEnabled(player);
    ui->song_progress->setEnabled(player);
    ui->config_button->setEnabled(player);

    ui->select_file_button->setEnabled(rest);
    ui->midi_info_text->setEnabled(rest);
}

void MainWindow::display_message(QString message, bool good){
    ui->statusbar->showMessage(message, 3000);
    if(good == true) ui->statusbar->setStyleSheet("background-color: rgb(0, 255, 0);");
    else ui->statusbar->setStyleSheet("background-color: rgb(255, 0, 0);");
}

void MainWindow::disconnect_floppy(){
    QByteArray data;
    data.append((char)0x1F);
    serial->write(data);
    serial->close();

    ui->song_progress->setValue(0);
    set_widget_visibility(false, false);
    oneshot->stop();

    ui->connect_button->setText("Connect");

    connected = false;
}

void MainWindow::recived_data(){
    QByteArray data;
    if(connected == false){
        data = serial->readAll();
        if(data.at(0) == (char)0x00 || data.size() != 1 || data.at(0) > 15) {
            disconnect_floppy();
            display_message("Error in initialization.",false);
        }
        else{
            configwindow->floppy_number = data.at(0);
            connected = true;
            display_message("Sucesfully connected", true);
            ui->connect_button->setText("Disconect");
            set_widget_visibility(false,true);
        }
    }
    else{
        disconnect_floppy();
        display_message("Error in comunication.", false);
    }
}

void MainWindow::set_can_connect() {
    can_connect = true;
}

void MainWindow::on_connect_button_clicked(){
    QByteArray data;
    if(connected == false){
        if(can_connect == true){
            can_connect = false;
            QTimer::singleShot(1000, this, &MainWindow::set_can_connect);

            serial->close();
            serial->setPortName(ui->serial_port_list->currentText());

            if(!serial->open(QIODevice::ReadWrite)) {
                display_message("Error opening serial port.",false);
            }
            else {
                data.append((char)0x0F);
                serial->write(data);
            }
        }
    }
    else{
        disconnect_floppy();
        display_message("Sucesfully disconnected", true);
    }
}

void MainWindow::on_refresh_button_clicked()
{
    ui->serial_port_list->clear();
    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()){
        ui->serial_port_list->addItem(serialPortInfo.portName());
    }
}

void MainWindow::on_config_button_clicked()
{
    configwindow->analyze_track_assigment();
    configwindow->show();
}


void MainWindow::midi_player(){
    QByteArray serial_data;
    bool song_ended = false;

    do{
        if(midifile[0][current_event].isNoteOn()) {
            for(int i = 0; i < configwindow->track_assigment[midifile[0][current_event].track].size(); i++){
                serial_data.append((0x30 + (configwindow->track_assigment[midifile[0][current_event].track].at(i))));
                serial_data.append(midifile[0][current_event].getKeyNumber());
            }
        }
        if(midifile[0][current_event].isNoteOff()) {
            for(int i = 0; i < configwindow->track_assigment[midifile[0][current_event].track].size(); i++){
                serial_data.append((0x20 + (configwindow->track_assigment[midifile[0][current_event].track].at(i))));
            }
        }
        current_event++;
        if(current_event >= midifile[0].size()) {
            serial_data.append(0x1F);
            song_ended = true;
            break;
        }
    }while(midifile[0][current_event-1].tick == midifile[0][current_event].tick);

    ui->song_progress->setValue(midifile[0][current_event-1].tick);

    serial->write(serial_data);

    if(song_ended == false){
        oneshot->start((midifile.getTimeInSeconds(0,current_event) - midifile.getTimeInSeconds(0,current_event-1))*1000);
    }
}

void MainWindow::on_select_file_button_clicked()
{
    QString string;
    QTextStream stream(&string);

    file_name = QFileDialog::getOpenFileName(this, tr("Open Midi File"), QDir::currentPath(), tr("Midi Files (*.midi *.mid)"));

    if(file_name != "") {
        set_widget_visibility(true,true);

        midifile.read(file_name.toStdString());
        midifile.doTimeAnalysis();
        stream<<"TPQ: "<<midifile.getTicksPerQuarterNote()<<Qt::endl;
        configwindow->track_number = midifile.getTrackCount();
        configwindow->initialize_track_assigment();
        stream<<"Tracks: "<<configwindow->track_number<<Qt::endl;
        midifile.joinTracks();
        stream<<"Total Time: "<<std::round(midifile.getTimeInSeconds(0,midifile[0].size()-1))<<"s"<<Qt::endl;

        ui->song_progress->setMinimum(0);
        ui->song_progress->setMaximum(midifile[0][midifile[0].size()-1].tick);
    }
    else {
        set_widget_visibility(false,true);
    }

    ui->midi_info_text->setPlainText(string);
}

void MainWindow::on_play_button_clicked()
{
    if(playing_song == false){
        set_widget_visibility(true,false);
        current_event = 0;
        midi_player();
    }
}

void MainWindow::on_stop_button_clicked()
{
    set_widget_visibility(true,true);

    playing_song = false;
    oneshot->stop();

    ui->song_progress->setValue(0);

    QByteArray data;
    data.append((char)0x1F);
    serial->write(data);
}


