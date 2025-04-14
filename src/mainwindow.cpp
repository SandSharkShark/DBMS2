#include "mainwindow.h"
#include "TableViewDialog.h"
#include "LoginDialog.h"
#include "UserManagerDialog.h"
#include "SettingsDialog.h"
#include "BatchProcessDialog.h"
#include <QtWidgets>
#include <QtCore/QTextStream>
#include <QtCore/QFile>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QFileDialog>
#include <QSettings>
#include <QInputDialog>
#include <QShortcut>
#include <QKeyEvent>
#include <QToolBar>
#include <QMenu>
#include <QApplication>
#include <QStyle>
#include <QIcon>
#include <QTimer>
#include <QTextStream>
#include <QFile>
#include <QTextBrowser>
#include <QtCore/QSettings>
#include <QtCore/QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , dbManager("/Volumes/HIKSEMI/课程文件/Code/DBMS2/data")
    , userManager("/Volumes/HIKSEMI/课程文件/Code/DBMS2/data/users") {
    setupUi();
    createMenus();
    createToolBars();
    createStatusBar();
    
    // 初始化数据库
    initializeDatabase();
    
    // 加载设置
    loadSettings();
    
    // 初始化UI权限状态
    updateUIForUser();
    
    // 在构造函数完成后，使用 QTimer 来显示登录对话框
    QTimer::singleShot(0, this, [this]() {
        showInitialLoginDialog();
    });
}

MainWindow::~MainWindow() {
    saveSettings();
    delete highlighter;
}

void MainWindow::setupUi() {
    // 创建中心部件
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // 创建主布局
    mainLayout = new QVBoxLayout(centralWidget);
    
    // 设置顶部面板（数据库路径选择）
    setupTopPanel();
    
    // 创建SQL模板工具栏
    createSqlTemplateToolBar();
    
    // 创建主分割布局
    QHBoxLayout* splitLayout = new QHBoxLayout();
    
    // 创建左侧面板
    QWidget* leftPanel = new QWidget;
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    
    // 数据库选择器
    dbSelector = new QComboBox;
    leftLayout->addWidget(new QLabel("数据库:"));
    leftLayout->addWidget(dbSelector);
    
    // 表列表
    tableList = new QListWidget;
    leftLayout->addWidget(new QLabel("表:"));
    leftLayout->addWidget(tableList);
    
    // SQL历史记录
    historyList = new QListWidget;
    leftLayout->addWidget(new QLabel("历史记录:"));
    leftLayout->addWidget(historyList);
    
    // 创建右侧面板
    QWidget* rightPanel = new QWidget;
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    
    // SQL输入区
    sqlInput = new QTextEdit;
    sqlInput->setPlaceholderText("在此输入SQL语句...");
    rightLayout->addWidget(sqlInput);
    
    // 按钮区
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    executeBtn = new QPushButton("执行");
    buttonLayout->addWidget(executeBtn);
    buttonLayout->addStretch();
    rightLayout->addLayout(buttonLayout);
    
    // 结果显示区
    resultTable = new QTableWidget;
    rightLayout->addWidget(resultTable);
    
    // 设置分割器
    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(leftPanel);
    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(1, 2);  // 右侧面板占更多空间
    
    splitLayout->addWidget(splitter);
    
    mainLayout->addLayout(splitLayout);
    
    // 连接信号槽
    connect(executeBtn, &QPushButton::clicked, this, &MainWindow::executeSQL);
    connect(dbSelector, &QComboBox::currentTextChanged, this, &MainWindow::switchDatabase);
    connect(tableList, &QListWidget::itemDoubleClicked, this, &MainWindow::showTableContent);
    connect(historyList, &QListWidget::itemDoubleClicked, this, &MainWindow::loadHistoryItem);
    
    // 设置语法高亮
    highlighter = new SQLHighlighter(sqlInput->document());
    
    // 设置窗口属性
    resize(1200, 800);
    setWindowTitle("数据库管理系统");
    
    // 禁用所有需要权限的功能，直到用户登录
    executeBtn->setEnabled(false);
    sqlInput->setReadOnly(true);
}

