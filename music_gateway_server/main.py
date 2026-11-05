"""
⚠️ 【免责声明】
1. 本代码仅供个人学习、技术研究与 Qt 客户端开发测试使用，严禁用于任何商业用途。
2. 本接口为非官方公开接口抓取，可能存在版权限制、随时失效或变更。
3. 生产环境/商业项目请务必接入网易云官方正版 API/SDK，尊重音乐版权。
"""
from fastapi import FastAPI
from pydantic import BaseModel
import requests

app = FastAPI()

class SearchRequest(BaseModel):
    keyword: str

@app.post("/search_song")
async def search_song(request: SearchRequest):
    try:
        headers = {
            "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36",
            "Referer": "https://music.163.com/"
        }

        # 1. 搜索歌曲
        search_url = "http://music.163.com/api/search/get/web"
        params = {"s": request.keyword, "type": 1, "offset": 0, "limit": 1}
        res = requests.get(search_url, headers=headers, params=params).json()

        if not res.get('result') or not res['result'].get('songs'):
            return {"error": "没找到这首歌"}

        song_info = res['result']['songs'][0]
        song_id = song_info['id']
        song_name = song_info['name']
        artist_name = song_info['artists'][0]['name']

        # 2. 获取歌曲详情（封面）
        detail_url = f"http://music.163.com/api/song/detail/?id={song_id}&ids=[{song_id}]"
        detail_res = requests.get(detail_url, headers=headers).json()
        cover_url = ""
        if detail_res.get('songs'):
            cover_url = detail_res['songs'][0]['album']['picUrl']

        # 3. 获取歌词
        lyric_url = f"http://music.163.com/api/song/lyric?id={song_id}&lv=1&kv=1&tv=-1"
        lyric_res = requests.get(lyric_url, headers=headers).json()
        lyric_text = lyric_res.get('lrc', {}).get('lyric', "暂无歌词")

         # 4. 【关键】使用官方内部接口获取真实音频链接
        audio_api = f"http://music.163.com/api/song/enhance/player/url?ids=[{song_id}]&br=320000"
        
        # 请求这个内部 API
        audio_res = requests.get(audio_api, headers=headers).json()
        
        # 请求这个内部 API
        audio_res = requests.get(audio_api, headers=headers).json()

        # 检查返回的数据里有没有可用的 url
        if audio_res.get('data') and audio_res['data'][0].get('url'):
            final_audio_url = audio_res['data'][0]['url']
        else:
            return {"error": "获取音频直链失败，可能是版权限制或VIP歌曲"}

        # 5. 返回完整数据包
        return {
            "song_name": song_name,
            "artist": artist_name,
            "cover_url": cover_url,
            "lyrics": lyric_text,
            "play_url": final_audio_url  # 返回真实的 CDN 音频地址
        }
    except Exception as e:
        return {"error": str(e)}

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="127.0.0.1", port=8001)
