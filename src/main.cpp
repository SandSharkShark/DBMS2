#include <QtWidgets/QApplication>
#include <filesystem>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // 确保数据目录存在
    std::filesystem::create_directories("./data");
    std::filesystem::create_directories("./data/users");
    
    MainWindow window;
    window.show();
    
    return app.exec();
} 