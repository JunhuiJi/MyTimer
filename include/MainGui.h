#ifndef MAINGUI_H
#define MAINGUI_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QString>
#include <QItemSelection>
#include <QSqlTableModel>
#include <QItemSelectionModel>
#include <QTimer>
#include <QDateTime>
#include <QModelIndex>

namespace Ui {
class MainGui;
}

class MainGui : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainGui(QWidget *parent = 0);
    ~MainGui();

protected:
    void closeEvent(QCloseEvent *event);

signals:
    void tempSignal();

private slots:
    void on_startButton_clicked();
    void on_saveButton_clicked();
    void on_filterButton_clicked();
    void on_displayAllButton_clicked();
    void on_deleteRowsButton_clicked();
//    void onDatabaseChanged_(const QString);
    void onRecordInserted_();
    void tempSlot_(QItemSelection, QItemSelection);
    void setTableViewEditable_();
    void calculateTime_();
    void on_setTableViewEditableCheckBox_clicked();
    void on_selectTodayRowsCheckBox_clicked();
    void on_calculateTimeCheckBox_clicked();
    void updateLcdNumber_();

private:
    Ui::MainGui *ui_;
    QSqlTableModel *model_;
    QItemSelectionModel *selectionModel_;
    QTimer *timer_;
    QDateTime startingDateTime_;
    QDateTime endDateTime_;
    bool countDownOver_;
    QModelIndexList currentRows_();
    void selectTodayRows_();
    void startCountDown_();
    void startCountUp_();
    void updateModel_();
    void loadSettings();
    void saveSettings();
    void setupUi_(int state);
};

#endif // MAINGUI_H
