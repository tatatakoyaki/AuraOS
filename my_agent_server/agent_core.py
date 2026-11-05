import os
os.environ["HF_ENDPOINT"] = "https://hf-mirror.com"
import io
import wave
import requests
import asyncio
import edge_tts # 👈 新增：微软 TTS 库
from langchain_openai import ChatOpenAI
from langchain.agents import create_agent
from langchain_core.tools import tool
from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from pydantic import BaseModel
from faster_whisper import WhisperModel
import uvicorn

# ==========================================
# 1. 初始化千问大模型 (LangChain Agent 部分)
# ==========================================
llm = ChatOpenAI(
    api_key=os.getenv("DASHSCOPE_API_KEY", "请在此处替换为你的千问API_KEY"),
    base_url="https://ws-qh6vzs93hc9xd7eh.cn-beijing.maas.aliyuncs.com/compatible-mode/v1",
    model="qwen3.7-plus",
    temperature=0.1 
)

# ==========================================
# 2. 定义工具（向 C++ 发 HTTP 请求）
# ==========================================
CPP_SERVER_URL = "http://127.0.0.1:8003/execute"

@tool
def control_ac_temp(temperature: int) -> str:
    """调节空调温度。参数 temperature 必须在 16 到 30 之间。"""
    print(f"🔥【Agent 决策】准备设置空调温度为 {temperature}，向 C++ 发送指令...")
    try:
        res = requests.post(CPP_SERVER_URL, json={
            "tool": "set_ac_temp",
            "args": {"temperature": temperature}
        }, timeout=3)
        if res.status_code == 200:
            return f"执行成功，空调已设为 {temperature} 度。"
        else:
            return "执行失败，车辆系统拒绝执行该指令。"
    except Exception as e:
        print(f"❌ 发送给 C++ 失败: {e}")
        return "网络连接失败，无法控制车辆。"

@tool
def play_music(song_name: str) -> str:
    """播放指定名字的歌曲。"""
    print(f"🎵【Agent 决策】准备播放音乐 {song_name}，向 C++ 发送指令...")
    try:
        res = requests.post(CPP_SERVER_URL, json={
            "tool": "play_music",
            "args": {"song_name": song_name}
        }, timeout=3)
        if res.status_code == 200:
            return f"执行成功，正在播放 {song_name}。"
        else:
            return "执行失败，系统拒绝执行。"
    except Exception as e:
        return "网络连接失败，无法播放音乐。"

@tool
def open_system_page(page_name: str) -> str:
    """打开系统页面。参数 page_name 只能是 'settings' 或 'home'。"""
    print(f"⚙️【Agent 决策】准备打开页面 {page_name}，向 C++ 发送指令...")
    try:
        res = requests.post(CPP_SERVER_URL, json={
            "tool": "open_page",
            "args": {"page_name": page_name}
        }, timeout=3)
        if res.status_code == 200:
            return f"执行成功，已打开 {page_name} 页面。"
        else:
            return "执行失败，系统拒绝执行。"
    except Exception as e:
        return "网络连接失败，无法切换页面。"

tools = [control_ac_temp, play_music, open_system_page]

# ==========================================
# 3. 组装核心引擎
# ==========================================
agent = create_agent(
    model=llm,
    tools=tools,
    system_prompt="""你是一个车机控制中枢。
【绝对规则】
1. 如果用户的意图是控制空调、音乐或页面，你【必须】调用相应的工具（tool），绝对不允许只用文字回答！
2. 对于“调低一点/调高一点”这种模糊指令，你可以基于常识推测一个合理的值（比如 22 度或 26 度），但必须调用 control_ac_temp 工具！
3. 你的回复文字只能在工具执行完毕后，作为辅助说明输出。
"""
)

# 全局记忆列表
chat_history = []

# 封装一个 Agent 推理函数，供文本和语音接口共用
def run_agent_logic(user_text: str) -> str:
    chat_history.append(("user", user_text))
    response = agent.invoke({"messages": chat_history})
    ai_reply = response["messages"][-1].content
    chat_history.append(("assistant", ai_reply))
    return ai_reply

