<?php
session_start();
require_once __DIR__ . '/db.php';

$db = get_db();
$user = null;

if (isset($_SESSION['user_id'])) {
    $stmt = $db->prepare('SELECT id, username, created_at FROM users WHERE id = :id');
    $stmt->bindValue(':id', $_SESSION['user_id'], SQLITE3_INTEGER);
    $result = $stmt->execute();
    $user = $result->fetchArray(SQLITE3_ASSOC) ?: null;
}

if (!$user) {
    header('Location: index.php?error=Please+log+in+to+view+dashboard');
    exit;
}

$statsStmt = $db->prepare('SELECT COUNT(*) as video_count, COALESCE(SUM(views), 0) as total_views FROM videos WHERE user_id = :user_id');
$statsStmt->bindValue(':user_id', $user['id'], SQLITE3_INTEGER);
$statsResult = $statsStmt->execute();
$stats = $statsResult->fetchArray(SQLITE3_ASSOC) ?: ['video_count' => 0, 'total_views' => 0];

$recentStmt = $db->prepare('SELECT title, views, created_at FROM videos WHERE user_id = :user_id ORDER BY created_at DESC LIMIT 5');
$recentStmt->bindValue(':user_id', $user['id'], SQLITE3_INTEGER);
$recentResult = $recentStmt->execute();
$recent = [];
while ($row = $recentResult->fetchArray(SQLITE3_ASSOC)) {
    $recent[] = $row;
}

$growth = [];
for ($i = 6; $i >= 0; $i--) {
    $date = date('Y-m-d', strtotime('-' . $i . ' days'));
    $growthStmt = $db->prepare('SELECT COUNT(*) as count FROM videos WHERE user_id = :user_id AND date(created_at) = :date');
    $growthStmt->bindValue(':user_id', $user['id'], SQLITE3_INTEGER);
    $growthStmt->bindValue(':date', $date, SQLITE3_TEXT);
    $growthResult = $growthStmt->execute();
    $growthRow = $growthResult->fetchArray(SQLITE3_ASSOC);
    $growth[] = ['date' => $date, 'count' => (int) $growthRow['count']];
}
?>
<!doctype html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ATF Creator Dashboard</title>
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

        .grid {
            display: grid;
            gap: 16px;
            grid-template-columns: repeat(auto-fit, minmax(240px, 1fr));
        }

        .card {
            background: var(--card);
            border-radius: 16px;
            padding: 18px;
            border: 1px solid var(--outline);
        }

        .muted {
            color: var(--muted);
            font-size: 13px;
        }

        .chart {
            display: grid;
            gap: 8px;
        }

        .bar {
            background: #232323;
            border-radius: 999px;
            overflow: hidden;
        }

        .bar span {
            display: block;
            height: 10px;
            background: var(--accent);
        }

        table {
            width: 100%;
            border-collapse: collapse;
            font-size: 14px;
        }

        th, td {
            text-align: left;
            padding: 8px 0;
            border-bottom: 1px solid var(--outline);
        }
    </style>
</head>
<body>
<header>
    <strong>ATF Creator Dashboard</strong>
    <div>
        <a class="pill" href="index.php">Home</a>
        <a class="pill" href="upload.php">Upload</a>
        <a class="pill" href="library.php">Library</a>
        <a class="pill" href="settings.php">Settings</a>
    </div>
</header>
<main>
    <div class="grid">
        <div class="card">
            <h3>Total uploads</h3>
            <p><?php echo number_format((int) $stats['video_count']); ?> videos</p>
            <p class="muted">Since <?php echo htmlspecialchars(date('M Y', strtotime($user['created_at']))); ?></p>
        </div>
        <div class="card">
            <h3>Total views</h3>
            <p><?php echo number_format((int) $stats['total_views']); ?> views</p>
            <p class="muted">Lifetime performance</p>
        </div>
        <div class="card">
            <h3>Weekly uploads</h3>
            <div class="chart">
                <?php $max = max(array_column($growth, 'count')) ?: 1; ?>
                <?php foreach ($growth as $day): ?>
                    <div>
                        <span class="muted"><?php echo htmlspecialchars(date('D', strtotime($day['date']))); ?></span>
                        <div class="bar"><span style="width: <?php echo (int) (($day['count'] / $max) * 100); ?>%"></span></div>
                    </div>
                <?php endforeach; ?>
            </div>
        </div>
    </div>

    <div class="card" style="margin-top: 24px;">
        <h3>Recent uploads</h3>
        <table>
            <thead>
                <tr>
                    <th>Title</th>
                    <th>Views</th>
                    <th>Date</th>
                </tr>
            </thead>
            <tbody>
                <?php if (count($recent) === 0): ?>
                    <tr>
                        <td colspan="3" class="muted">No uploads yet.</td>
                    </tr>
                <?php else: ?>
                    <?php foreach ($recent as $video): ?>
                        <tr>
                            <td><?php echo htmlspecialchars($video['title']); ?></td>
                            <td><?php echo number_format((int) $video['views']); ?></td>
                            <td><?php echo htmlspecialchars(date('M j, Y', strtotime($video['created_at']))); ?></td>
                        </tr>
                    <?php endforeach; ?>
                <?php endif; ?>
            </tbody>
        </table>
    </div>
</main>
</body>
</html>