void MainWindow::setupTopPanel() {
    QHBoxLayout* topLayout = new QHBoxLayout();
    
    // 数据库路径选择
    pathSelectBtn = new QPushButton("选择数据库目录");
    pathDisplay = new QLineEdit();
    pathDisplay->setReadOnly(true);
    
    topLayout->addWidget(new QLabel("数据库路径:"));
    topLayout->addWidget(pathDisplay);
    topLayout->addWidget(pathSelectBtn);
    
    // 连接信号槽
    connect(pathSelectBtn, &QPushButton::clicked, this, &MainWindow::selectDatabasePath);
    
    // 更新路径显示
    updatePathDisplay();
    
    mainLayout->addLayout(topLayout);
}

void MainWindow::createSqlTemplateToolBar() {
    sqlToolBar = new QToolBar("SQL模板");
    addToolBar(Qt::TopToolBarArea, sqlToolBar);
    
    // 添加SQL模板按钮
    sqlToolBar->addAction("SELECT模板", this, &MainWindow::insertSelectTemplate);
    sqlToolBar->addAction("JOIN查询模板", this, &MainWindow::insertJoinTemplate);
    sqlToolBar->addAction("CREATE DATABASE模板", this, &MainWindow::insertCreateTemplate);
    sqlToolBar->addAction("CREATE TABLE模板", this, &MainWindow::insertCreateTableTemplate);
    sqlToolBar->addAction("DROP TABLE模板", this, &MainWindow::insertDropTableTemplate);
    sqlToolBar->addAction("INSERT模板", this, &MainWindow::insertInsertTemplate);
    sqlToolBar->addAction("UPDATE模板", this, &MainWindow::insertUpdateTemplate);
    sqlToolBar->addAction("DELETE模板", this, &MainWindow::insertDeleteTemplate);
}

void MainWindow::selectDatabasePath() {
    QString dir = QFileDialog::getExistingDirectory(this, 
        "选择数据库目录",
        getCurrentPath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    
    if (!dir.isEmpty()) {
        dbManager.setDbPath(dir.toStdString());
        updatePathDisplay();
        refreshDatabaseList();
    }
}

void MainWindow::updatePathDisplay() {
    pathDisplay->setText(getCurrentPath());
}

QString MainWindow::getCurrentPath() const {
    return QString::fromStdString(dbManager.getDbPath());
}

// SQL模板插入方法
void MainWindow::insertSelectTemplate() {
    sqlInput->setText(
        "SELECT * FROM Students;"
    );
}

void MainWindow::insertCreateTemplate() {
    sqlInput->setText("CREATE DATABASE database_name;");
}   

void MainWindow::insertInsertTemplate() {
    sqlInput->setText("INSERT INTO Students (ID, Name, Age, Gender, Department, EnrollmentDate)\n"
                    "VALUES (4, 赵六, 19, 男, 自动化系, 2023-09-01);");
}

void MainWindow::insertUpdateTemplate() {
    sqlInput->setText("UPDATE Students SET Age = 21 \n"
                     "WHERE ID = 1");
}

void MainWindow::insertDeleteTemplate() {
    sqlInput->setText("DELETE FROM Students \n"
                     "WHERE ID = 2");
}

void MainWindow::insertCreateTableTemplate() {
    sqlInput->setText(
        "CREATE TABLE Courses (\n"
        "    ID INTEGER PRIMARY KEY,\n"
        "    CourseName TEXT NOT NULL,\n"
        "    StudentID INTEGER,\n"
        "    FOREIGN KEY (StudentID) REFERENCES Students(ID)\n"
        ");"
    );
}

void MainWindow::insertJoinTemplate() {
    sqlInput->setText(
        "SELECT s.Name, c.CourseName, c.Grade\n"
        "FROM Students s\n"
        "JOIN Courses c ON s.ID = c.StudentID\n"
        "WHERE c.Grade >= 60;"
    );
}

void MainWindow::insertDropTableTemplate() {
    sqlInput->setText("DROP TABLE table_name;");
}

void MainWindow::initializeDatabase() {
    try {
        // 刷新数据库列表
        refreshDatabaseList();
        
        // 如果有数据库，自动选择第一个
        auto databases = dbManager.getDatabaseList();
        if (!databases.empty()) {
            dbManager.useDatabase(databases[0]);
            refreshTableList();
            statusLabel->setText("当前数据库: " + QString::fromStdString(databases[0]));
        }
    } catch (const std::exception& e) {
        showError(QString::fromStdString(e.what()));
    }
}

void MainWindow::createMenus() {
    menuBar()->addMenu(createDatabaseMenu());
    menuBar()->addMenu(createTableMenu());
    menuBar()->addMenu(createQueryMenu());
    menuBar()->addMenu(createToolsMenu());
    menuBar()->addMenu(createUserMenu());
    menuBar()->addMenu(createHelpMenu());
}

QMenu* MainWindow::createDatabaseMenu() {
    QMenu* menu = new QMenu("数据库(&D)", this);
    
    // 创建子菜单：新建
    QMenu* newMenu = menu->addMenu(QIcon::fromTheme("document-new"), "新建(&N)");
    newMenu->addAction("数据库(&D)", this, [this]() {
        QString name = QInputDialog::getText(this, "新建数据库", "数据库名称:");
        if (!name.isEmpty()) {
            try {
                if (dbManager.createDatabase(name.toStdString())) {
                    refreshDatabaseList();
                    statusLabel->setText("数据库创建成功");
                }
            } catch (const std::exception& e) {
                showError(QString::fromStdString(e.what()));
            }
        }
    });
    
    // 打开数据库
    menu->addAction("打开数据库(&O)", this, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, "选择数据库目录");
        if (!dir.isEmpty()) {
            dbManager.setDbPath(dir.toStdString());
            refreshDatabaseList();
        }
    });
    
    menu->addSeparator();
    menu->addAction("退出(&X)", this, &QWidget::close);
    
    return menu;
}

