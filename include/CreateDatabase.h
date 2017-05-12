#ifndef CREATEDATABASE_H
#define CREATEDATABASE_H

#include <QSqlDatabase>
#include <QMessageBox>
#include <QSqlQuery>

//create a database, a table and test connection
static bool createDatabase()
{
    QSqlDatabase db1 = QSqlDatabase::addDatabase("QSQLITE", "connection1");
    db1.setDatabaseName("Database.db");
    if (!db1.open())
    {
        QMessageBox::critical(0, "Cannot open database",
                              "Unable to establish a database connection.", QMessageBox::Close);
        return false;
    }
    QSqlQuery query1(db1);
    query1.exec("CREATE TABLE Database ( "
                "startingDate TEXT, "
                "startingTime TEXT, "
                "endDate TEXT, "
                "endTime TEXT, "
                "goThroughTime TEXT )");
    query1.exec("CREATE TABLE Settings ( "
                "countDownTime TEXT, "
                "setTableViewEditable TEXT, "
                "selectTodayRows TEXT, "
                "calculateTime TEXT, "
                "windowHeight TEXT, "
                "windowWidth TEXT)");
    return true;
}

#endif // CREATEDATABASE_H
