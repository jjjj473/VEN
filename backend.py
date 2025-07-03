from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import HTMLResponse
from pydantic import BaseModel

app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)

text_data = {"content": ""}

class TextUpdate(BaseModel):
    content: str

@app.get("/text")
async def get_text():
    return text_data

@app.post("/text")
async def set_text(update: TextUpdate):
    text_data["content"] = update.content
    return {"status": "ok"}

@app.get("/")
async def index():
    with open("index.html", "r", encoding="utf-8") as f:
        html = f.read()
    return HTMLResponse(html)