QMenu* MainWindow::createTableMenu() {
    QMenu* menu = new QMenu("表(&T)", this);
    
    // 新建表
    menu->addAction("新建表(&N)", this, &MainWindow::createNewTable);
    
    // 表操作子菜单
    QMenu* operationsMenu = menu->addMenu("操作(&O)");
    operationsMenu->addAction("查看结构(&S)", this, &MainWindow::showTableStructure);
    operationsMenu->addAction("查看数据(&V)", this, &MainWindow::showTableContent);
    operationsMenu->addAction("编辑数据(&E)", this, &MainWindow::editTableData);
    operationsMenu->addAction("删除表(&D)", this, &MainWindow::deleteTable);
    
    // 导入导出子菜单
    QMenu* importExportMenu = menu->addMenu("导入/导出(&I)");
    importExportMenu->addAction("导出数据(&E)", this, &MainWindow::exportTableData);
    importExportMenu->addAction("导入数据(&I)", this, &MainWindow::importTableData);
    
    return menu;
}

QMenu* MainWindow::createQueryMenu() {
    QMenu* menu = new QMenu("查询(&Q)", this);
    
    menu->addAction("SELECT 查询", this, &MainWindow::insertSelectTemplate);
    menu->addAction("JOIN 查询", this, &MainWindow::insertJoinTemplate);
    menu->addSeparator();
    menu->addAction("INSERT 语句", this, &MainWindow::insertInsertTemplate);
    menu->addAction("UPDATE 语句", this, &MainWindow::insertUpdateTemplate);
    menu->addAction("DELETE 语句", this, &MainWindow::insertDeleteTemplate);
    
    return menu;
}

