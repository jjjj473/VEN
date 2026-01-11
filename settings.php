<?php
session_start();
require_once __DIR__ . '/db.php';

$db = get_db();
$user = null;

if (isset($_SESSION['user_id'])) {
    $stmt = $db->prepare('SELECT id, username, qr_token, created_at FROM users WHERE id = :id');
    $stmt->bindValue(':id', $_SESSION['user_id'], SQLITE3_INTEGER);
    $result = $stmt->execute();
    $user = $result->fetchArray(SQLITE3_ASSOC) ?: null;
}

if (!$user) {
    header('Location: index.php?error=Please+log+in+to+view+settings');
    exit;
}

$messages = [];
$errors = [];

$profileStmt = $db->prepare('SELECT display_name, bio, location, theme FROM profiles WHERE user_id = :user_id');
$profileStmt->bindValue(':user_id', $user['id'], SQLITE3_INTEGER);
$profileResult = $profileStmt->execute();
$profile = $profileResult->fetchArray(SQLITE3_ASSOC) ?: [
    'display_name' => $user['username'],
    'bio' => '',
    'location' => '',
    'theme' => 'midnight'
];

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $action = $_POST['action'] ?? '';

    if ($action === 'update_profile') {
        $displayName = trim($_POST['display_name'] ?? '');
        $bio = trim($_POST['bio'] ?? '');
        $location = trim($_POST['location'] ?? '');
        $theme = trim($_POST['theme'] ?? 'midnight');

        if ($displayName === '') {
            $errors[] = 'Display name is required.';
        } else {
            $updateProfile = $db->prepare('INSERT INTO profiles (user_id, display_name, bio, location, theme) VALUES (:user_id, :display_name, :bio, :location, :theme)
                ON CONFLICT(user_id) DO UPDATE SET display_name = excluded.display_name, bio = excluded.bio, location = excluded.location, theme = excluded.theme');
            $updateProfile->bindValue(':user_id', $user['id'], SQLITE3_INTEGER);
            $updateProfile->bindValue(':display_name', $displayName, SQLITE3_TEXT);
            $updateProfile->bindValue(':bio', $bio, SQLITE3_TEXT);
            $updateProfile->bindValue(':location', $location, SQLITE3_TEXT);
            $updateProfile->bindValue(':theme', $theme, SQLITE3_TEXT);
            $updateProfile->execute();
            $messages[] = 'Profile updated.';
            $profile['display_name'] = $displayName;
            $profile['bio'] = $bio;
            $profile['location'] = $location;
            $profile['theme'] = $theme;
        }
    }

    if ($action === 'regenerate_qr') {
        $newToken = bin2hex(random_bytes(16));
        $stmt = $db->prepare('UPDATE users SET qr_token = :token WHERE id = :id');
        $stmt->bindValue(':token', $newToken, SQLITE3_TEXT);
        $stmt->bindValue(':id', $user['id'], SQLITE3_INTEGER);
        $stmt->execute();
        $messages[] = 'QR login token regenerated.';
        $user['qr_token'] = $newToken;
    }

    if ($action === 'change_password') {
        $current = $_POST['current_password'] ?? '';
        $new = $_POST['new_password'] ?? '';

        $check = $db->prepare('SELECT password_hash FROM users WHERE id = :id');
        $check->bindValue(':id', $user['id'], SQLITE3_INTEGER);
        $result = $check->execute();
        $row = $result->fetchArray(SQLITE3_ASSOC);

        if (!$row || !password_verify($current, $row['password_hash'])) {
            $errors[] = 'Current password is incorrect.';
        } elseif (strlen($new) < 6) {
            $errors[] = 'New password must be at least 6 characters.';
        } else {
            $hash = password_hash($new, PASSWORD_DEFAULT);
            $update = $db->prepare('UPDATE users SET password_hash = :hash WHERE id = :id');
            $update->bindValue(':hash', $hash, SQLITE3_TEXT);
            $update->bindValue(':id', $user['id'], SQLITE3_INTEGER);
            $update->execute();
            $messages[] = 'Password updated successfully.';
        }
    }
}

