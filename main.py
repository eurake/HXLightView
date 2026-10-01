import network
import socket
import time
import select

# ---------- 1. WiFi AP模式 ----------
ap = network.WLAN(network.AP_IF)
ap.active(True)
ap.config(essid='ESP32-Device', password='12345678', authmode=network.AUTH_WPA2_PSK)
print('AP热点已开启:', ap.ifconfig())

# ---------- 2. CH390以太网初始化 ----------
eth = network.CH390(spi_id, cs=cs_pin, int_pin=int_pin)
eth.active(True)

def eth_connect(timeout_s=10):
    t = 0
    while not eth.isconnected() and t < timeout_s:
        time.sleep(1)
        t += 1
    return eth.isconnected()

if eth_connect():
    print('以太网已连接:', eth.ifconfig())

# ---------- 3. TCP客户端配置 ----------
SERVER_IP = '192.168.1.100'
SERVER_PORT = 8899
TCP_RETRY_INTERVAL = 20   # 断线重连间隔
UPLOAD_INTERVAL = 20      # 数据上传间隔
DEVICE_ID = 'ESP32-001'   # 设备唯一标识

tcp_sock = None
last_tcp_try = 0
last_upload = 0
handshake_done = False  # 标记ID是否已上报

def tcp_connect():
    global tcp_sock, handshake_done
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(5)
        s.connect((SERVER_IP, SERVER_PORT))
        s.setblocking(False)  # 连接建立后切非阻塞,用于后续收发
        print('TCP服务器连接成功')
        tcp_sock = s
        handshake_done = False
        return s
    except Exception as e:
        print('TCP连接失败:', e)
        tcp_sock = None
        return None

def tcp_close():
    global tcp_sock, handshake_done
    if tcp_sock:
        try:
            tcp_sock.close()
        except:
            pass
    tcp_sock = None
    handshake_done = False

def send_handshake():
    """连接成功后第一次上报设备ID"""
    global handshake_done
    try:
        msg = 'ID:%s\n' % DEVICE_ID
        tcp_sock.send(msg.encode())
        handshake_done = True
        print('已上报设备ID:', DEVICE_ID)
    except Exception as e:
        print('ID上报失败:', e)
        tcp_close()

def send_data():
    """定时上传业务数据"""
    try:
        payload = 'DATA:temp=25.6,status=ok\n'  # 替换成你的实际数据
        tcp_sock.send(payload.encode())
        print('数据已上传')
    except Exception as e:
        print('数据上传失败:', e)
        tcp_close()

def parse_and_reply(raw_data):
    """解析服务器下发数据,返回响应内容"""
    try:
        text = raw_data.decode().strip()
        print('收到服务器数据:', text)

        # 按你的协议解析,这里假设格式为 CMD:xxx
        if text.startswith('CMD:'):
            cmd = text[4:]
            if cmd == 'STATUS':
                reply = 'ACK:STATUS_OK\n'
            elif cmd == 'RESET':
                reply = 'ACK:RESET_RECEIVED\n'
                # 这里可以加实际复位逻辑
            else:
                reply = 'ACK:UNKNOWN_CMD\n'
        else:
            reply = 'ACK:INVALID_FORMAT\n'

        tcp_sock.send(reply.encode())
        print('已回复服务器:', reply.strip())

    except Exception as e:
        print('解析服务器数据异常:', e)

def check_recv():
    """非阻塞检查是否有服务器下发数据"""
    try:
        r, _, _ = select.select([tcp_sock], [], [], 0)  # 0秒超时,立即返回
        if r:
            data = tcp_sock.recv(256)
            if data:
                parse_and_reply(data)
            else:
                # recv返回空说明对端主动关闭连接
                print('服务器已断开连接')
                tcp_close()
    except OSError:
        # 非阻塞模式下没数据时会抛EAGAIN,属于正常情况,忽略即可
        pass
    except Exception as e:
        print('接收异常:', e)
        tcp_close()

# ---------- 4. 主循环 ----------
last_eth_check = time.time()
ETH_CHECK_INTERVAL = 5

while True:
    try:
        now = time.time()

        # --- 以太网链路检测 ---
        if now - last_eth_check >= ETH_CHECK_INTERVAL:
            last_eth_check = now
            if not eth.isconnected():
                print('以太网断开,尝试重连...')
                eth.active(False)
                time.sleep(0.5)
                eth.active(True)
                if eth_connect(timeout_s=5):
                    print('以太网重连成功:', eth.ifconfig())
                tcp_close()  # 网断了TCP肯定失效

        if eth.isconnected():
            # --- TCP连接维护 ---
            if tcp_sock is None:
                if now - last_tcp_try >= TCP_RETRY_INTERVAL:
                    last_tcp_try = now
                    if tcp_connect():
                        send_handshake()  # 连接成功立即上报ID
                        last_upload = now  # 重置上传计时,避免立刻又发一次
            else:
                # --- 已连接:定时上传 + 实时接收 ---
                check_recv()  # 每次循环都检查一次服务器下发数据,非阻塞

                if handshake_done and now - last_upload >= UPLOAD_INTERVAL:
                    last_upload = now
                    send_data()
        else:
            tcp_close()

    except Exception as e:
        print('主循环异常:', e)

    time.sleep(0.5)  # 缩短到0.5秒,让接收响应更及时