void MainWindow::createToolBars() {
    // 数据库工具栏
    QToolBar* dbToolBar = addToolBar("数据库操作");
    dbToolBar->setIconSize(QSize(24, 24));
    addToolBarBreak();  // 添加工具栏换行
    
    // 数据库操作按钮组
    QWidget* dbGroup = new QWidget;
    QHBoxLayout* dbLayout = new QHBoxLayout(dbGroup);
    dbLayout->setContentsMargins(5, 0, 5, 0);
    dbLayout->setSpacing(2);
    
    // 添加数据库操作按钮
    auto newDbBtn = new QToolButton;
    newDbBtn->setIcon(QIcon::fromTheme("document-new"));
    newDbBtn->setText("新建数据库");
    newDbBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    
    auto openDbBtn = new QToolButton;
    openDbBtn->setIcon(QIcon::fromTheme("document-open"));
    openDbBtn->setText("打开数据库");
    openDbBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    
    dbLayout->addWidget(newDbBtn);
    dbLayout->addWidget(openDbBtn);
    dbToolBar->addWidget(dbGroup);
    
    // 连接数据库操作信号
    connect(newDbBtn, &QToolButton::clicked, this, [this]() {
        QString name = QInputDialog::getText(this, "新建数据库", "数据库名称:");
        if (!name.isEmpty()) {
            try {
                if (dbManager.createDatabase(name.toStdString())) {
                    refreshDatabaseList();
                    statusLabel->setText("数据库创建成功");
                }
            } catch (const std::exception& e) {
                showError(QString::fromStdString(e.what()));
            }
        }
    });
    
    connect(openDbBtn, &QToolButton::clicked, this, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, "选择数据库目录");
        if (!dir.isEmpty()) {
            dbManager.setDbPath(dir.toStdString());
            refreshDatabaseList();
        }
    });
    
    // 表操作工具栏
    QToolBar* tableToolBar = addToolBar("表操作");
    tableToolBar->setIconSize(QSize(24, 24));
    
    // 表操作按钮组
    QWidget* tableGroup = new QWidget;
    QHBoxLayout* tableLayout = new QHBoxLayout(tableGroup);
    tableLayout->setContentsMargins(5, 0, 5, 0);
    tableLayout->setSpacing(2);
    
    // 添加表操作按钮
    auto newTableBtn = new QToolButton;
    newTableBtn->setIcon(QIcon::fromTheme("list-add"));
    newTableBtn->setText("新建表");
    newTableBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    
    auto editTableBtn = new QToolButton;
    editTableBtn->setIcon(QIcon::fromTheme("document-edit"));
    editTableBtn->setText("编辑数据");
    editTableBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    
    auto deleteTableBtn = new QToolButton;
    deleteTableBtn->setIcon(QIcon::fromTheme("list-remove"));
    deleteTableBtn->setText("删除表");
    deleteTableBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    
    tableLayout->addWidget(newTableBtn);
    tableLayout->addWidget(editTableBtn);
    tableLayout->addWidget(deleteTableBtn);
    tableToolBar->addWidget(tableGroup);
    
    // 连接表操作信号
    connect(newTableBtn, &QToolButton::clicked, this, &MainWindow::createNewTable);
    connect(editTableBtn, &QToolButton::clicked, this, &MainWindow::editTableData);
    connect(deleteTableBtn, &QToolButton::clicked, this, &MainWindow::deleteTable);
    
    // 查询工具栏
    QToolBar* queryToolBar = addToolBar("查询操作");
    queryToolBar->setIconSize(QSize(24, 24));
    
    // 查询按钮组
    QWidget* queryGroup = new QWidget;
    QHBoxLayout* queryLayout = new QHBoxLayout(queryGroup);
    queryLayout->setContentsMargins(5, 0, 5, 0);
    queryLayout->setSpacing(2);
    
    // SQL模板按钮
    auto selectBtn = new QToolButton;
    selectBtn->setIcon(QIcon::fromTheme("system-search"));
    selectBtn->setText("SELECT");
    selectBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    
    auto joinBtn = new QToolButton;
    joinBtn->setIcon(QIcon::fromTheme("view-refresh"));
    joinBtn->setText("JOIN");
    joinBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    
    auto insertBtn = new QToolButton;
    insertBtn->setIcon(QIcon::fromTheme("document-new"));
    insertBtn->setText("INSERT");
    insertBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    
    auto updateBtn = new QToolButton;
    updateBtn->setIcon(QIcon::fromTheme("document-edit"));
    updateBtn->setText("UPDATE");
    updateBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    
    auto deleteBtn = new QToolButton;
    deleteBtn->setIcon(QIcon::fromTheme("edit-delete"));
    deleteBtn->setText("DELETE");
    deleteBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    
    queryLayout->addWidget(selectBtn);
    queryLayout->addWidget(joinBtn);
    queryLayout->addWidget(insertBtn);
    queryLayout->addWidget(updateBtn);
    queryLayout->addWidget(deleteBtn);
    queryToolBar->addWidget(queryGroup);
    
    // 连接查询操作信号
    connect(selectBtn, &QToolButton::clicked, this, &MainWindow::insertSelectTemplate);
    connect(joinBtn, &QToolButton::clicked, this, &MainWindow::insertJoinTemplate);
    connect(insertBtn, &QToolButton::clicked, this, &MainWindow::insertInsertTemplate);
    connect(updateBtn, &QToolButton::clicked, this, &MainWindow::insertUpdateTemplate);
    connect(deleteBtn, &QToolButton::clicked, this, &MainWindow::insertDeleteTemplate);
}

