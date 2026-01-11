<?php
session_start();
require_once __DIR__ . '/db.php';

$db = get_db();
$user = null;

if (isset($_SESSION['user_id'])) {
    $stmt = $db->prepare('SELECT id, username FROM users WHERE id = :id');
    $stmt->bindValue(':id', $_SESSION['user_id'], SQLITE3_INTEGER);
    $result = $stmt->execute();
    $user = $result->fetchArray(SQLITE3_ASSOC) ?: null;
}

if (!$user) {
    header('Location: index.php?error=Please+log+in+to+upload');
    exit;
}

$statsStmt = $db->prepare('SELECT COUNT(*) as total, COALESCE(SUM(views), 0) as views FROM videos WHERE user_id = :user_id');
$statsStmt->bindValue(':user_id', $user['id'], SQLITE3_INTEGER);
$statsResult = $statsStmt->execute();
$stats = $statsResult->fetchArray(SQLITE3_ASSOC) ?: ['total' => 0, 'views' => 0];

$errors = [];
$success = false;
$title = '';
$description = '';
$category = 'Tech';

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $title = trim($_POST['title'] ?? '');
    $description = trim($_POST['description'] ?? '');
    $category = trim($_POST['category'] ?? 'Tech');

    if ($title === '') {
        $errors[] = 'Title is required.';
    }

    if ($description === '') {
        $errors[] = 'Description is required.';
    }

    if ($category === '') {
        $errors[] = 'Category is required.';
    }

    if (count($errors) === 0) {
        $stmt = $db->prepare('INSERT INTO videos (user_id, title, description, category) VALUES (:user_id, :title, :description, :category)');
        $stmt->bindValue(':user_id', $user['id'], SQLITE3_INTEGER);
        $stmt->bindValue(':title', $title, SQLITE3_TEXT);
        $stmt->bindValue(':description', $description, SQLITE3_TEXT);
        $stmt->bindValue(':category', $category, SQLITE3_TEXT);
        $stmt->execute();

        $notifStmt = $db->prepare('INSERT INTO notifications (user_id, title, body) VALUES (:user_id, :title, :body)');
        $notifStmt->bindValue(':user_id', $user['id'], SQLITE3_INTEGER);
        $notifStmt->bindValue(':title', 'Upload ready for review', SQLITE3_TEXT);
        $notifStmt->bindValue(':body', 'Your video "' . $title . '" is now in your library.', SQLITE3_TEXT);
        $notifStmt->execute();

        $success = true;
        $title = '';
        $description = '';
    }
}

$categories = ['Tech', 'Music', 'Education', 'Gaming', 'Lifestyle', 'Sports'];
?>
<!doctype html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ATF Upload Studio</title>
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
            max-width: 900px;
            margin: 32px auto;
            padding: 0 24px 48px;
        }

        .card {
            background: var(--card);
            border-radius: 18px;
            border: 1px solid var(--outline);
            padding: 24px;
        }

        .layout {
            display: grid;
            grid-template-columns: 2fr 1fr;
            gap: 20px;
        }

        form {
            display: grid;
            gap: 16px;
        }

        label {
            font-size: 13px;
            color: var(--muted);
        }

        input, textarea, select {
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
            padding: 10px 16px;
            border-radius: 10px;
            border: none;
            background: var(--accent);
            color: #fff;
            font-weight: 600;
            cursor: pointer;
        }

        .success {
            color: #6ee7b7;
            font-size: 14px;
        }

        .error {
            color: #fca5a5;
            font-size: 14px;
        }

        .grid {
            display: grid;
            gap: 16px;
            grid-template-columns: repeat(auto-fit, minmax(240px, 1fr));
        }

        .checklist {
            display: grid;
            gap: 10px;
            color: var(--muted);
            font-size: 13px;
        }

        .badge {
            display: inline-block;
            padding: 4px 10px;
            border-radius: 999px;
            background: rgba(255, 26, 26, 0.16);
            color: #ffb4b4;
            font-size: 12px;
        }

        @media (max-width: 960px) {
            .layout {
                grid-template-columns: 1fr;
            }
        }
    </style>
</head>
<body>
<header>
    <strong>ATF Upload Studio</strong>
    <div>
        <a class="pill" href="index.php">Home</a>
        <a class="pill" href="library.php">Library</a>
        <a class="pill" href="dashboard.php">Dashboard</a>
        <a class="pill" href="settings.php">Settings</a>
    </div>
</header>
<main>
    <div class="layout">
        <div class="card">
            <h2>Upload a new video</h2>
            <p class="pill">Signed in as <?php echo htmlspecialchars($user['username']); ?></p>

            <?php if ($success): ?>
                <p class="success">Your video draft has been added to ATF!</p>
            <?php endif; ?>

            <?php foreach ($errors as $error): ?>
                <p class="error"><?php echo htmlspecialchars($error); ?></p>
            <?php endforeach; ?>

            <form method="post">
                <div class="grid">
                    <div>
                        <label>Video title</label>
                        <input type="text" name="title" value="<?php echo htmlspecialchars($title); ?>" required>
                    </div>
                    <div>
                        <label>Category</label>
                        <select name="category">
                            <?php foreach ($categories as $option): ?>
                                <option value="<?php echo htmlspecialchars($option); ?>" <?php echo $category === $option ? 'selected' : ''; ?>><?php echo htmlspecialchars($option); ?></option>
                            <?php endforeach; ?>
                        </select>
                    </div>
                </div>
                <div>
                    <label>Description</label>
                    <textarea name="description" required><?php echo htmlspecialchars($description); ?></textarea>
                </div>
                <button type="submit">Publish to ATF</button>
            </form>
        </div>
        <div class="card">
            <h3>Creator snapshot</h3>
            <p class="muted">Your channel momentum this month.</p>
            <p><span class="badge"><?php echo number_format((int) $stats['total']); ?> uploads</span></p>
            <p><span class="badge"><?php echo number_format((int) $stats['views']); ?> total views</span></p>
            <h4>Upload checklist</h4>
            <div class="checklist">
                <span>✔ Write a clear title</span>
                <span>✔ Choose the best category</span>
                <span>✔ Add a helpful description</span>
                <span>✔ Share with your community</span>
            </div>
        </div>
    </div>
</main>
</body>
</html>
