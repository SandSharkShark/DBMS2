#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QComboBox>
#include <QListWidget>
#include <QSplitter>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QShortcut>
#include <QLineEdit>
#include <QToolBar>
#include <QToolButton>
#include <QHBoxLayout>

#include "DatabaseManager.h"
#include "SQLParser.h"
#include "SQLHighlighter.h"
#include "UserManager.h"
#include <QTextBrowser>

class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
private slots:
    void executeSQL();
    void displayResults(const std::vector<std::vector<std::string>>& results,
                       const std::vector<std::string>& headers);
    void showError(const QString& message);
    void switchDatabase(const QString& dbName);
    void refreshTableList();
    void showTableStructure();
    void clearHistory();
    void loadHistoryItem(QListWidgetItem* item);
    void showAbout();
    void refreshDatabaseList();
    void showTableContent();
    void selectDatabasePath();
    void insertSelectTemplate();
    void insertCreateTemplate();
    void insertInsertTemplate();
    void insertUpdateTemplate();
    void insertDeleteTemplate();
    void insertCreateTableTemplate();
    void insertJoinTemplate();
    void insertDropTableTemplate();
    void deleteTable();
    void editTableData();
    void createNewTable();
    void exportTableData();
    void importTableData();
    void showLoginDialog();
    void showUserManager();
    void logout();
    void updateUIForUser();
    void showSettingsDialog();
    void applySettings();
    void showBatchProcessDialog();
    void showHelp();
    
private:
    // UI组件
    QTextEdit* sqlInput;          // SQL输入框
    QTableWidget* resultTable;    // 结果显示表格
    QPushButton* executeBtn;      // 执行按钮
    QComboBox* dbSelector;        // 数据库选择器
    QListWidget* tableList;       // 表列表
    QListWidget* historyList;     // SQL历史记录
    QLabel* statusLabel;          // 状态标签
    QPushButton* pathSelectBtn;    // 选择数据库路径按钮
    QLineEdit* pathDisplay;        // 显示当前数据库路径
    QToolBar* sqlToolBar;          // SQL模板工具栏
    QVBoxLayout* mainLayout;       // 主布局
    
    // 数据管理
    DatabaseManager dbManager;
    SQLParser::SQLParser sqlParser;
    QStringList sqlHistory;       // SQL历史记录
    SQLHighlighter* highlighter;  // 语法高亮器
    UserManager userManager;
    
    // 创建UI组件
    void setupUi();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void setupMainLayout();
    
    // 保存/加载设置
    void saveSettings();
    void loadSettings();
    
    // 辅助函数
    void initializeDatabase();
    bool eventFilter(QObject* obj, QEvent* event) override;
    
    // 新增辅助方法
    void createSqlTemplateToolBar();
    void setupTopPanel();
    QString getCurrentPath() const;
    void updatePathDisplay();
    
    // 新增菜单相关成员
    QMenu* createDatabaseMenu();
    QMenu* createTableMenu();
    QMenu* createQueryMenu();
    QMenu* createToolsMenu();
    QMenu* createHelpMenu();
    QMenu* createUserMenu();
    
    // 新增工具栏相关成员
    void createDatabaseToolBar();
    void createTableToolBar();
    void createQueryToolBar();
    
    void showInitialLoginDialog();
    void loadHelpContent();
    QTextBrowser* helpBrowser;
};

#endif 