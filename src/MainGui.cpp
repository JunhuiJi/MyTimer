#include "MainGui.h"
#include "ui_MainGui.h"
#include <QDate>
//#include <QFileSystemWatcher>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
//#include <QDebug>

MainGui::MainGui(QWidget *parent) :
    QMainWindow(parent),
    ui_(new Ui::MainGui),
    model_(new QSqlTableModel(parent,QSqlDatabase::database("connection1"))),
    timer_(new QTimer(this))
{
    ui_->setupUi(this);

    ui_->startButton->setDefault(true);
    ui_->saveButton->setDefault(false);
    ui_->startButton->setFocus();

    ui_->dateEdit->setDate(QDate::currentDate());
    ui_->lcdNumber->display("00:00:00");

    model_->setTable("Database");
    updateModel_();
    model_->setEditStrategy(QSqlTableModel::OnFieldChange);

    ui_->tableView->setModel(model_);

//    QFileSystemWatcher *watcher = new QFileSystemWatcher(parent);
//    watcher->addPath("Database.db");
//    connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(onDatabaseChanged_(QString)));

    selectionModel_ = ui_->tableView->selectionModel();
    connect(selectionModel_, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(tempSlot_(QItemSelection,QItemSelection)));
//    connect(this, SIGNAL(tempSignal()), this, SLOT(setTableViewEditable_()));
    connect(this, SIGNAL(tempSignal()), this, SLOT(calculateTime_()));

    connect(timer_, SIGNAL(timeout()), this, SLOT(updateLcdNumber_()));

    countDownOver_ = false;

    loadSettings();
}

MainGui::~MainGui()
{
    delete ui_;
    delete model_;
    delete selectionModel_;
    delete timer_;
}

//before closing mainGui, save the settings
void MainGui::closeEvent(QCloseEvent *event)
{
    saveSettings();
    event->accept();
}

void MainGui::on_startButton_clicked()
{
    if (ui_->startButton->text() == tr("开始")) {
        int countDownSeconds = ui_->countDownTimeSpinBox->value();
        int hour = countDownSeconds / 3600;
        int minute = (countDownSeconds % 3600) / 60;
        int second = ((countDownSeconds % 3600) % 60);
        ui_->lcdNumber->display(QString("%1:%2:%3").arg(hour, 2, 10, QLatin1Char('0'))
                                .arg(minute, 2, 10, QLatin1Char('0'))
                                .arg(second, 2, 10, QLatin1Char('0')));

        setupUi_(1);

        startingDateTime_ = QDateTime::currentDateTime();
        countDownOver_ = false;
        timer_->start(1000);
    }
    else {
        timer_->stop();
        countDownOver_ = false;
        setupUi_(0);
    }

    return;
}

void MainGui::on_saveButton_clicked()
{
    timer_->stop();

    if (countDownOver_ == true) {
        endDateTime_ = QDateTime::currentDateTime();
        countDownOver_ = false;

        QSqlDatabase db1 = QSqlDatabase::database("connection1");
        QSqlQuery query1(db1);
        query1.prepare("INSERT INTO Database ("
                       "startingDate, startingTime, "
                       "endDate, endTime, "
                       "goThroughTime) "
                       "values (?, ?, ?, ?, ?)");
        QString startingDate = startingDateTime_.toString("yyyy-MM-dd");
        QString startingTime = startingDateTime_.toString("hh:mm:ss");
        QString endDate = endDateTime_.toString("yyyy-MM-dd");
        QString endTime = endDateTime_.toString("hh:mm:ss");
        int countUpSeconds = startingDateTime_.secsTo(endDateTime_) - ui_->countDownTimeSpinBox->value();
        int hour = countUpSeconds / 3600;
        int minute = (countUpSeconds % 3600) / 60;
        int second = ((countUpSeconds % 3600) % 60);
        QString goThroughTime = QString("%1:%2:%3").arg(hour, 2, 10, QLatin1Char('0'))
                .arg(minute, 2, 10, QLatin1Char('0'))
                .arg(second, 2, 10, QLatin1Char('0'));
        query1.addBindValue(startingDate);
        query1.addBindValue(startingTime);
        query1.addBindValue(endDate);
        query1.addBindValue(endTime);
        query1.addBindValue(goThroughTime);
        query1.exec();

        onRecordInserted_();

        setupUi_(0);
    }
    return;
}