void MainWindow::createStatusBar() {
    statusLabel = new QLabel("就绪");
    statusBar()->addWidget(statusLabel);
}

void MainWindow::executeSQL() {
    // 添加权限检查
    if (!userManager.isLoggedIn()) {
        showError("请先登录");
        return;
    }
    
    // 对于修改操作，检查用户权限
    QString sql = sqlInput->toPlainText().trimmed().toUpper();
    bool isModifyOperation = sql.startsWith("INSERT") || 
                           sql.startsWith("UPDATE") || 
                           sql.startsWith("DELETE") ||
                           sql.startsWith("CREATE") ||
                           sql.startsWith("DROP");
    
    if (isModifyOperation && !userManager.getCurrentUser()->canModifyData()) {
        showError("当前用户没有修改数据的权限");
        return;
    }
    
    try {
        QString sql = sqlInput->toPlainText().trimmed();
        if (sql.isEmpty()) {
            showError("SQL语句不能为空");
            return;
        }
        
        // 解析SQL
        auto query = sqlParser.parse(sql.toStdString());
        
        // 执行SQL
        if (query.type == "SELECT") {
            // 获取表的所有列
            const Table& table = dbManager.getTable(query.tableName);
            std::vector<std::string> headers;
            
            // 如果是 SELECT *，使用所有列名
            if (query.columns.size() == 1 && query.columns[0].name == "*") {
                for (const auto& col : table.getColumns()) {
                    headers.push_back(col.name);
                }
            } else {
                // 使用查询指定的列名
                for (const auto& col : query.columns) {
                    headers.push_back(col.name);
                }
            }
            
            // 执行查询
            auto results = dbManager.executeSelect(query);
            
            // 显示结果
            displayResults(results, headers);
            
            // 更新状态栏
            statusLabel->setText(QString("查询返回 %1 行").arg(results.size()));
        } else {
            if (dbManager.executeNonQuery(query)) {
                statusLabel->setText("执行成功");
                refreshTableList();
            } else {
                showError("执行失败");
            }
        }
        
        // 添加到历史记录
        sqlHistory.append(sql);
        historyList->addItem(sql);
        
    } catch (const std::exception& e) {
        showError(QString::fromStdString(e.what()));
    }
}

void MainWindow::displayResults(const std::vector<std::vector<std::string>>& results,
                              const std::vector<std::string>& headers) {
    resultTable->clear();
    resultTable->setRowCount(results.size());
    resultTable->setColumnCount(headers.size());
    
    // 设置表头
    QStringList headerLabels;
    for (const auto& header : headers) {
        headerLabels << QString::fromStdString(header);
    }
    resultTable->setHorizontalHeaderLabels(headerLabels);
    
    // 填充数据
    for (int i = 0; i < results.size(); i++) {
        for (int j = 0; j < results[i].size() && j < headers.size(); j++) {
            QTableWidgetItem* item = new QTableWidgetItem(
                QString::fromStdString(results[i][j]));
            resultTable->setItem(i, j, item);
        }
    }
    
    // 自动调整列宽
    resultTable->resizeColumnsToContents();
}

void MainWindow::showError(const QString& message) {
    QMessageBox::critical(this, "错误", message);
    statusLabel->setText("错误: " + message);
}

void MainWindow::switchDatabase(const QString& dbName) {
    try {
        if (dbManager.useDatabase(dbName.toStdString())) {
            refreshTableList();
            statusLabel->setText("当前数据库: " + dbName);
        }
    } catch (const std::exception& e) {
        showError(QString::fromStdString(e.what()));
    }
}

void MainWindow::refreshTableList() {
    tableList->clear();
    auto tables = dbManager.getTableList();
    for (const auto& table : tables) {
        tableList->addItem(QString::fromStdString(table));
    }
}

