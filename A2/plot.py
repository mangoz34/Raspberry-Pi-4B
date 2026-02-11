import pandas as pd
import matplotlib.pyplot as plt

def generate_final_plot():
    try:
        # 1. 讀取數據：暫時不設定欄位名稱，避免 Length mismatch
        # engine='python' 配合 sep=None 可以自動偵測逗號或空格
        df = pd.read_csv('hardware_benchmark.txt', header=None, sep=None, engine='python', on_bad_lines='skip')

        # 2. 清理數據
        # 先把所有東西轉成字串，去掉 'V'，再轉成數字
        df = df.astype(str).apply(lambda x: x.str.replace('V', '', regex=False))

        for col in df.columns:
            df[col] = pd.to_numeric(df[col], errors='coerce')

        # 3. 關鍵一步：先過濾掉 NaN，再檢查欄位數量
        df = df.dropna().reset_index(drop=True)

        # 確保我們現在有 4 欄 (Time, Temp, Freq, Volt)
        # 如果因為讀取問題多了或少了，我們只取前 4 欄
        if df.shape[1] >= 4:
            df = df.iloc[:, :4]
            df.columns = ['time', 'temp', 'freq', 'volt']
        else:
            print(f"Error: Data format error. Found {df.shape[1]} columns instead of 4.")
            return

        # 4. 繪圖邏輯
        fig, ax1 = plt.subplots(figsize=(12, 7))

        # --- 溫度 (左軸) ---
        color_temp = 'tab:red'
        ax1.set_xlabel('Time (s)', fontsize=12, fontweight='bold')
        ax1.set_ylabel('Temperature ($^\circ$C)', color=color_temp, fontsize=12, fontweight='bold')
        ax1.plot(df['time'], df['temp'], color=color_temp, linewidth=2.5, label='Temperature')
        ax1.tick_params(axis='y', labelcolor=color_temp)
        ax1.grid(True, linestyle='--', alpha=0.6)
        ax1.set_ylim(df['temp'].min() - 5, df['temp'].max() + 15)

        # --- 電壓 (右軸) ---
        ax2 = ax1.twinx()
        color_volt = 'tab:blue'
        ax2.set_ylabel('Core Voltage (V)', color=color_volt, fontsize=12, fontweight='bold')
        ax2.plot(df['time'], df['volt'], color=color_volt, linewidth=2, linestyle='--', label='Voltage')
        ax2.tick_params(axis='y', labelcolor=color_volt)
        ax2.set_ylim(1.0, 3.0)

        # 5. 標題與存檔
        plt.title('Raspberry Pi 4B: Thermal and Voltage Characterization', fontsize=14, pad=20, fontweight='bold')
        lines1, labels1 = ax1.get_legend_handles_labels()
        lines2, labels2 = ax2.get_legend_handles_labels()
        ax1.legend(lines1 + lines2, labels1 + labels2, loc='upper left', frameon=True, shadow=True)

        plt.tight_layout()
        plt.savefig('comprehensive_benchmark_plot.png', bbox_inches='tight', dpi=300)
        print(f"[Success] Plot generated! Processed {len(df)} lines of data.")

    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    generate_final_plot()