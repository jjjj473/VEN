<?php
session_start();
require_once __DIR__ . '/db.php';

if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    header('Location: login.php');
    exit;
}

$action = $_POST['action'] ?? '';
$username = trim($_POST['username'] ?? '');
$password = $_POST['password'] ?? '';

if ($username === '' || $password === '') {
    $target = $action === 'signup' ? 'signup.php' : 'login.php';
    header('Location: ' . $target . '?error=Please+fill+in+all+fields');
    exit;
}

$db = get_db();

if ($action === 'signup') {
    $hash = password_hash($password, PASSWORD_DEFAULT);
    $token = bin2hex(random_bytes(16));

    $stmt = $db->prepare('INSERT INTO users (username, password_hash, qr_token) VALUES (:username, :password_hash, :qr_token)');
    $stmt->bindValue(':username', $username, SQLITE3_TEXT);
    $stmt->bindValue(':password_hash', $hash, SQLITE3_TEXT);
    $stmt->bindValue(':qr_token', $token, SQLITE3_TEXT);

    $result = @$stmt->execute();
    if ($result === false) {
        header('Location: signup.php?error=Username+already+taken');
        exit;
    }

    $userId = $db->lastInsertRowID();
    $profileStmt = $db->prepare('INSERT OR IGNORE INTO profiles (user_id, display_name) VALUES (:user_id, :display_name)');
    $profileStmt->bindValue(':user_id', $userId, SQLITE3_INTEGER);
    $profileStmt->bindValue(':display_name', $username, SQLITE3_TEXT);
    $profileStmt->execute();

    header('Location: login.php?success=1');
    exit;
}

if ($action === 'login') {
    $stmt = $db->prepare('SELECT id, password_hash FROM users WHERE username = :username');
    $stmt->bindValue(':username', $username, SQLITE3_TEXT);
    $result = $stmt->execute();
    $user = $result->fetchArray(SQLITE3_ASSOC);

    if (!$user || !password_verify($password, $user['password_hash'])) {
        header('Location: login.php?error=Invalid+credentials');
        exit;
    }

    $_SESSION['user_id'] = (int) $user['id'];
    header('Location: index.php');
    exit;
}

header('Location: login.php?error=Unknown+action');
exit;
