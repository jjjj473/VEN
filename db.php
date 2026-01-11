<?php
$databasePath = __DIR__ . '/atf.sqlite';

function get_db(): SQLite3
{
    global $databasePath;

    $db = new SQLite3($databasePath);
    $db->exec('PRAGMA foreign_keys = ON');

    $db->exec('CREATE TABLE IF NOT EXISTS users (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        username TEXT UNIQUE NOT NULL,
        password_hash TEXT NOT NULL,
        qr_token TEXT UNIQUE NOT NULL,
        created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
    )');

    $db->exec('CREATE TABLE IF NOT EXISTS profiles (
        user_id INTEGER PRIMARY KEY,
        display_name TEXT NOT NULL DEFAULT "",
        bio TEXT NOT NULL DEFAULT "",
        location TEXT NOT NULL DEFAULT "",
        theme TEXT NOT NULL DEFAULT "midnight",
        FOREIGN KEY (user_id) REFERENCES users (id) ON DELETE CASCADE
    )');

    $db->exec('CREATE TABLE IF NOT EXISTS videos (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        user_id INTEGER NOT NULL,
        title TEXT NOT NULL,
        description TEXT NOT NULL,
        category TEXT NOT NULL,
        views INTEGER NOT NULL DEFAULT 0,
        created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
        FOREIGN KEY (user_id) REFERENCES users (id) ON DELETE CASCADE
    )');

    $db->exec('CREATE TABLE IF NOT EXISTS notifications (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        user_id INTEGER NOT NULL,
        title TEXT NOT NULL,
        body TEXT NOT NULL,
        created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
        FOREIGN KEY (user_id) REFERENCES users (id) ON DELETE CASCADE
    )');

    return $db;
}
