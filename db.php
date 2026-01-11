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
        qr_token TEXT UNIQUE NOT NULL
    )');

    return $db;
}
