#ifndef TABLEVIEWDIALOG_H
#define TABLEVIEWDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include "Table.h"
#include "DatabaseManager.h"

class TableViewDialog : public QDialog {
    Q_OBJECT
    
private:
    QTableWidget* tableWidget;
    DatabaseManager& dbManager;
    QString tableName;
    std::vector<ColumnDef> columns;
    
private slots:
    void saveChanges();
    void cellChanged(int row, int column);
    
public:
    TableViewDialog(const Table& table, DatabaseManager& dbManager, QWidget* parent = nullptr);
    
private:
    void refreshTableData();
    void setupUi();
    bool validateCell(int row, int column, const QString& value);
};

#endif 