# ==========================================
# 4. 初始化本地 faster-whisper 模型 (耳朵)
# ==========================================
print("⏳ 正在加载 Whisper 模型，请稍候...")
whisper_model = WhisperModel("base", device="cpu", compute_type="int8")
print("✅ Whisper 模型加载成功！")

# ==========================================
# 5. FastAPI 服务端定义
# ==========================================
app = FastAPI()

class ChatRequest(BaseModel):
    message: str

@app.post("/chat")
async def chat_endpoint(request: ChatRequest):
    print(f"\n👤 【收到文本请求】用户说: {request.message}")
    ai_reply = run_agent_logic(request.message)
    print(f"🤖 【Agent 回复】: {ai_reply}")
    return {"reply": ai_reply}

@app.websocket("/ws_audio")
async def websocket_audio_endpoint(websocket: WebSocket):
    await websocket.accept()
    print("\n✅ C++ 语音流已连接！")
    
    audio_buffer = io.BytesIO()

    try:
        while True:
            data = await websocket.receive()
            if "bytes" in data:
                print(f"🎵 收到音频块: {len(data['bytes'])} 字节")
                audio_buffer.write(data["bytes"])
            elif "text" in data and data["text"] == "STOP":
                print("🛑 收到结束指令，开始识别...")
                break
    except WebSocketDisconnect:
        print("❌ C++ 语音流意外断开")
        return

    # --- 处理音频 ---
    raw_pcm_data = audio_buffer.getvalue()
    audio_buffer.close()
    
    if len(raw_pcm_data) < 1000:
        await websocket.send_text("说话时间太短啦")
        return

    # 给裸 PCM 加 WAV 头 (严格对齐 C++ 的 48000Hz, 双声道)
    wav_buffer = io.BytesIO()
    with wave.open(wav_buffer, 'wb') as wf:
        wf.setnchannels(2)       
        wf.setsampwidth(2)       
        wf.setframerate(48000)   
        wf.writeframes(raw_pcm_data)
    wav_buffer.seek(0)

    # 1. ASR 识别
    print("🧠 正在识别音频...")
    segments, info = whisper_model.transcribe(wav_buffer, language="zh")
    text = "".join([segment.text for segment in segments]).strip()
    print(f"🎙️ 用户说：【{text}】")
    
    if not text:
        await websocket.send_text("我没有听清，请再说一遍")
        return

    # 2. 交给 LangChain Agent 进行推理
    print("🤖 正在请求大模型...")
    ai_reply = run_agent_logic(text)
    print(f"✅ 最终回复给车机：【{ai_reply}】")

    # 3. 先把文字发给 C++，让 UI 立刻显示
    await websocket.send_text(ai_reply)

    # ==========================================
    # 👇 4. 核心新增：生成 TTS 音频并发送回 C++
    # ==========================================
    print("🔊 正在生成 AI 语音播报...")
    voice = "zh-CN-XiaoxiaoNeural" # 晓晓（自然女声，非常逼真）
    
    try:
        communicate = edge_tts.Communicate(ai_reply, voice)
        # 不存硬盘，直接在内存中收集音频流（企业级做法）
        tts_audio_buffer = io.BytesIO()
        async for chunk in communicate.stream():
            if chunk["type"] == "audio":
                tts_audio_buffer.write(chunk["data"])
        
        tts_audio_data = tts_audio_buffer.getvalue()
        tts_audio_buffer.close()
        
        print(f"🎵 TTS 音频生成完毕，大小: {len(tts_audio_data)} 字节。正在发送给 C++...")
        
        # 👇 发送二进制音频数据给 C++
        await websocket.send_bytes(tts_audio_data)
        print("✅ 音频已发送！")
        
    except Exception as e:
        print(f"❌ TTS 生成或发送失败: {e}")

if __name__ == "__main__":
    print("🚀 正在启动车载 Agent 网关...")
    uvicorn.run(app, host="127.0.0.1", port=8002)
