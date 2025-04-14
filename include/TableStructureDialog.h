#ifndef TABLESTRUCTUREDIALOG_H
#define TABLESTRUCTUREDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include "Table.h"

class TableStructureDialog : public QDialog {
    Q_OBJECT
    
private:
    QTableWidget* structureTable;
    
public:
    TableStructureDialog(const std::vector<ColumnDef>& columns, QWidget* parent = nullptr);
};

#endif 