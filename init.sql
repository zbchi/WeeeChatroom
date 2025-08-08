CREATE DATABASE IF NOT EXISTS chatdb CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE chatdb;

CREATE TABLE IF NOT EXISTS users
(
    id INT AUTO_INCREMENT PRIMARY KEY,
    email VARCHAR(64) NOT NULL,
    nickname VARCHAR(64) NOT NULL,
    password VARCHAR(64) NOT NULL,
    state ENUM('alive','die')DEFAULT 'alive',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
ALTER TABLE users AUTO_INCREMENT = 2;
INSERT INTO users (id, email, nickname, password)
VALUES (1, '1662308219', '系统消息', '1662308219@Zb');

CREATE TABLE IF NOT EXISTS messages
(
    id INT AUTO_INCREMENT PRIMARY KEY,
    sender_id INT NOT NULL,
    receiver_id INT NOT NULL,
    content TEXT NOT NULL,
    json TEXT NOT NULL,
    sent_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS offlineMessages
(
    id INT AUTO_INCREMENT PRIMARY KEY,
    sender_id INT NOT NULL,
    receiver_id INT NOT NULL,
    json TEXT NOT NULL,
    sent_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN kEY (sender_id) REFERENCES users(id),
    FOREIGN KEY (receiver_id) REFERENCES users(id)
);


CREATE TABLE IF NOT EXISTS friends
(
    user_id INT NOT NULL,
    friend_id INT NOT NULL,
    #json TEXT NOT NULL,
    status ENUM('pending','accepted','rejected')DEFAULT 'accepted',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,

    FOREIGN KEY (user_id) REFERENCES users(id),
    FOREIGN KEY (friend_id) REFERENCES users(id),
    UNIQUE KEY unique_friendship(user_id,friend_id)
);

CREATE TABLE IF NOT EXISTS `groups`
(
    id INT AUTO_INCREMENT PRIMARY KEY,
    name TEXT,
    description TEXT,
    creator_id INT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN kEY (creator_id) REFERENCES users(id)
);

CREATE TABLE IF NOT EXISTS friend_requests
(
    id INT AUTO_INCREMENT PRIMARY KEY, 
    from_user_id INT NOT NULL,
    to_user_id INT NOT NULL,
    json TEXT NOT NULL,
    status ENUM('pending','accepted','rejected')DEFAULT 'pending',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,

    FOREIGN KEY (from_user_id) REFERENCES users(id),
    FOREIGN KEY (to_user_id) REFERENCES users(id)
);

CREATE TABLE IF NOT EXISTS group_requests
(
    id INT AUTO_INCREMENT PRIMARY KEY, 
    from_user_id INT NOT NULL,
    to_user_id INT,
    group_id INT NOT NULL,
    json TEXT NOT NULL,
    status ENUM('pending','accepted','rejected')DEFAULT 'pending',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,

    FOREIGN KEY (to_user_id) REFERENCES users(id),
    FOREIGN KEY (from_user_id) REFERENCES users(id),
    FOREIGN KEY (group_id) REFERENCES `groups`(id)
);

CREATE TABLE IF NOT EXISTS remove_jsons
(
    id INT AUTO_INCREMENT PRIMARY KEY, 
    to_user_id INT,
    json TEXT NOT NULL,
    FOREIGN KEY (to_user_id) REFERENCES users(id)
);

CREATE TABLE IF NOT EXISTS group_members
(
    id INT AUTO_INCREMENT PRIMARY KEY,
    group_id INT NOT NULL,
    user_id INT NOT NULL,
    role ENUM('member','admin','owner') DEFAULT 'member',
    joined_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (group_id) REFERENCES `groups`(id),
    FOREIGN KEY (user_id) REFERENCES users(id)
);

CREATE TABLE IF NOT EXISTS group_messages
(
    id INT AUTO_INCREMENT PRIMARY KEY,
    group_id INT NOT NULL,
    sender_id INT NOT NULL,
    content TEXT NOT NULL,
    json TEXT NOT NULL,
    send_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (group_id) REFERENCES `groups`(id),
    FOREIGN KEY (sender_id) REFERENCES users(id)
);

CREATE TABLE IF NOT EXISTS files
(
    id INT AUTO_INCREMENT PRIMARY KEY,
    sender_id INT,
    receiver_id INT,
    group_id INT,
    file_size BIGINT,
    file_name TEXT,
    is_group BOOL DEFAULT FALSE,
    send_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE USER IF NOT EXISTS 'zb'@'localhost' IDENTIFIED BY '1662308219@Zb';
GRANT ALL PRIVILEGES ON chatdb.* To 'zb'@'localhost';
FLUSH PRIVILEGES;