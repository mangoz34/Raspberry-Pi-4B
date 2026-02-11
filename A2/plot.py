import pandas as pd
import matplotlib.pyplot as plt

def generate_final_plot():
    try:
        # 1. 強制跳過前兩行 (說明行與標題行)，並手動指定 4 個欄位名稱
        # 這樣 pandas 一開始就會以 4 欄的架構來讀取數據
        df = pd.read_csv('hardware_benchmark.txt', skiprows=2, header=None, names=['time', 'temp', 'freq', 'volt'])

        # 2. 數據清理：處理可能殘留的字串
        # 去掉電壓裡的 'V'
        df['volt'] = df['volt'].astype(str).str.replace('V', '', regex=False)

        # 強制轉換所有欄位為數字，無法轉換的會變成 NaN
        for col in df.columns:
            df[col] = pd.to_numeric(df[col], errors='coerce')

        # 刪除任何含有 NaN 的無效行，確保運算安全
        df = df.dropna().reset_index(drop=True)

        if df.empty:
            print("Error: No valid numeric data found. Please check hardware_benchmark.txt.")
            return

        # 3. 繪圖邏輯 (包含你要求的 °C 與 (s))
        fig, ax1 = plt.subplots(figsize=(12, 7))

        # --- 溫度 (左軸) ---
        color_temp = 'tab:red'
        ax1.set_xlabel('Time (s)', fontsize=12, fontweight='bold')
        ax1.set_ylabel('Temperature ($^\circ$C)', color=color_temp, fontsize=12, fontweight='bold')
        ax1.plot(df['time'], df['temp'], color=color_temp, linewidth=2.5, label='Temperature')
        ax1.tick_params(axis='y', labelcolor=color_temp)
        ax1.grid(True, linestyle='--', alpha=0.6)

        # 設定 Y 軸範圍
        ax1.set_ylim(df['temp'].min() - 5, df['temp'].max() + 15)

        # --- 電壓 (右軸) ---
        ax2 = ax1.twinx()
        color_volt = 'tab:blue'
        ax2.set_ylabel('Core Voltage (V)', color=color_volt, fontsize=12, fontweight='bold')
        ax2.plot(df['time'], df['volt'], color=color_volt, linewidth=2, linestyle='--', label='Voltage')
        ax2.tick_params(axis='y', labelcolor=color_volt)

        # 設定電壓軸為 1.8V 附近
        ax2.set_ylim(1.6, 2.0)

        # 4. 裝飾與存檔
        plt.title('Raspberry Pi 4B: Thermal and Voltage Characterization', fontsize=14, pad=20, fontweight='bold')
        lines1, labels1 = ax1.get_legend_handles_labels()
        lines2, labels2 = ax2.get_legend_handles_labels()
        ax1.legend(lines1 + lines2, labels1 + labels2, loc='upper left', frameon=True, shadow=True)

        plt.tight_layout()
        plt.savefig('comprehensive_benchmark_plot.png', bbox_inches='tight', dpi=300)
        print(f"[Success] Successfully processed {len(df)} data points into the plot!")

    except Exception as e:
        print(f"Error detail: {e}")

if __name__ == "__main__":
    generate_final_plot()