//filter the table by date
void MainGui::on_filterButton_clicked()
{
    model_->setFilter(QString("startingDate = '%1'").arg(ui_->dateEdit->text()));
    updateModel_();
    return;
}

//display the whole table
void MainGui::on_displayAllButton_clicked()
{
    model_->setTable("Database");
    updateModel_();
    return;
}

//delete selected row(s)
void MainGui::on_deleteRowsButton_clicked()
{
    if (currentRows_().isEmpty()) {
        QMessageBox::critical(0, QObject::tr("选择错误"),
                              QObject::tr("必须选中完整的一行或多行"), QMessageBox::Close);
        return;
    }
    else {
        QMessageBox msgBox;
        msgBox.setWindowTitle(QObject::tr("删除确认对话框"));
        msgBox.setText(QObject::tr("是否删除？"));
        msgBox.setIcon(QMessageBox::Question);
        QPushButton *yes = msgBox.addButton(QObject::tr("是"), QMessageBox::YesRole);
        QPushButton *no = msgBox.addButton(QObject::tr("否"), QMessageBox::NoRole);
        msgBox.setDefaultButton(no);
        msgBox.exec();
        if (msgBox.clickedButton() == yes) {
            QModelIndex index;
            QModelIndexList list = currentRows_();
            int i = 0;
            qSort(list.begin(),list.end());
            foreach (index, list) {
                model_->removeRow(index.row() - i);
                i++;
            }
            return;
        }
        else if (msgBox.clickedButton() == no)
            return;
    }
}

//void MainGui::onDatabaseChanged_(const QString)
//{
//    updateModel_();
//    selectTodayRows_();
//    calculateTime_();
//    return;
//}

void MainGui::onRecordInserted_()
{
    updateModel_();
    setTableViewEditable_();
    selectTodayRows_();
    calculateTime_();
    ui_->tableView->scrollToBottom();
    return;
}

void MainGui::tempSlot_(QItemSelection, QItemSelection)
{
    emit tempSignal();
}

