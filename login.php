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

if ($user) {
    header('Location: index.php');
    exit;
}
?>
<!doctype html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ATF Login</title>
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
            min-height: 100vh;
            display: grid;
            place-items: center;
            padding: 24px;
        }

        .layout {
            display: grid;
            grid-template-columns: 1.1fr 0.9fr;
            gap: 24px;
            width: min(960px, 100%);
        }

        .card {
            background: var(--card);
            border-radius: 18px;
            border: 1px solid var(--outline);
            padding: 28px;
            display: grid;
            gap: 16px;
        }

        .side-panel {
            background: linear-gradient(135deg, rgba(255, 26, 26, 0.18), rgba(255, 255, 255, 0.06));
            border-radius: 18px;
            border: 1px solid rgba(255, 255, 255, 0.08);
            padding: 28px;
            display: grid;
            gap: 12px;
        }

        .side-panel ul {
            margin: 0;
            padding-left: 18px;
            color: var(--muted);
            font-size: 14px;
        }

        label {
            font-size: 13px;
            color: var(--muted);
        }

        input {
            width: 100%;
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

        .pill {
            padding: 8px 16px;
            border-radius: 999px;
            background: #232323;
            border: 1px solid var(--outline);
            color: var(--text);
            text-decoration: none;
            font-size: 14px;
        }

        .error {
            color: #fca5a5;
            font-size: 13px;
        }

        @media (max-width: 900px) {
            .layout {
                grid-template-columns: 1fr;
            }
        }
    </style>
</head>
<body>
    <div class="layout">
        <div class="card">
            <div>
                <strong>ATF Login</strong>
                <p class="pill">Welcome back to ATF</p>
            </div>
            <?php if (isset($_GET['success'])): ?>
                <div class="pill">Account created! Log in below.</div>
            <?php elseif (isset($_GET['error'])): ?>
                <div class="error"><?php echo htmlspecialchars($_GET['error']); ?></div>
            <?php endif; ?>
            <form method="post" action="auth.php">
                <input type="hidden" name="action" value="login">
                <label>Username</label>
                <input type="text" name="username" required>
                <label>Password</label>
                <input type="password" name="password" required>
                <button type="submit">Log in</button>
            </form>
            <div>
                <span class="pill">New here?</span>
                <a class="pill" href="signup.php">Create an account</a>
                <a class="pill" href="index.php">Back to home</a>
            </div>
        </div>
        <div class="side-panel">
            <h3>Why creators love ATF</h3>
            <ul>
                <li>Centralized creator dashboard</li>
                <li>Upload studio with guided workflow</li>
                <li>Secure link-based login on any device</li>
                <li>Real-time library insights</li>
            </ul>
            <p class="muted">Log in to keep your audience growing and your library organized.</p>
        </div>
    </div>
</body>
</html>
