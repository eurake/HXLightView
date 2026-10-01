import network
import socket
import time
import select

# ============================================================
# 1. WiFi AP模式 - 本地热点(用于手机/PC查看设备状态网页)
# ============================================================
ap = network.WLAN(network.AP_IF)
ap.active(True)
ap.config(essid='ESP32-Device', password='12345678', authmode=network.AUTH_WPA2_PSK)
print('AP热点已开启:', ap.ifconfig())

# ============================================================
# 2. CH390以太网初始化
# ============================================================
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
else:
    print('以太网首次连接失败,进入主循环后台重试')

# ============================================================
# 3. TCP客户端配置(连接业务服务器)
# ============================================================
SERVER_IP = '192.168.1.100'
SERVER_PORT = 8899
TCP_RETRY_INTERVAL = 20
UPLOAD_INTERVAL = 20
DEVICE_ID = 'ESP32-001'

tcp_sock = None
last_tcp_try = 0
last_upload = 0
handshake_done = False

# 状态记录,供网页展示使用
status = {
    'tcp_connected': False,
    'last_upload_time': '从未',
    'last_upload_content': '',
    'last_recv_time': '从未',
    'last_recv_content': '',
    'last_reply_content': '',
    'upload_count': 0,
    'recv_count': 0,
}


def now_str():
    t = time.localtime()
    return '%04d-%02d-%02d %02d:%02d:%02d' % (t[0], t[1], t[2], t[3], t[4], t[5])


def tcp_connect():
    global tcp_sock, handshake_done
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(5)
        s.connect((SERVER_IP, SERVER_PORT))
        s.setblocking(False)
        print('TCP服务器连接成功')
        tcp_sock = s
        handshake_done = False
        status['tcp_connected'] = True
        return s
    except Exception as e:
        print('TCP连接失败:', e)
        tcp_sock = None
        status['tcp_connected'] = False
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
    status['tcp_connected'] = False


def send_handshake():
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
    try:
        payload = 'DATA:temp=25.6,status=ok\n'  # 替换成实际业务数据
        tcp_sock.send(payload.encode())
        status['last_upload_time'] = now_str()
        status['last_upload_content'] = payload.strip()
        status['upload_count'] += 1
        print('数据已上传')
    except Exception as e:
        print('数据上传失败:', e)
        tcp_close()


def parse_and_reply(raw_data):
    try:
        text = raw_data.decode().strip()
        print('收到服务器数据:', text)
        status['last_recv_time'] = now_str()
        status['last_recv_content'] = text
        status['recv_count'] += 1

        if text.startswith('CMD:'):
            cmd = text[4:]
            if cmd == 'STATUS':
                reply = 'ACK:STATUS_OK\n'
            elif cmd == 'RESET':
                reply = 'ACK:RESET_RECEIVED\n'
            else:
                reply = 'ACK:UNKNOWN_CMD\n'
        else:
            reply = 'ACK:INVALID_FORMAT\n'

        tcp_sock.send(reply.encode())
        status['last_reply_content'] = reply.strip()
        print('已回复服务器:', reply.strip())

    except Exception as e:
        print('解析服务器数据异常:', e)


def check_recv():
    try:
        r, _, _ = select.select([tcp_sock], [], [], 0)
        if r:
            data = tcp_sock.recv(256)
            if data:
                parse_and_reply(data)
            else:
                print('服务器已断开连接')
                tcp_close()
    except OSError:
        pass
    except Exception as e:
        print('接收异常:', e)
        tcp_close()


# ============================================================
# 4. WiFi AP状态网页服务器(非阻塞HTTP服务)
# ============================================================
web_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
web_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
web_sock.bind(('0.0.0.0', 80))
web_sock.listen(2)
web_sock.setblocking(False)
print('状态网页服务已启动,浏览器访问 http://%s/' % ap.ifconfig()[0])


