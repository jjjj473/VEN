import requests
import sys

BASE_URL = "http://localhost:8000"


def list_videos():
    r = requests.get(f"{BASE_URL}/videos")
    r.raise_for_status()
    return r.json().get("videos", [])


def upload_videos(paths):
    files = [("files", open(p, "rb")) for p in paths]
    try:
        r = requests.post(f"{BASE_URL}/upload", files=files)
        r.raise_for_status()
        return r.json()
    finally:
        for _, f in files:
            f.close()


if __name__ == "__main__":
    if len(sys.argv) == 1:
        for v in list_videos():
            print(v)
    else:
        info = upload_videos(sys.argv[1:])
        print(info)

