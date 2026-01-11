<?php
session_start();
require_once __DIR__ . '/db.php';

$db = get_db();
$user = null;

if (isset($_SESSION['user_id'])) {
    $stmt = $db->prepare('SELECT users.id, users.username, users.qr_token, profiles.display_name FROM users LEFT JOIN profiles ON users.id = profiles.user_id WHERE users.id = :id');
    $stmt->bindValue(':id', $_SESSION['user_id'], SQLITE3_INTEGER);
    $result = $stmt->execute();
    $user = $result->fetchArray(SQLITE3_ASSOC) ?: null;
}

$videos = [];
$videoStmt = $db->prepare('SELECT videos.id, videos.title, videos.description, videos.category, videos.views, videos.created_at, users.username FROM videos JOIN users ON videos.user_id = users.id ORDER BY videos.created_at DESC LIMIT 8');
$videoResult = $videoStmt->execute();
while ($row = $videoResult->fetchArray(SQLITE3_ASSOC)) {
    $videos[] = $row;
}

$userStats = null;
if ($user) {
    $statsStmt = $db->prepare('SELECT COUNT(*) as video_count, COALESCE(SUM(views), 0) as total_views FROM videos WHERE user_id = :user_id');
    $statsStmt->bindValue(':user_id', $user['id'], SQLITE3_INTEGER);
    $statsResult = $statsStmt->execute();
    $userStats = $statsResult->fetchArray(SQLITE3_ASSOC) ?: ['video_count' => 0, 'total_views' => 0];
}

$platformStmt = $db->prepare('SELECT (SELECT COUNT(*) FROM users) as creators, (SELECT COUNT(*) FROM videos) as uploads');
$platformResult = $platformStmt->execute();
$platformStats = $platformResult->fetchArray(SQLITE3_ASSOC) ?: ['creators' => 0, 'uploads' => 0];