def build_html():
    eth_state = '已连接' if eth.isconnected() else '未连接'
    eth_ip = eth.ifconfig()[0] if eth.isconnected() else '--'
    tcp_state = '已连接' if status['tcp_connected'] else '未连接'

    html = """<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<meta http-equiv="refresh" content="3">
<title>ESP32 设备状态</title>
<style>
body {{ font-family: Arial, "Microsoft YaHei", sans-serif; background:#f2f2f2; margin:0; padding:20px; }}
.card {{ background:#fff; border-radius:8px; padding:16px 20px; margin-bottom:16px; box-shadow:0 1px 4px rgba(0,0,0,0.1); }}
h2 {{ margin:0 0 10px 0; font-size:18px; color:#333; }}
.ok {{ color:#2e7d32; font-weight:bold; }}
.bad {{ color:#c62828; font-weight:bold; }}
table {{ width:100%; border-collapse:collapse; }}
td {{ padding:6px 4px; border-bottom:1px solid #eee; font-size:14px; }}
td.label {{ color:#666; width:40%; }}
</style>
</head>
<body>
<h1>ESP32 设备状态监控</h1>

<div class="card">
<h2>WiFi 热点(AP模式)</h2>
<table>
<tr><td class="label">状态</td><td class="ok">运行中</td></tr>
<tr><td class="label">SSID</td><td>{ssid}</td></tr>
<tr><td class="label">AP地址</td><td>{ap_ip}</td></tr>
</table>
</div>

<div class="card">
<h2>CH390 以太网</h2>
<table>
<tr><td class="label">连接状态</td><td class="{eth_class}">{eth_state}</td></tr>
<tr><td class="label">IP地址</td><td>{eth_ip}</td></tr>
</table>
</div>

<div class="card">
<h2>TCP 服务器连接</h2>
<table>
<tr><td class="label">服务器地址</td><td>{server_ip}:{server_port}</td></tr>
<tr><td class="label">连接状态</td><td class="{tcp_class}">{tcp_state}</td></tr>
</table>
</div>

<div class="card">
<h2>数据上传</h2>
<table>
<tr><td class="label">最近上传时间</td><td>{last_upload_time}</td></tr>
<tr><td class="label">最近上传内容</td><td>{last_upload_content}</td></tr>
<tr><td class="label">累计上传次数</td><td>{upload_count}</td></tr>
</table>
</div>

<div class="card">
<h2>服务器下发数据</h2>
<table>
<tr><td class="label">最近接收时间</td><td>{last_recv_time}</td></tr>
<tr><td class="label">最近接收内容</td><td>{last_recv_content}</td></tr>
<tr><td class="label">最近回复内容</td><td>{last_reply_content}</td></tr>
<tr><td class="label">累计接收次数</td><td>{recv_count}</td></tr>
</table>
</div>

<p style="color:#999;font-size:12px;">页面每3秒自动刷新</p>
</body>
</html>""".format(
        ssid='ESP32-Device',
        ap_ip=ap.ifconfig()[0],
        eth_class='ok' if eth.isconnected() else 'bad',
        eth_state=eth_state,
        eth_ip=eth_ip,
        server_ip=SERVER_IP,
        server_port=SERVER_PORT,
        tcp_class='ok' if status['tcp_connected'] else 'bad',
        tcp_state=tcp_state,
        last_upload_time=status['last_upload_time'],
        last_upload_content=status['last_upload_content'],
        upload_count=status['upload_count'],
        last_recv_time=status['last_recv_time'],
        last_recv_content=status['last_recv_content'],
        last_reply_content=status['last_reply_content'],
        recv_count=status['recv_count'],
    )
    return html


def check_web_request():
    """非阻塞检查是否有浏览器访问,有则返回状态页"""
    try:
        r, _, _ = select.select([web_sock], [], [], 0)
        if r:
            conn, addr = web_sock.accept()
            conn.settimeout(2)
            try:
                _ = conn.recv(1024)  # 读取并丢弃HTTP请求头,不做路由区分
                html = build_html()
                response = 'HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nConnection: close\r\n\r\n' + html
                conn.send(response.encode())
            except Exception as e:
                print('网页请求处理异常:', e)
            finally:
                conn.close()
    except OSError:
        pass
    except Exception as e:
        print('网页服务异常:', e)


# ============================================================
# 5. 主循环
# ============================================================
last_eth_check = time.time()
ETH_CHECK_INTERVAL = 5

while True:
    try:
        now = time.time()

        # --- 以太网链路检测与重连 ---
        if now - last_eth_check >= ETH_CHECK_INTERVAL:
            last_eth_check = now
            if not eth.isconnected():
                print('以太网断开,尝试重连...')
                eth.active(False)
                time.sleep(0.5)
                eth.active(True)
                if eth_connect(timeout_s=5):
                    print('以太网重连成功:', eth.ifconfig())
                tcp_close()

        # --- TCP连接维护 + 数据收发 ---
        if eth.isconnected():
            if tcp_sock is None:
                if now - last_tcp_try >= TCP_RETRY_INTERVAL:
                    last_tcp_try = now
                    if tcp_connect():
                        send_handshake()
                        last_upload = now
            else:
                check_recv()
                if handshake_done and now - last_upload >= UPLOAD_INTERVAL:
                    last_upload = now
                    send_data()
        else:
            tcp_close()

        # --- 状态网页请求处理(每次循环都检查,响应及时) ---
        check_web_request()

    except Exception as e:
        print('主循环异常:', e)

    time.sleep(0.3)
