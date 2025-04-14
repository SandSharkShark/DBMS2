#include "UserManager.h"
#include <fstream>
#include <sstream>
#include <openssl/sha.h>
#include <filesystem>

UserManager::UserManager(const std::string& path) 
    : dbPath(path), currentUser(nullptr) {
    loadUsers();
}

std::string UserManager::hashPassword(const std::string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, password.c_str(), password.length());
    SHA256_Final(hash, &sha256);
    
    std::stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

bool UserManager::registerUser(const std::string& username, 
                             const std::string& password,
                             UserRole role) {
    if (users.find(username) != users.end()) {
        return false;  // 用户已存在
    }
    
    users[username] = User(username, hashPassword(password), role);
    return saveUsers();
}

bool UserManager::login(const std::string& username, const std::string& password) {
    auto it = users.find(username);
    if (it == users.end()) return false;
    
    if (it->second.getPasswordHash() == hashPassword(password)) {
        currentUser = &it->second;
        return true;
    }
    return false;
}

void UserManager::logout() {
    currentUser = nullptr;
}

bool UserManager::loadUsers() {
    try {
        std::string filename = dbPath + "/users.dat";
        if (!std::filesystem::exists(filename)) {
            // 如果文件不存在，创建默认管理员账户
            registerUser("root", "123456", UserRole::ADMIN);
            return true;
        }
        
        std::ifstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        users.clear();
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string username, passwordHash, roleStr;
            
            if (std::getline(iss, username, ',') &&
                std::getline(iss, passwordHash, ',') &&
                std::getline(iss, roleStr)) {
                
                UserRole role = static_cast<UserRole>(std::stoi(roleStr));
                users[username] = User(username, passwordHash, role);
            }
        }
        
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool UserManager::saveUsers() {
    try {
        // 确保目录存在
        std::filesystem::create_directories(dbPath);
        
        std::string filename = dbPath + "/users.dat";
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        for (const auto& [username, user] : users) {
            file << user.getUsername() << ","
                 << user.getPasswordHash() << ","
                 << static_cast<int>(user.getRole()) << "\n";
        }
        
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

std::vector<std::string> UserManager::getUserList() const {
    std::vector<std::string> userList;
    for (const auto& [username, user] : users) {
        userList.push_back(username);
    }
    return userList;
}

bool UserManager::changePassword(const std::string& username, 
                               const std::string& oldPassword,
                               const std::string& newPassword) {
    auto it = users.find(username);
    if (it == users.end()) {
        return false;
    }
    
    if (it->second.getPasswordHash() != hashPassword(oldPassword)) {
        return false;
    }
    
    it->second = User(username, hashPassword(newPassword), it->second.getRole());
    return saveUsers();
}

bool UserManager::changeUserRole(const std::string& username, UserRole newRole) {
    auto it = users.find(username);
    if (it == users.end()) {
        return false;
    }
    
    it->second = User(username, it->second.getPasswordHash(), newRole);
    return saveUsers();
}

bool UserManager::deleteUser(const std::string& username) {
    if (users.erase(username) > 0) {
        return saveUsers();
    }
    return false;
}

const User* UserManager::getUser(const std::string& username) const {
    auto it = users.find(username);
    if (it != users.end()) {
        return &it->second;
    }
    return nullptr;
}

bool UserManager::resetPassword(const std::string& username, const std::string& newPassword) {
    auto it = users.find(username);
    if (it == users.end()) {
        return false;
    }
    
    it->second = User(username, hashPassword(newPassword), it->second.getRole());
    return saveUsers();
}

// ... 实现其他方法 ... 