$qrLoginUrl = null;
if ($user) {
    $qrLoginUrl = sprintf('%s://%s%s/qr_login.php?token=%s',
        (!empty($_SERVER['HTTPS']) && $_SERVER['HTTPS'] !== 'off') ? 'https' : 'http',
        $_SERVER['HTTP_HOST'],
        rtrim(dirname($_SERVER['PHP_SELF']), '/'),
        urlencode($user['qr_token'])
    );
}
?>
<!doctype html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ATF | Media Platform</title>
    <style>
        :root {
            color-scheme: light;
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
            margin: 0;
            padding: 0;
        }

        body {
            background: var(--bg);
            color: var(--text);
            min-height: 100vh;
        }

        header {
            display: flex;
            align-items: center;
            justify-content: space-between;
            padding: 18px 32px;
            border-bottom: 1px solid var(--outline);
            position: sticky;
            top: 0;
            background: rgba(15, 15, 15, 0.92);
            backdrop-filter: blur(12px);
            z-index: 10;
        }

        .brand {
            display: flex;
            gap: 12px;
            align-items: center;
        }

        .logo {
            width: 40px;
            height: 40px;
            border-radius: 12px;
            background: var(--accent);
            display: grid;
            place-items: center;
            font-weight: 700;
            font-size: 18px;
        }

        .search {
            flex: 1;
            max-width: 520px;
            margin: 0 32px;
            display: flex;
            align-items: center;
            background: #181818;
            border-radius: 999px;
            padding: 10px 18px;
            border: 1px solid var(--outline);
        }

        .search input {
            background: transparent;
            border: none;
            color: var(--text);
            width: 100%;
            font-size: 15px;
            outline: none;
        }

        .header-actions {
            display: flex;
            gap: 16px;
            align-items: center;
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

        .pill.soft {
            background: rgba(255, 255, 255, 0.06);
            border-color: rgba(255, 255, 255, 0.08);
        }

        .hero-card {
            background: linear-gradient(135deg, rgba(255, 26, 26, 0.18), rgba(255, 255, 255, 0.04));
            border-radius: 20px;
            padding: 24px;
            border: 1px solid rgba(255, 255, 255, 0.08);
            display: grid;
            gap: 12px;
        }

        .hero-card h1 {
            font-size: 28px;
        }

        .hero-card p {
            color: var(--muted);
            line-height: 1.5;
        }

        .stat-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
            gap: 16px;
            margin: 16px 0 24px;
        }

        .stat-card {
            background: var(--card);
            border-radius: 16px;
            padding: 16px;
            border: 1px solid var(--outline);
            display: grid;
            gap: 6px;
        }

        .stat-card strong {
            font-size: 20px;
        }

        .tag-list {
            display: flex;
            flex-wrap: wrap;
            gap: 8px;
        }

        .chip {
            padding: 6px 12px;
            border-radius: 999px;
            background: #1a1a1a;
            border: 1px solid var(--outline);
            color: var(--muted);
            font-size: 12px;
        }

        .section-stack {
            display: grid;
            gap: 20px;
        }

        main {
            display: grid;
            grid-template-columns: 260px 1fr;
            gap: 24px;
            padding: 28px 32px 48px;
        }

        aside {
            display: flex;
            flex-direction: column;
            gap: 16px;
        }

        .nav-card {
            background: var(--card);
            padding: 18px;
            border-radius: 16px;
            border: 1px solid var(--outline);
        }

        .nav-card h3 {
            margin-bottom: 12px;
            font-size: 15px;
        }

        .nav-card ul {
            list-style: none;
            display: grid;
            gap: 10px;
            color: var(--muted);
            font-size: 14px;
        }

        .content-header {
            display: flex;
            align-items: center;
            justify-content: space-between;
            margin-bottom: 24px;
        }

        .content-header h2 {
            font-size: 24px;
        }

        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(260px, 1fr));
            gap: 20px;
        }

        .video-card {
            background: var(--card);
            border-radius: 18px;
            overflow: hidden;
            border: 1px solid var(--outline);
            transition: transform 0.2s ease, box-shadow 0.2s ease;
        }

        .video-card:hover {
            transform: translateY(-4px);
            box-shadow: 0 12px 30px rgba(0, 0, 0, 0.35);
        }

        .thumb {
            height: 150px;
            background: linear-gradient(135deg, #2b2b2b, #3a1a1a);
            display: flex;
            align-items: center;
            justify-content: center;
            font-weight: 600;
        }

        .video-info {
            padding: 16px;
            display: grid;
            gap: 6px;
        }

        .video-info h4 {
            font-size: 15px;
        }

        .video-meta {
            color: var(--muted);
            font-size: 13px;
        }

        .video-desc {
            font-size: 12px;
            color: #cfcfcf;
            line-height: 1.4;
        }

        .empty-state {
            background: var(--card);
            border-radius: 18px;
            border: 1px dashed var(--outline);
            padding: 32px;
            text-align: center;
            color: var(--muted);
        }

        .auth-card {
            background: var(--card);
            border-radius: 16px;
            padding: 20px;
            border: 1px solid var(--outline);
            display: grid;
            gap: 16px;
        }

        .auth-card h3 {
            font-size: 16px;
        }

        .auth-card form {
            display: grid;
            gap: 12px;
        }

        label {
            font-size: 13px;
            color: var(--muted);
        }

        input[type="text"],
        input[type="password"] {
            padding: 10px 12px;
            border-radius: 10px;
            border: 1px solid var(--outline);
            background: #141414;
            color: var(--text);
        }

        button {
            padding: 10px 14px;
            border-radius: 10px;
            border: none;
            background: var(--accent);
            color: #fff;
            font-weight: 600;
            cursor: pointer;
        }

        .qr-card {
            display: grid;
            gap: 12px;
            text-align: center;
        }

        .qr-card img {
            width: 180px;
            height: 180px;
            border-radius: 12px;
            border: 1px solid var(--outline);
            background: #fff;
        }

        .success {
            color: #6ee7b7;
            font-size: 13px;
        }

        .error {
            color: #fca5a5;
            font-size: 13px;
        }

        .muted {
            color: var(--muted);
            font-size: 13px;
        }

        @media (max-width: 960px) {
            main {
                grid-template-columns: 1fr;
            }

            header {
                flex-wrap: wrap;
                gap: 16px;
            }

            .search {
                order: 3;
                margin: 0;
                width: 100%;
            }
        }
    </style>
</head>
<body>
<header>
    <div class="brand">
        <div class="logo">ATF</div>
        <div>
            <strong>ATF Media</strong><br>
            <span class="muted">Stream. Create. Connect.</span>
        </div>
    </div>
    <div class="search">
        <input type="text" placeholder="Search videos, creators, or topics...">
    </div>
    <div class="header-actions">
        <?php if ($user): ?>
            <span class="pill">Hi, <?php echo htmlspecialchars(($user['display_name'] ?? '') !== '' ? $user['display_name'] : $user['username']); ?></span>
            <a class="pill" href="logout.php">Logout</a>
        <?php else: ?>
            <a class="pill" href="login.php">Log in</a>
            <a class="pill" href="signup.php">Sign up</a>
        <?php endif; ?>
    </div>
</header>
<main>
    <aside>
        <div class="nav-card">
            <h3>Quick Access</h3>
            <ul>
                <li>Home Feed</li>
                <li>Trending Now</li>
                <li>Subscriptions</li>
                <li>ATF Originals</li>
                <li>Watch Later</li>
            </ul>
        </div>
        <div class="nav-card">
            <h3>Creator Tools</h3>
            <ul>
                <li><a class="pill" href="upload.php">Upload Studio</a></li>
                <li><a class="pill" href="library.php">Video Library</a></li>
                <li><a class="pill" href="dashboard.php">Creator Dashboard</a></li>
                <li><a class="pill" href="settings.php">Account Settings</a></li>
            </ul>
        </div>
        <div class="nav-card">
            <h3>Trending tags</h3>
            <div class="tag-list">
                <span class="chip">#ATFOriginals</span>
                <span class="chip">#CreatorSprint</span>
                <span class="chip">#LiveCoding</span>
                <span class="chip">#StudioTips</span>
                <span class="chip">#DailyUpload</span>
            </div>
        </div>
        <?php if ($user): ?>
            <div class="nav-card qr-card">
                <h3>Login on your phone</h3>
                <p class="muted">Use your secure login link on any device.</p>
                <p class="muted">Link: <?php echo htmlspecialchars($qrLoginUrl); ?></p>
            </div>
            <div class="nav-card">
                <h3>Creator pulse</h3>
                <p class="muted">Your creator stats this month.</p>
                <div class="stat-grid">
                    <div class="stat-card">
                        <span class="muted">Uploads</span>
                        <strong><?php echo number_format((int) $userStats['video_count']); ?></strong>
                    </div>
                    <div class="stat-card">
                        <span class="muted">Views</span>
                        <strong><?php echo number_format((int) $userStats['total_views']); ?></strong>
                    </div>
                </div>
            </div>
        <?php endif; ?>
    </aside>

    <section class="section-stack">
        <div class="hero-card">
            <h1>Welcome to ATF Media</h1>
            <p>Build your channel, ship videos fast, and keep your audience engaged with next-level creator tools.</p>
            <div>
                <a class="pill" href="upload.php">Start uploading</a>
                <a class="pill soft" href="library.php">Manage library</a>
            </div>
        </div>
        <div class="stat-grid">
            <div class="stat-card">
                <span class="muted">Active creators</span>
                <strong><?php echo number_format((int) $platformStats['creators']); ?></strong>
            </div>
            <div class="stat-card">
                <span class="muted">New uploads</span>
                <strong><?php echo number_format((int) $platformStats['uploads']); ?></strong>
            </div>
            <div class="stat-card">
                <span class="muted">Categories</span>
                <strong>6</strong>
            </div>
        </div>
        <div class="content-header">
            <h2>ATF Home</h2>
            <span class="muted">Personalized for you</span>
        </div>
        <div class="grid">
            <?php if (count($videos) === 0): ?>
                <div class="empty-state">
                    <h4>No videos yet</h4>
                    <p>Start uploading to build your ATF library.</p>
                    <p><a class="pill" href="upload.php">Upload your first video</a></p>
                </div>
            <?php else: ?>
                <?php foreach ($videos as $index => $video): ?>
                    <article class="video-card">
                        <div class="thumb">ATF Video <?php echo $index + 1; ?></div>
                        <div class="video-info">
                            <h4><?php echo htmlspecialchars($video['title']); ?></h4>
                            <div class="video-meta">by <?php echo htmlspecialchars($video['username']); ?></div>
                            <div class="video-desc"><?php echo htmlspecialchars($video['description']); ?></div>
                            <div class="video-meta"><?php echo htmlspecialchars($video['category']); ?> · <?php echo number_format((int) $video['views']); ?> views · <?php echo htmlspecialchars(date('M j, Y', strtotime($video['created_at']))); ?></div>
                        </div>
                    </article>
                <?php endforeach; ?>
            <?php endif; ?>
        </div>
    </section>

    <section>
        <div class="content-header">
            <h2><?php echo $user ? 'Welcome back' : 'Join ATF'; ?></h2>
            <span class="muted">Secure login & signup</span>
        </div>
        <div class="auth-card">
            <?php if (isset($_GET['success'])): ?>
                <div class="success">Account created! You can log in now.</div>
            <?php elseif (isset($_GET['error'])): ?>
                <div class="error"><?php echo htmlspecialchars($_GET['error']); ?></div>
            <?php endif; ?>

            <?php if (!$user): ?>
                <div>
                    <h3>Log in or create an account</h3>
                    <p class="muted">Use the dedicated pages for a focused sign-in experience.</p>
                    <p>
                        <a class="pill" href="login.php">Log in</a>
                        <a class="pill" href="signup.php">Sign up</a>
                    </p>
                </div>
            <?php else: ?>
                <p class="muted">You are signed in. Use the secure link in Settings to log in on another device.</p>
            <?php endif; ?>
        </div>
    </section>
</main>
</body>
</html>
