<?php
session_start();
session_destroy();
$next = $_GET['next'] ?? 'login.php';
if (str_contains($next, '://') || str_starts_with($next, '//')) {
    $next = 'login.php';
}
header('Location: ' . $next);
exit;