$qrLoginUrl = sprintf('%s://%s%s/qr_login.php?token=%s',
    (!empty($_SERVER['HTTPS']) && $_SERVER['HTTPS'] !== 'off') ? 'https' : 'http',
    $_SERVER['HTTP_HOST'],
    rtrim(dirname($_SERVER['PHP_SELF']), '/'),
    urlencode($user['qr_token'])
);
?>
<!doctype html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ATF Account Settings</title>
    <style>
        :root {
            --bg: #0f0f0f;
            --card: #1c1c1c;
            --muted: #aaaaaa;
            --accent: #ff1a1a;
            --text: #ffffff;
            --outline: #2b2b2b;
        }

        * {
            box-sizing: border-box;
            font-family: "Inter", "Segoe UI", sans-serif;
        }

        body {
            margin: 0;
            background: var(--bg);
            color: var(--text);
        }

        header {
            display: flex;
            align-items: center;
            justify-content: space-between;
            padding: 18px 32px;
            border-bottom: 1px solid var(--outline);
        }

        .pill {
            padding: 8px 16px;
            border-radius: 999px;
            background: #232323;
            border: 1px solid var(--outline);
            color: var(--text);
            text-decoration: none;
            font-size: 14px;
        }

        main {
            max-width: 1000px;
            margin: 32px auto;
            padding: 0 24px 48px;
        }

        .grid {
            display: grid;
            gap: 16px;
            grid-template-columns: repeat(auto-fit, minmax(260px, 1fr));
        }

        .form-grid {
            display: grid;
            gap: 12px;
            grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
        }

        .card {
            background: var(--card);
            border-radius: 16px;
            padding: 18px;
            border: 1px solid var(--outline);
        }

        label {
            display: block;
            font-size: 13px;
            color: var(--muted);
            margin-bottom: 8px;
        }

        input {
            width: 100%;
            padding: 10px 12px;
            border-radius: 10px;
            border: 1px solid var(--outline);
            background: #141414;
            color: var(--text);
        }

        textarea, select {
            width: 100%;
            padding: 10px 12px;
            border-radius: 10px;
            border: 1px solid var(--outline);
            background: #141414;
            color: var(--text);
        }

        textarea {
            min-height: 120px;
            resize: vertical;
        }

        button {
            margin-top: 12px;
            padding: 10px 16px;
            border-radius: 10px;
            border: none;
            background: var(--accent);
            color: #fff;
            font-weight: 600;
            cursor: pointer;
        }

        .muted {
            color: var(--muted);
            font-size: 13px;
        }

        .success {
            color: #6ee7b7;
        }

        .error {
            color: #fca5a5;
        }
    </style>
</head>
<body>
<header>
    <strong>ATF Account Settings</strong>
    <div>
        <a class="pill" href="index.php">Home</a>
        <a class="pill" href="upload.php">Upload</a>
        <a class="pill" href="library.php">Library</a>
        <a class="pill" href="dashboard.php">Dashboard</a>
    </div>
</header>
<main>
    <div class="card">
        <h2>Profile</h2>
        <p><strong><?php echo htmlspecialchars($user['username']); ?></strong></p>
        <p class="muted">Member since <?php echo htmlspecialchars(date('M j, Y', strtotime($user['created_at']))); ?></p>
        <?php foreach ($messages as $message): ?>
            <p class="success"><?php echo htmlspecialchars($message); ?></p>
        <?php endforeach; ?>
        <?php foreach ($errors as $error): ?>
            <p class="error"><?php echo htmlspecialchars($error); ?></p>
        <?php endforeach; ?>
    </div>

    <div class="grid" style="margin-top: 16px;">
        <div class="card">
            <h3>Creator profile</h3>
            <form method="post">
                <input type="hidden" name="action" value="update_profile">
                <div class="form-grid">
                    <div>
                        <label>Display name</label>
                        <input type="text" name="display_name" value="<?php echo htmlspecialchars($profile['display_name']); ?>" required>
                    </div>
                    <div>
                        <label>Location</label>
                        <input type="text" name="location" value="<?php echo htmlspecialchars($profile['location']); ?>">
                    </div>
                </div>
                <label>Bio</label>
                <textarea name="bio"><?php echo htmlspecialchars($profile['bio']); ?></textarea>
                <label>Theme</label>
                <select name="theme">
                    <?php foreach (['midnight', 'neon', 'aurora', 'classic'] as $theme): ?>
                        <option value="<?php echo $theme; ?>" <?php echo $profile['theme'] === $theme ? 'selected' : ''; ?>><?php echo ucfirst($theme); ?></option>
                    <?php endforeach; ?>
                </select>
                <button type="submit">Save profile</button>
            </form>
        </div>
        <div class="card">
            <h3>Login link</h3>
            <p class="muted">Use this secure link to sign in on another device.</p>
            <p class="muted"><?php echo htmlspecialchars($qrLoginUrl); ?></p>
            <form method="post">
                <input type="hidden" name="action" value="regenerate_qr">
                <button type="submit">Regenerate login token</button>
            </form>
        </div>
        <div class="card">
            <h3>Change password</h3>
            <form method="post">
                <input type="hidden" name="action" value="change_password">
                <label>Current password</label>
                <input type="password" name="current_password" required>
                <label>New password</label>
                <input type="password" name="new_password" required>
                <button type="submit">Update password</button>
            </form>
        </div>
    </div>
</main>
</body>
</html>
