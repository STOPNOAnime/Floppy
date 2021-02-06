#include "ui_configwindow.h"
#include "mainwindow.h"

ConfigWindow::ConfigWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigWindow)
{
    ui->setupUi(this);

    bar = new QStatusBar(this);
    ui->barlayout->addWidget(bar);
    bar->setSizeGripEnabled(false);
    setWindowFlags(windowFlags() & Qt::MSWindowsFixedSizeDialogHint & ~Qt::WindowContextHelpButtonHint);

    ui->track_table->verticalHeader()->setVisible(false);
    ui->track_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->track_table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
}

ConfigWindow::~ConfigWindow()
{
    delete ui;
}

void ConfigWindow::initialize_track_assigment(){
    for(uint i=0; i < track_number; i++) track_assigment[i].clear();
    if(floppy_number>=track_number) for(uint i=0; i < floppy_number; i++) track_assigment[i%track_number].append(i);
    else for(uint i=0; i < track_number; i++) track_assigment[i].append(i%floppy_number);
}

void ConfigWindow::analyze_track_assigment(){
    bar->showMessage("");
    bar->setStyleSheet("background-color: rgba(0,0,0,0);");

    ui->track_table->setRowCount(0);

    for(uint i=0; i < track_number; i++){
        ui->track_table->insertRow(ui->track_table->rowCount());
        ui->track_table->setItem(ui->track_table->rowCount()-1,0,new QTableWidgetItem(QString::number(i)));
        QTableWidgetItem* item = ui->track_table->item(ui->track_table->rowCount()-1,0);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);

        ui->track_table->setItem(ui->track_table->rowCount()-1,1,new QTableWidgetItem(""));
        for(int j=0; j < track_assigment[i].size(); j++){
            ui->track_table->setItem(ui->track_table->rowCount()-1,1,new QTableWidgetItem(QString::number(track_assigment[i].at(j)) + " "));
        }
    }
}

void ConfigWindow::apply_track_assigment(){
    QVector<unsigned int> temp_track_assigment[16];
    uint tmp;
    bool ok;
    bool all_ok = true;

    for(uint i=0; i < track_number; i++){
        QStringList list = ui->track_table->item(i,1)->text().split(' ',Qt::SkipEmptyParts);

        for(int j=0; j < list.size(); j++){
            tmp = list[j].toUInt(&ok);
            if(ok == true && tmp < floppy_number){
                temp_track_assigment[i].append(tmp);
            }
            else{
                all_ok = false;
                break;
            }
        }
    }

    if(all_ok == true){
        for(uint i=0;i < 16; i++){
            track_assigment[i] = temp_track_assigment[i];
        }
        bar->showMessage("Sucesfully applied config.");
        bar->setStyleSheet("background-color: rgb(0, 255, 0);");
    }
    else{
        bar->showMessage("Invalid config. Failed to apply.");
        bar->setStyleSheet("background-color: rgb(255, 0, 0);");
    }
}

void ConfigWindow::on_close_button_clicked(){
    this->hide();
}

void ConfigWindow::on_default_button_clicked(){
    initialize_track_assigment();
    analyze_track_assigment();
    bar->showMessage("Sucesfully applied config.");
    bar->setStyleSheet("background-color: rgb(0, 255, 0);");
}

void ConfigWindow::on_apply_button_clicked(){
    apply_track_assigment();
}

void ConfigWindow::on_clear_button_clicked(){
    for(uint i=0; i < track_number; i++) track_assigment[i].clear();
    analyze_track_assigment();
    bar->showMessage("Sucesfully applied config.");
    bar->setStyleSheet("background-color: rgb(0, 255, 0);");
}
