<?php
session_start();
require_once __DIR__ . '/db.php';

$token = $_GET['token'] ?? '';
if ($token === '') {
    header('Location: index.php?error=Missing+token');
    exit;
}

function safe_redirect_target(string $target): string
{
    if ($target === '' || str_contains($target, '://') || str_starts_with($target, '//')) {
        return 'index.php';
    }

    return $target;
}

$db = get_db();
$stmt = $db->prepare('SELECT id FROM users WHERE qr_token = :token');
$stmt->bindValue(':token', $token, SQLITE3_TEXT);
$result = $stmt->execute();
$user = $result->fetchArray(SQLITE3_ASSOC);

if (!$user) {
    header('Location: index.php?error=Invalid+QR+token');
    exit;
}

$_SESSION['user_id'] = (int) $user['id'];
$next = safe_redirect_target($_GET['next'] ?? 'index.php');
header('Location: ' . $next);
exit;