void MainWindow::refreshDatabaseList() {
    try {
        auto databases = dbManager.getDatabaseList();
        dbSelector->clear();
        for (const auto& db : databases) {
            dbSelector->addItem(QString::fromStdString(db));
        }
    } catch (const std::exception& e) {
        showError(QString::fromStdString(e.what()));
    }
}

void MainWindow::showTableStructure() {
    auto item = tableList->currentItem();
    if (!item) {
        showError("请先选择一个表");
        return;
    }
    
    try {
        QString tableName = item->text();
        const Table& table = dbManager.getTable(tableName.toStdString());
        
        QString structure = "表结构:\n\n";
        for (const auto& col : table.getColumns()) {
            structure += QString("列名: %1\n类型: %2\n可空: %3\n主键: %4\n\n")
                .arg(QString::fromStdString(col.name))
                .arg(QString::fromStdString(col.type))
                .arg(col.nullable ? "是" : "否")
                .arg(col.primaryKey ? "是" : "否");
        }
        
        QMessageBox::information(this, "表结构 - " + tableName, structure);
    } catch (const std::exception& e) {
        showError(QString::fromStdString(e.what()));
    }
}

void MainWindow::showTableContent() {
    auto item = tableList->currentItem();
    if (!item) return;
    
    try {
        QString tableName = item->text();
        
        // 获取表的所有列名
        const Table& table = dbManager.getTable(tableName.toStdString());
        std::vector<std::string> columnNames;
        for (const auto& col : table.getColumns()) {
            columnNames.push_back(col.name);
        }
        
        // 执行查询
        auto results = dbManager.select(
            tableName.toStdString(),
            columnNames
        );
        
        // 显示结果
        displayResults(results, columnNames);
    } catch (const std::exception& e) {
        showError(QString::fromStdString(e.what()));
    }
}

void MainWindow::showAbout() {
    QMessageBox::about(this, "关于",
        "数据库管理系统 v1.4\n\n"
        "简单的数据库管理系统，支持基本的SQL操作。\n"
        "支持多用户管理，支持权限管理，支持数据导入导出，支持批处理操作。\n"
        "支持SQL语句的语法高亮，支持SQL语句的自动补全，支持SQL语句的执行历史记录。\n"
        "支持数据库的备份和恢复，支持数据库的压缩和解压。\n\n"
        "基于C++17和QT5开发\n\n"
        "作者: 计科第8组\n"
        "日期: 2025-01-14");
}

void MainWindow::clearHistory() {
    sqlHistory.clear();
    historyList->clear();
}

void MainWindow::loadHistoryItem(QListWidgetItem* item) {
    if (item) {
        sqlInput->setText(item->text());
    }
}

void MainWindow::saveSettings() {
    QSettings settings("YourCompany", "DatabaseSystem");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("sqlHistory", sqlHistory);
}

