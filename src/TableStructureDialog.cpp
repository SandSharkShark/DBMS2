#include "TableStructureDialog.h"
#include <QVBoxLayout>
#include <QHeaderView>

TableStructureDialog::TableStructureDialog(
    const std::vector<ColumnDef>& columns, 
    QWidget* parent) : QDialog(parent) {
    
    setWindowTitle("表结构");
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    structureTable = new QTableWidget(this);
    structureTable->setColumnCount(4);
    structureTable->setHorizontalHeaderLabels(
        {"列名", "数据类型", "可空", "主键"});
    
    structureTable->setRowCount(columns.size());
    
    for (size_t i = 0; i < columns.size(); i++) {
        structureTable->setItem(i, 0, 
            new QTableWidgetItem(QString::fromStdString(columns[i].name)));
        structureTable->setItem(i, 1, 
            new QTableWidgetItem(QString::fromStdString(columns[i].type)));
        structureTable->setItem(i, 2, 
            new QTableWidgetItem(columns[i].nullable ? "是" : "否"));
        structureTable->setItem(i, 3, 
            new QTableWidgetItem(columns[i].primaryKey ? "是" : "否"));
    }
    
    structureTable->resizeColumnsToContents();
    layout->addWidget(structureTable);
    
    resize(400, 300);
} 