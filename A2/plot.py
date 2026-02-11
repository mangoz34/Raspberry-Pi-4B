import matplotlib.pyplot as plt
import pandas as pd

def plot_comprehensive_benchmark():
    try:
        # 1. 讀取數據
        data = pd.read_csv('hardware_benchmark.txt', skiprows=1)
        time = data.iloc[:, 0]
        temp = data.iloc[:, 1]

        # 2. 在這裡做換算：把 MHz 變成 GHz (懶人核心邏輯)
        freq_ghz = data.iloc[:, 2] / 1000.0

        # 3. 處理電壓 (去掉 'V' 並轉成 float)
        volt = data.iloc[:, 3].astype(str).str.replace('V', '').astype(float)

        fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 10), sharex=True)

        # --- 上圖：溫度 ---
        ax1.plot(time, temp, color='tab:red', linewidth=2, label='Temperature (°C)')
        ax1.set_ylabel('Temperature (°C)', color='tab:red')
        ax1.set_title('UW ECE - Raspberry Pi System Characterization', fontsize=14)
        ax1.grid(True, alpha=0.3)

        # --- 下圖：頻率 (GHz) 與 電壓 ---
        color_freq = 'tab:blue'
        # 使用換算後的 freq_ghz
        ax2.step(time, freq_ghz, color=color_freq, linewidth=2, label='CPU Frequency (GHz)', where='post')
        ax2.set_ylabel('Frequency (GHz)', color=color_freq)
        ax2.tick_params(axis='y', labelcolor=color_freq)
        # 設定 Y 軸範圍，讓 0.6GHz 到 1.5GHz 清楚呈現
        ax2.set_ylim(0.5, 1.8)

        # 右 Y 軸畫電壓
        ax3 = ax2.twinx()
        color_volt = 'tab:green'
        ax3.plot(time, volt, color=color_volt, linewidth=2, linestyle='--', label='Core Voltage (V)')
        ax3.set_ylabel('Voltage (V)', color=color_volt)
        ax3.tick_params(axis='y', labelcolor=color_volt)

        fig.tight_layout()
        plt.savefig('comprehensive_benchmark_plot.png')
        print("[Success] Plot generated with GHz units using Python scaling!")

    except Exception as e:
        print(f"[Error]: {e}")