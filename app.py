import os
from fastapi import FastAPI, UploadFile, File
from fastapi.middleware.cors import CORSMiddleware
from fastapi.staticfiles import StaticFiles
from typing import List

app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)

VIDEOS_DIR = "videos"
os.makedirs(VIDEOS_DIR, exist_ok=True)

@app.get("/videos")
async def list_videos():
    files = [f for f in os.listdir(VIDEOS_DIR)
             if os.path.isfile(os.path.join(VIDEOS_DIR, f))]
    return {"videos": files}

@app.post("/upload")
async def upload_videos(files: List[UploadFile] = File(...)):
    uploaded = []
    for file in files:
        file_path = os.path.join(VIDEOS_DIR, file.filename)
        with open(file_path, "wb") as out:
            out.write(await file.read())
        uploaded.append(file.filename)
    return {"uploaded": uploaded}

app.mount("/videos", StaticFiles(directory=VIDEOS_DIR), name="videos")

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)
