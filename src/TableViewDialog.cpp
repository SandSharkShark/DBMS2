#include "TableViewDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QHeaderView>
#include <QToolBar>

TableViewDialog::TableViewDialog(const Table& table, DatabaseManager& manager, QWidget* parent)
    : QDialog(parent), dbManager(manager), tableName(QString::fromStdString(table.getName())),
      columns(table.getColumns()) {
    setupUi();
    refreshTableData();
}

void TableViewDialog::setupUi() {
    setWindowTitle("编辑表数据 - " + tableName);
    resize(800, 600);
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    // 创建表格
    tableWidget = new QTableWidget(this);
    tableWidget->setColumnCount(columns.size());
    
    // 设置表头
    QStringList headers;
    for (const auto& col : columns) {
        headers << QString::fromStdString(col.name);
    }
    tableWidget->setHorizontalHeaderLabels(headers);
    
    // 允许编辑
    tableWidget->setEditTriggers(QAbstractItemView::DoubleClicked | 
                                QAbstractItemView::EditKeyPressed |
                                QAbstractItemView::AnyKeyPressed);
    
    // 设置选择模式
    tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectItems);
    
    // 设置表格属性
    tableWidget->horizontalHeader()->setStretchLastSection(true);
    tableWidget->verticalHeader()->setVisible(true);
    tableWidget->setAlternatingRowColors(true);  // 交替行颜色
    
    // 创建按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    
    // 添加工具按钮
    QToolBar* toolBar = new QToolBar(this);
    toolBar->setIconSize(QSize(16, 16));
    
    // 添加行按钮
    QPushButton* addRowButton = new QPushButton("添加行", this);
    addRowButton->setIcon(QIcon::fromTheme("list-add"));
    
    // 删除行按钮
    QPushButton* deleteRowButton = new QPushButton("删除行", this);
    deleteRowButton->setIcon(QIcon::fromTheme("list-remove"));
    
    // 保存按钮
    QPushButton* saveButton = new QPushButton("保存", this);
    saveButton->setIcon(QIcon::fromTheme("document-save"));
    
    // 关闭按钮
    QPushButton* closeButton = new QPushButton("关闭", this);
    closeButton->setIcon(QIcon::fromTheme("window-close"));
    
    buttonLayout->addWidget(addRowButton);
    buttonLayout->addWidget(deleteRowButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(closeButton);
    
    layout->addWidget(toolBar);
    layout->addWidget(tableWidget);
    layout->addLayout(buttonLayout);
    
    // 连接信号槽
    connect(saveButton, &QPushButton::clicked, this, &TableViewDialog::saveChanges);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);
    connect(tableWidget, &QTableWidget::cellChanged, this, &TableViewDialog::cellChanged);
    connect(addRowButton, &QPushButton::clicked, this, [this]() {
        int row = tableWidget->rowCount();
        tableWidget->insertRow(row);
        for (int col = 0; col < tableWidget->columnCount(); ++col) {
            QTableWidgetItem* item = new QTableWidgetItem("");
            item->setFlags(item->flags() | Qt::ItemIsEditable);  // 确保可编辑
            tableWidget->setItem(row, col, item);
        }
    });
    
    connect(deleteRowButton, &QPushButton::clicked, this, [this]() {
        QList<QTableWidgetItem*> items = tableWidget->selectedItems();
        if (items.isEmpty()) {
            QMessageBox::warning(this, "警告", "请先选择要删除的单元格");
            return;
        }
        
        if (QMessageBox::question(this, "确认", "确定要删除选中的行吗？",
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            QSet<int> rowsToDelete;
            for (QTableWidgetItem* item : items) {
                rowsToDelete.insert(item->row());
            }
            
            // 使用新的方法创建QList
            QList<int> rows(rowsToDelete.begin(), rowsToDelete.end());
            std::sort(rows.begin(), rows.end(), std::greater<int>());
            for (int row : rows) {
                tableWidget->removeRow(row);
            }
        }
    });
}

void TableViewDialog::refreshTableData() {
    // 暂时断开cellChanged信号，避免触发验证
    tableWidget->blockSignals(true);
    
    try {
        const Table& table = dbManager.getTable(tableName.toStdString());
        const auto& data = table.getData();
        
        tableWidget->setRowCount(data.size());
        
        // 填充数据
        for (int i = 0; i < data.size(); i++) {
            for (int j = 0; j < data[i].size(); j++) {
                QTableWidgetItem* item = new QTableWidgetItem(
                    QString::fromStdString(data[i][j]));
                item->setFlags(item->flags() | Qt::ItemIsEditable);  // 确保可编辑
                tableWidget->setItem(i, j, item);
            }
        }
        
        // 调整列宽
        tableWidget->resizeColumnsToContents();
        tableWidget->horizontalHeader()->setStretchLastSection(true);
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "错误", QString::fromStdString(e.what()));
    }
    
    tableWidget->blockSignals(false);
}