void MainGui::setTableViewEditable_()
{
    if (ui_->setTableViewEditableCheckBox->isChecked())
        ui_->tableView->setEditTriggers(QAbstractItemView::DoubleClicked);
    else
        ui_->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

//calculate overall time of current selected row(s)
void MainGui::calculateTime_()
{
    if (ui_->calculateTimeCheckBox->isChecked()) {
        if (currentRows_().isEmpty()) {
            ui_->overallTimeLabel->setText(QObject::tr("0分钟"));
            return;
        }
        else {
            QModelIndex index;
            QModelIndexList list = currentRows_();
            int hour;
            int minute;
            int second;
            double overallMinute = 0;
            foreach (index, list) {
                hour = model_->record(index.row()).value("goThroughTime").toString().section(':',0,0).toInt();
                minute = model_->record(index.row()).value("goThroughTime").toString().section(':',1,1).toInt();
                second = model_->record(index.row()).value("goThroughTime").toString().section(':',2,2).toInt();
                overallMinute += hour * 60.0 + minute + second / 60.0;
            }
            ui_->overallTimeLabel->setText(QObject::tr("%1分钟").arg(overallMinute));
            return;
        }
    }
    else {
        ui_->overallTimeLabel->setText(QObject::tr("NULL"));
        return;
    }
}

void MainGui::on_setTableViewEditableCheckBox_clicked()
{
    setTableViewEditable_();
}

void MainGui::on_selectTodayRowsCheckBox_clicked()
{
    selectTodayRows_();
    return;
}

void MainGui::on_calculateTimeCheckBox_clicked()
{
    calculateTime_();
    return;
}

void MainGui::updateLcdNumber_()
{
    startCountDown_();
    startCountUp_();
    return;
}

//return current selected row(s)
QModelIndexList MainGui::currentRows_()
{
    return selectionModel_->selectedRows();
}

void MainGui::selectTodayRows_()
{
    if (ui_->selectTodayRowsCheckBox->isChecked()) {
        model_->setFilter(QString("startingDate = '%1'").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd")));
        updateModel_();
        ui_->tableView->selectAll();
    }
    else
        on_displayAllButton_clicked();
    return;
}

void MainGui::startCountDown_()
{
    if (countDownOver_ == true)
        return;
    else {
        QDateTime currentDateTime = QDateTime::currentDateTime();
        int goThroughSeconds = startingDateTime_.secsTo(currentDateTime);
        int countDownSeconds = ui_->countDownTimeSpinBox->value() - goThroughSeconds;
        if (countDownSeconds >= 0) {
            int hour = countDownSeconds / 3600;
            int minute = (countDownSeconds % 3600) / 60;
            int second = ((countDownSeconds % 3600) % 60);
            ui_->lcdNumber->display(QString("%1:%2:%3").arg(hour, 2, 10, QLatin1Char('0'))
                                    .arg(minute, 2, 10, QLatin1Char('0'))
                                    .arg(second, 2, 10, QLatin1Char('0')));
            return;
        }
        else {
            countDownOver_ = true;
            setupUi_(2);
            return;
        }
    }
}

void MainGui::startCountUp_()
{
    if (countDownOver_ == true) {
        QDateTime currentDateTime = QDateTime::currentDateTime();
        int countUpSeconds = startingDateTime_.secsTo(currentDateTime) - ui_->countDownTimeSpinBox->value();
        int hour = countUpSeconds / 3600;
        int minute = (countUpSeconds % 3600) / 60;
        int second = ((countUpSeconds % 3600) % 60);
        ui_->lcdNumber->display(QString("%1:%2:%3").arg(hour, 2, 10, QLatin1Char('0'))
                                .arg(minute, 2, 10, QLatin1Char('0'))
                                .arg(second, 2, 10, QLatin1Char('0')));
    }
    else
        return;
}

void MainGui::updateModel_()
{
    model_->select();
    model_->setHeaderData(0, Qt::Horizontal, QObject::tr("开始日期"));
    model_->setHeaderData(1, Qt::Horizontal, QObject::tr("开始时间"));
    model_->setHeaderData(2, Qt::Horizontal, QObject::tr("结束日期"));
    model_->setHeaderData(3, Qt::Horizontal, QObject::tr("结束时间"));
    model_->setHeaderData(4, Qt::Horizontal, QObject::tr("经过时间"));
}

void MainGui::loadSettings()
{
    QSqlDatabase db1 = QSqlDatabase::database("connection1");
    QSqlQuery query1(db1);
    query1.exec("SELECT * FROM Settings");
    //    if there is at least one record in table Settings
    if (query1.seek(0)) {
        QSqlRecord record1 = query1.record();
        int countDownTime = record1.value(0).toInt();
        int setTableViewEditable = record1.value(1).toInt();
        int selectTodayRows = record1.value(2).toInt();
        int calculateTime = record1.value(3).toInt();
        int windowHeight = record1.value(4).toInt();
        int windowWidth = record1.value(5).toInt();

        ui_->countDownTimeSpinBox->setValue(countDownTime);
        if (setTableViewEditable == 0)
            ui_->setTableViewEditableCheckBox->setChecked(false);
        else
            ui_->setTableViewEditableCheckBox->setChecked(true);
        if (selectTodayRows == 0)
            ui_->selectTodayRowsCheckBox->setChecked(false);
        else
            ui_->selectTodayRowsCheckBox->setChecked(true);
        if (calculateTime == 0)
            ui_->calculateTimeCheckBox->setChecked(false);
        else
            ui_->calculateTimeCheckBox->setChecked(true);
        this->resize(windowWidth, windowHeight);
    }
    else
        return;
}

void MainGui::saveSettings()
{
    QSqlDatabase db1 = QSqlDatabase::database("connection1");
    QSqlQuery query1(db1);
    query1.exec("DELETE FROM Settings");
    query1.prepare("INSERT INTO Settings ("
                   "countDownTime, setTableViewEditable, "
                   "selectTodayRows, calculateTime, "
                   "windowHeight, windowWidth)"
                   "values (?, ?, ?, ?, ?, ?)");
    QString countDownTime = QString("%1").arg(ui_->countDownTimeSpinBox->value());
    QString setTableViewEditable;
    if (ui_->setTableViewEditableCheckBox->isChecked())
        setTableViewEditable = "1";
    else
        setTableViewEditable = "0";
    QString selectTodayRows;
    if (ui_->selectTodayRowsCheckBox->isChecked())
        selectTodayRows = "1";
    else
        selectTodayRows = "0";
    QString calculateTime;
    if (ui_->calculateTimeCheckBox->isChecked())
        calculateTime = "1";
    else
        calculateTime = "0";
    QString windowHeight = QString("%1").arg(this->size().height());
    QString windowWidth = QString("%1").arg(this->size().width());
    query1.addBindValue(countDownTime);
    query1.addBindValue(setTableViewEditable);
    query1.addBindValue(selectTodayRows);
    query1.addBindValue(calculateTime);
    query1.addBindValue(windowHeight);
    query1.addBindValue(windowWidth);
    query1.exec();
}

void MainGui::setupUi_(int state)
{
    //    state 0 means that timer has stopped
    //    state 1 means that timer has started, but counting down hasn't stopped
    //    state 2 means that timer has started, and counting down has stopped, counting up has started

    switch (state) {

    case 0 :
        ui_->lcdNumber->display("00:00:00");
        ui_->countDownTimeLabel->setEnabled(true);
        ui_->countDownTimeSpinBox->setEnabled(true);
        ui_->startButton->setText(QObject::tr("开始"));
        ui_->saveButton->setEnabled(false);
        ui_->dateEdit->setEnabled(true);
        ui_->filterButton->setEnabled(true);
        ui_->displayAllButton->setEnabled(true);
        ui_->deleteRowsButton->setEnabled(true);
        ui_->tableView->setEnabled(true);
        ui_->setTableViewEditableCheckBox->setEnabled(true);
        ui_->selectTodayRowsCheckBox->setEnabled(true);
        ui_->calculateTimeCheckBox->setEnabled(true);
        ui_->overallTimeLabel->setEnabled(true);
        ui_->startButton->setDefault(true);
        ui_->saveButton->setDefault(false);
        ui_->startButton->setFocus();
        break;

    case 1 :
        ui_->countDownTimeLabel->setEnabled(false);
        ui_->countDownTimeSpinBox->setEnabled(false);
        ui_->startButton->setText(QObject::tr("停止"));
        ui_->saveButton->setEnabled(false);
        ui_->dateEdit->setEnabled(false);
        ui_->filterButton->setEnabled(false);
        ui_->displayAllButton->setEnabled(false);
        ui_->deleteRowsButton->setEnabled(false);
        ui_->tableView->setEnabled(false);
        ui_->setTableViewEditableCheckBox->setEnabled(false);
        ui_->selectTodayRowsCheckBox->setEnabled(false);
        ui_->calculateTimeCheckBox->setEnabled(false);
        ui_->overallTimeLabel->setEnabled(false);
        ui_->startButton->setDefault(true);
        ui_->saveButton->setDefault(false);
        ui_->startButton->setFocus();
        break;

    case 2 :
        ui_->countDownTimeLabel->setEnabled(false);
        ui_->countDownTimeSpinBox->setEnabled(false);
        ui_->startButton->setText(QObject::tr("停止"));
        ui_->saveButton->setEnabled(true);
        ui_->dateEdit->setEnabled(false);
        ui_->filterButton->setEnabled(false);
        ui_->displayAllButton->setEnabled(false);
        ui_->deleteRowsButton->setEnabled(false);
        ui_->tableView->setEnabled(false);
        ui_->setTableViewEditableCheckBox->setEnabled(false);
        ui_->selectTodayRowsCheckBox->setEnabled(false);
        ui_->calculateTimeCheckBox->setEnabled(false);
        ui_->overallTimeLabel->setEnabled(false);
        ui_->startButton->setDefault(false);
        ui_->saveButton->setDefault(true);
        ui_->saveButton->setFocus();
        break;

    default :
        break;
    }
}