void MainWindow::loadSettings() {
    QSettings settings("YourCompany", "DatabaseSystem");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    sqlHistory = settings.value("sqlHistory").toStringList();
    
    // 加载历史记录
    historyList->addItems(sqlHistory);
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event) {
    if (obj == sqlInput && event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return && 
            keyEvent->modifiers() == Qt::ControlModifier) {
            executeSQL();
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

// 新增的槽函数实现
void MainWindow::deleteTable() {
    // 添加权限检查
    if (!userManager.isLoggedIn()) {
        showError("请先登录");
        return;
    }
    
    if (!userManager.getCurrentUser()->canModifyData()) {
        showError("当前用户没有删除表的权限");
        return;
    }
    
    auto item = tableList->currentItem();
    if (!item) {
        showError("请先选择要删除的表");
        return;
    }
    
    QString tableName = item->text();
    if (QMessageBox::question(this, "确认删除", 
        QString("确定要删除表 %1 吗？").arg(tableName),
        QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        try {
            if (dbManager.dropTable(tableName.toStdString())) {
                refreshTableList();
                statusLabel->setText("表删除成功");
            }
        } catch (const std::exception& e) {
            showError(QString::fromStdString(e.what()));
        }
    }
}

void MainWindow::editTableData() {
    // 添加权限检查
    if (!userManager.isLoggedIn()) {
        showError("请先登录");
        return;
    }
    
    if (!userManager.getCurrentUser()->canModifyData()) {
        showError("当前用户没有修改数据的权限");
        return;
    }
    
    auto item = tableList->currentItem();
    if (!item) {
        showError("请先选择要编辑的表");
        return;
    }
    
    try {
        QString tableName = item->text();
        const Table& table = dbManager.getTable(tableName.toStdString());
        
        // 创建并显示表数据编辑对话框
        TableViewDialog* dialog = new TableViewDialog(table, dbManager, this);
        dialog->setWindowModality(Qt::WindowModal);
        dialog->show();
    } catch (const std::exception& e) {
        showError(QString::fromStdString(e.what()));
    }
}

void MainWindow::exportTableData() {
    auto item = tableList->currentItem();
    if (!item) {
        showError("请先选择要导出的表");
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(this,
        "导出数据", "", "CSV文件 (*.csv);;所有文件 (*)");
    
    if (!fileName.isEmpty()) {
        try {
            // TODO: 实现导出功能
            statusLabel->setText("数据导出成功");
        } catch (const std::exception& e) {
            showError(QString::fromStdString(e.what()));
        }
    }
}

void MainWindow::importTableData() {
    auto item = tableList->currentItem();
    if (!item) {
        showError("请先选择要导入数据的表");
        return;
    }
    
    QString fileName = QFileDialog::getOpenFileName(this,
        "导入数据", "", "CSV文件 (*.csv);;所有文件 (*)");
    
    if (!fileName.isEmpty()) {
        try {
            // TODO: 实现导入功能
            statusLabel->setText("数据导入成功");
        } catch (const std::exception& e) {
            showError(QString::fromStdString(e.what()));
        }
    }
}

QMenu* MainWindow::createToolsMenu() {
    QMenu* menu = new QMenu("工具(&T)", this);
    
    menu->addAction("批处理(&B)", this, &MainWindow::showBatchProcessDialog);
    menu->addSeparator();
    menu->addAction("设置(&S)", this, &MainWindow::showSettingsDialog);
    menu->addAction("清除历史记录(&C)", this, &MainWindow::clearHistory);
    
    return menu;
}

QMenu* MainWindow::createHelpMenu() {
    QMenu* menu = new QMenu("帮助(&H)", this);
    
    menu->addAction("关于(&A)", this, &MainWindow::showAbout);
    menu->addAction("帮助文档(&H)", this, &MainWindow::showHelp);
    
    return menu;
}

void MainWindow::createNewTable() {
    if (dbManager.getCurrentDatabase().empty()) {
        showError("请先选择数据库");
        return;
    }
    
    // 插入创建表的模板到SQL输入框
    insertCreateTableTemplate();
    
    // 聚焦到SQL输入框
    sqlInput->setFocus();
    
    // 显示提示
    statusLabel->setText("请编辑CREATE TABLE语句并执行");
}

void MainWindow::updateUIForUser() {
    bool isLoggedIn = userManager.isLoggedIn();
    const User* user = userManager.getCurrentUser();
    
    // 更新菜单状态
    menuBar()->clear();
    menuBar()->addMenu(createDatabaseMenu());
    menuBar()->addMenu(createTableMenu());
    menuBar()->addMenu(createQueryMenu());
    menuBar()->addMenu(createToolsMenu());
    menuBar()->addMenu(createUserMenu());
    menuBar()->addMenu(createHelpMenu());
    
    // 根据用户权限更新UI
    if (isLoggedIn) {
        bool canModify = user->canModifyData();
        executeBtn->setEnabled(true);
        sqlInput->setReadOnly(!canModify);
        
        // 更新工具栏按钮状态
        for (QAction* action : sqlToolBar->actions()) {
            if (action->text().contains("INSERT") ||
                action->text().contains("UPDATE") ||
                action->text().contains("DELETE")) {
                action->setEnabled(canModify);
            }
        }
    } else {
        executeBtn->setEnabled(false);
        sqlInput->setReadOnly(true);
    }
}

void MainWindow::showLoginDialog() {
    LoginDialog dialog(userManager, this);
    if (dialog.exec() == QDialog::Accepted) {
        updateUIForUser();  // 登录成功后更新UI
        statusLabel->setText("已登录: " + 
            QString::fromStdString(userManager.getCurrentUser()->getUsername()));
    }
}

QMenu* MainWindow::createUserMenu() {
    QMenu* menu = new QMenu("用户(&U)", this);
    
    // 添加用户相关操作
    if (!userManager.isLoggedIn()) {
        menu->addAction("登录(&L)", this, &MainWindow::showLoginDialog);
    } else {
        const User* user = userManager.getCurrentUser();
        QAction* userLabel = menu->addAction(QString("当前用户: %1").arg(
            QString::fromStdString(user->getUsername())));
        userLabel->setEnabled(false);  // 使其不可点击
        
        menu->addSeparator();
        
        if (user->canManageUsers()) {
            menu->addAction("用户管理(&M)", this, &MainWindow::showUserManager);
            menu->addSeparator();
        }
        
        menu->addAction("注销(&O)", this, &MainWindow::logout);
    }
    
    return menu;
}

void MainWindow::logout() {
    userManager.logout();
    updateUIForUser();
    statusLabel->setText("已注销");
}

void MainWindow::showUserManager() {
    if (!userManager.isLoggedIn() || 
        !userManager.getCurrentUser()->canManageUsers()) {
        QMessageBox::warning(this, "错误", "没有用户管理权限");
        return;
    }
    
    UserManagerDialog dialog(userManager, this);
    dialog.exec();
}

void MainWindow::showInitialLoginDialog() {
    LoginDialog dialog(userManager, this);
    dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowCloseButtonHint);  // 禁用关闭按钮
    
    while (!userManager.isLoggedIn()) {
        if (dialog.exec() == QDialog::Accepted) {
            updateUIForUser();
            statusLabel->setText("已登录: " + 
                QString::fromStdString(userManager.getCurrentUser()->getUsername()));
        } else {
            // 如果用户点击取消，则退出程序
            QApplication::quit();
            return;
        }
    }
}

void MainWindow::showSettingsDialog() {
    SettingsDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        // 应用新设置
        applySettings();
    }
}

void MainWindow::applySettings() {
    QSettings settings("MyCompany", "DatabaseSystem");
    
    // 应用数据库路径
    QString dbPath = settings.value("dbPath").toString();
    if (!dbPath.isEmpty()) {
        dbManager.setDbPath(dbPath.toStdString());
        updatePathDisplay();
    }
    
    // 应用编辑器设置
    if (settings.value("syntaxHighlight", true).toBool()) {
        if (!highlighter) {
            highlighter = new SQLHighlighter(sqlInput->document());
        }
    } else {
        delete highlighter;
        highlighter = nullptr;
    }
    
    // 设置编辑器字体
    QFont editorFont = settings.value("editorFont").value<QFont>();
    editorFont.setPointSize(settings.value("fontSize", 12).toInt());
    sqlInput->setFont(editorFont);
    
    // 应用其他设置
    // TODO: 实现其他设置的应用
}

void MainWindow::showBatchProcessDialog() {
    if (!userManager.isLoggedIn()) {
        showError("请先登录");
        return;
    }
    
    BatchProcessDialog dialog(dbManager, this);
    dialog.exec();
}

void MainWindow::showHelp() {
    // 创建帮助窗口
    QDialog* helpDialog = new QDialog(this);
    helpDialog->setWindowTitle(tr("使用说明"));
    helpDialog->resize(800, 600);
    
    // 创建文本浏览器
    helpBrowser = new QTextBrowser(helpDialog);
    
    // 创建布局
    QVBoxLayout* layout = new QVBoxLayout(helpDialog);
    layout->addWidget(helpBrowser);
    
    // 加载帮助内容
    loadHelpContent();
    
    // 显示对话框
    helpDialog->exec();
}

void MainWindow::loadHelpContent() {
    // 读取 README.md 文件
    QFile file(":/README.md");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("错误"), tr("无法加载帮助文档"));
        return;
    }
    
    QTextStream in(&file);
    QString content = in.readAll();
    file.close();
    
    // 设置文本内容
    helpBrowser->setMarkdown(content);
}