void TableViewDialog::saveChanges() {
    try {
        // 首先验证所有数据
        for (int row = 0; row < tableWidget->rowCount(); ++row) {
            for (int col = 0; col < tableWidget->columnCount(); ++col) {
                QTableWidgetItem* item = tableWidget->item(row, col);
                QString value = item ? item->text() : "";
                if (!validateCell(row, col, value)) {
                    tableWidget->setCurrentCell(row, col);
                    throw std::runtime_error(
                        QString("第 %1 行 %2 列的数据无效")
                            .arg(row + 1)
                            .arg(QString::fromStdString(columns[col].name))
                            .toStdString()
                    );
                }
            }
        }

        // 收集所有数据
        std::vector<std::vector<std::string>> newData;
        for (int row = 0; row < tableWidget->rowCount(); ++row) {
            std::vector<std::string> rowData;
            for (int col = 0; col < tableWidget->columnCount(); ++col) {
                QTableWidgetItem* item = tableWidget->item(row, col);
                rowData.push_back(item ? item->text().trimmed().toStdString() : "");
            }
            newData.push_back(rowData);
        }
        
        // 获取表引用
        Table& table = const_cast<Table&>(dbManager.getTable(tableName.toStdString()));
        
        // 创建新表（使用相同的结构）
        Table newTable(tableName.toStdString(), columns);
        
        // 插入新数据
        bool allSuccess = true;
        QString errorMessage;
        for (size_t i = 0; i < newData.size(); ++i) {
            try {
                if (!newTable.insertRow(newData[i])) {
                    allSuccess = false;
                    errorMessage = QString("第 %1 行数据插入失败").arg(i + 1);
                    break;
                }
            } catch (const std::exception& e) {
                allSuccess = false;
                errorMessage = QString("第 %1 行: %2").arg(i + 1).arg(e.what());
                break;
            }
        }
        
        if (!allSuccess) {
            throw std::runtime_error(errorMessage.toStdString());
        }
        
        // 如果所有数据都插入成功，替换原表
        table = std::move(newTable);
        
        // 保存到文件
        SQLParser::ParsedQuery query;
        query.type = "UPDATE";
        query.tableName = tableName.toStdString();
        
        if (!dbManager.executeNonQuery(query)) {
            throw std::runtime_error("保存到文件失败");
        }
        
        QMessageBox::information(this, "成功", "数据保存成功");
        
        // 刷新显示
        refreshTableData();
        
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "错误", 
            QString("保存失败: %1").arg(QString::fromStdString(e.what())));
    }
}

bool TableViewDialog::validateCell(int row, int column, const QString& value) {
    if (column >= columns.size()) return false;
    
    const auto& colDef = columns[column];
    
    // 检查非空约束
    if (!colDef.nullable && value.trimmed().isEmpty()) {
        QMessageBox::warning(this, "验证错误", 
            QString("列 %1 不允许为空").arg(QString::fromStdString(colDef.name)));
        return false;
    }
    
    // 如果是空值且允许为空，则验证通过
    if (value.trimmed().isEmpty() && colDef.nullable) {
        return true;
    }
    
    // 检查数据类型
    if (colDef.type == "INTEGER") {
        bool ok;
        value.toInt(&ok);
        if (!ok) {
            QMessageBox::warning(this, "验证错误", 
                QString("列 %1 必须是整数").arg(QString::fromStdString(colDef.name)));
            return false;
        }
    } else if (colDef.type == "FLOAT") {
        bool ok;
        value.toFloat(&ok);
        if (!ok) {
            QMessageBox::warning(this, "验证错误", 
                QString("列 %1 必须是数字").arg(QString::fromStdString(colDef.name)));
            return false;
        }
    }
    
    return true;
}

void TableViewDialog::cellChanged(int row, int column) {
    QTableWidgetItem* item = tableWidget->item(row, column);
    if (!item) return;
    
    // 暂时阻止信号以避免递归
    tableWidget->blockSignals(true);
    
    // 验证并格式化数据
    QString value = item->text().trimmed();
    if (!validateCell(row, column, value)) {
        item->setText("");
    } else {
        // 如果是数值类型，格式化显示
        const auto& colDef = columns[column];
        if (colDef.type == "INTEGER" && !value.isEmpty()) {
            bool ok;
            int num = value.toInt(&ok);
            if (ok) {
                item->setText(QString::number(num));
            }
        } else if (colDef.type == "FLOAT" && !value.isEmpty()) {
            bool ok;
            float num = value.toFloat(&ok);
            if (ok) {
                item->setText(QString::number(num, 'g', 6));
            }
        }
    }
    
    tableWidget->blockSignals(false);
} 