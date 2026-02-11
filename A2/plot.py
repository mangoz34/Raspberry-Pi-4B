import pandas as pd
import matplotlib.pyplot as plt

def generate_final_plot():
    try:
        df = pd.read_csv('hardware_benchmark.txt', sep=',', header=None, on_bad_lines='skip')
        def clean_numeric(x):
            if isinstance(x, str):
                return x.replace('V', '').replace('(', '').replace(')', '')
            return x

        df = df.applymap(clean_numeric)

        for col in df.columns:
            df[col] = pd.to_numeric(df[col], errors='coerce')
        df = df.dropna().reset_index(drop=True)

        df.columns = ['time', 'temp', 'freq', 'volt']

        if df.empty:
            print("Error: No numeric data found in the file. Check your hardware_benchmark.txt content.")
            return

        fig, ax1 = plt.subplots(figsize=(12, 7))

        color_temp = 'tab:red'
        ax1.set_xlabel('Time (s)', fontsize=12, fontweight='bold')
        ax1.set_ylabel('Temperature ($^\circ$C)', color=color_temp, fontsize=12, fontweight='bold')
        ax1.plot(df['time'], df['temp'], color=color_temp, linewidth=2.5, label='Temperature')
        ax1.tick_params(axis='y', labelcolor=color_temp)
        ax1.grid(True, linestyle='--', alpha=0.6)

        ax1.set_ylim(df['temp'].min() - 5, df['temp'].max() + 15)

        ax2 = ax1.twinx()
        color_volt = 'tab:blue'
        ax2.set_ylabel('Core Voltage (V)', color=color_volt, fontsize=12, fontweight='bold')
        ax2.plot(df['time'], df['volt'], color=color_volt, linewidth=2, linestyle='--', label='Voltage')
        ax2.tick_params(axis='y', labelcolor=color_volt)
        ax2.set_ylim(1.0, 3.0)

        plt.title('Raspberry Pi 4B: Thermal and Voltage Characterization', fontsize=14, pad=20, fontweight='bold')
        lines1, labels1 = ax1.get_legend_handles_labels()
        lines2, labels2 = ax2.get_legend_handles_labels()
        ax1.legend(lines1 + labels2, labels1 + labels2, loc='upper left', frameon=True, shadow=True)

        plt.tight_layout()
        plt.savefig('comprehensive_benchmark_plot.png', bbox_inches='tight', dpi=300)
        print("[Success] Image Saved")

    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    generate_final_plot()