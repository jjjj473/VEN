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
    header('Location: index.php?error=Please+log+in+to+view+your+library');
    exit;
}

$search = trim($_GET['search'] ?? '');
$category = trim($_GET['category'] ?? '');
$categories = ['','Tech', 'Music', 'Education', 'Gaming', 'Lifestyle', 'Sports'];

$query = 'SELECT id, title, description, category, views, created_at FROM videos WHERE user_id = :user_id';
$params = [':user_id' => $user['id']];

if ($search !== '') {
    $query .= ' AND (title LIKE :search OR description LIKE :search)';
    $params[':search'] = '%' . $search . '%';
}

if ($category !== '') {
    $query .= ' AND category = :category';
    $params[':category'] = $category;
}

$query .= ' ORDER BY created_at DESC';

$stmt = $db->prepare($query);
foreach ($params as $key => $value) {
    $stmt->bindValue($key, $value, is_int($value) ? SQLITE3_INTEGER : SQLITE3_TEXT);
}

$result = $stmt->execute();
$videos = [];
while ($row = $result->fetchArray(SQLITE3_ASSOC)) {
    $videos[] = $row;
}

$summaryStmt = $db->prepare('SELECT COUNT(*) as total_videos, COALESCE(SUM(views), 0) as total_views FROM videos WHERE user_id = :user_id');
$summaryStmt->bindValue(':user_id', $user['id'], SQLITE3_INTEGER);
$summaryResult = $summaryStmt->execute();
$summary = $summaryResult->fetchArray(SQLITE3_ASSOC) ?: ['total_videos' => 0, 'total_views' => 0];
?>
<!doctype html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ATF Library</title>
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
            max-width: 1100px;
            margin: 32px auto;
            padding: 0 24px 48px;
        }

        .filters {
            display: grid;
            gap: 12px;
            grid-template-columns: 2fr 1fr auto;
            margin-bottom: 20px;
        }

        .summary {
            display: grid;
            gap: 16px;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            margin-bottom: 20px;
        }

        .summary-card {
            background: var(--card);
            border-radius: 16px;
            padding: 16px;
            border: 1px solid var(--outline);
            display: grid;
            gap: 6px;
        }

        .summary-card strong {
            font-size: 20px;
        }

        input, select {
            padding: 10px 12px;
            border-radius: 10px;
            border: 1px solid var(--outline);
            background: #141414;
            color: var(--text);
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

        .grid {
            display: grid;
            gap: 16px;
            grid-template-columns: repeat(auto-fit, minmax(240px, 1fr));
        }

        .card {
            background: var(--card);
            border-radius: 16px;
            padding: 16px;
            border: 1px solid var(--outline);
        }

        .muted {
            color: var(--muted);
            font-size: 13px;
        }
    </style>
</head>
<body>
<header>
    <strong>ATF Library</strong>
    <div>
        <a class="pill" href="index.php">Home</a>
        <a class="pill" href="upload.php">Upload</a>
        <a class="pill" href="dashboard.php">Dashboard</a>
        <a class="pill" href="settings.php">Settings</a>
    </div>
</header>
<main>
    <p class="muted">Showing videos for <?php echo htmlspecialchars($user['username']); ?></p>
    <div class="summary">
        <div class="summary-card">
            <span class="muted">Total videos</span>
            <strong><?php echo number_format((int) $summary['total_videos']); ?></strong>
        </div>
        <div class="summary-card">
            <span class="muted">Total views</span>
            <strong><?php echo number_format((int) $summary['total_views']); ?></strong>
        </div>
        <div class="summary-card">
            <span class="muted">Current filter</span>
            <strong><?php echo $category === '' ? 'All' : htmlspecialchars($category); ?></strong>
        </div>
    </div>
    <form class="filters" method="get">
        <input type="text" name="search" placeholder="Search by title or description" value="<?php echo htmlspecialchars($search); ?>">
        <select name="category">
            <?php foreach ($categories as $option): ?>
                <option value="<?php echo htmlspecialchars($option); ?>" <?php echo $category === $option ? 'selected' : ''; ?>><?php echo $option === '' ? 'All categories' : htmlspecialchars($option); ?></option>
            <?php endforeach; ?>
        </select>
        <button type="submit">Filter</button>
    </form>

    <div class="grid">
        <?php if (count($videos) === 0): ?>
            <div class="card">
                <h3>No videos found</h3>
                <p class="muted">Try a different search or upload something new.</p>
            </div>
        <?php else: ?>
            <?php foreach ($videos as $video): ?>
                <div class="card">
                    <h3><?php echo htmlspecialchars($video['title']); ?></h3>
                    <p class="muted"><?php echo htmlspecialchars($video['category']); ?> · <?php echo number_format((int) $video['views']); ?> views</p>
                    <p><?php echo htmlspecialchars($video['description']); ?></p>
                    <p class="muted">Published <?php echo htmlspecialchars(date('M j, Y', strtotime($video['created_at']))); ?></p>
                </div>
            <?php endforeach; ?>
        <?php endif; ?>
    </div>
</main>
</body>
</html>
