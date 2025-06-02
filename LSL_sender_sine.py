import time
import math
from random import random as rand
from pylsl import StreamInfo, StreamOutlet

# 스트림 정보 생성 (채널 수 8, 샘플링 주파수 100Hz, 데이터 형식 float32)
info = StreamInfo('8ch_sine', 'EEG', 8, 100, 'float32', 'myuid34235')

# 채널 이름 추가
desc = info.desc()
channels = desc.append_child("channels")
channel_names = ["Fp1", "Fp2", "F7", "F8", "C3", "C4", "O1", "O2"]

for name in channel_names:
    channel = channels.append_child("channel")
    channel.append_child_value("label", name)
    channel.append_child_value("unit", "microvolts")
    channel.append_child_value("type", "EEG")

# 스트림 아웃렛 생성
outlet = StreamOutlet(info)

print("now sending data...")
while True:
    t = time.time()
    mysample = [math.sin(2 * math.pi * f * t) for f in [5, 10, 15, 20, 25, 30, 35, 40]]
    print(mysample)
    outlet.push_sample(mysample)
    time.sleep(0.01)