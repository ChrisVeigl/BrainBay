import time
from random import random as rand
from pylsl import StreamInfo, StreamOutlet

# 스트림 정보 생성 (채널 수 8, 샘플링 주파수 100Hz, 데이터 형식 float32)
info = StreamInfo('8ch_rand', 'EEG', 8, 100, 'float32', 'myuid34234')

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
    mysample = [rand() for _ in range(8)]
    print(mysample)
    outlet.push_sample(mysample)
    time.sleep(0.01)