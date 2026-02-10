#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>

/**
 * @brief Reads the desired benchmark duration from a configuration file.
 * @details Looks for "benchmark_time=" in config.txt. Defaults to 60s if not found.
 * @return int The duration in seconds.
 */
int read_config_time() {
    int time_val = 60;
    FILE *cf = fopen("config.txt", "r");
    if (cf) {
        char line[128];
        while (fgets(line, sizeof(line), cf)) {
            if (strncmp(line, "benchmark_time=", 15) == 0) {
                time_val = atoi(line + 15);
                break;
            }
        }
        fclose(cf);
    }
    return time_val;
}

/**
 * @brief Fetches specific hardware data using the vcgencmd utility.
 * @param cmd_type The specific vcgencmd command (e.g., "measure_temp").
 * @param output Buffer to store the resulting value.
 * @param size Size of the output buffer.
 */
void get_vcgen_data(const char *cmd_type, char *output, size_t size) {
    char cmd[128];
    sprintf(cmd, "vcgencmd %s", cmd_type);
    FILE *p = popen(cmd, "r");
    if (p) {
        if (fgets(output, size, p)) {
            char *eq = strchr(output, '=');
            if (eq) memmove(output, eq + 1, strlen(eq));
            output[strcspn(output, "\n")] = 0;
        }
        pclose(p);
    }
}

/**
 * @brief Generates a static report of the SoC and Memory specifications.
 * @note Answers Assignment Questions 1, 3, and 7.
 */
void generate_info_report() {
    FILE *fp = fopen("hardware_info.txt", "w");
    if (!fp) return;

    fprintf(fp, "Hardware Specification Report\n========================================\n");
    char buffer[256];

    // Get SoC Model
    FILE *p = popen("grep 'Model' /proc/cpuinfo | cut -d ':' -f 2", "r");
    if (p && fgets(buffer, sizeof(buffer), p)) fprintf(fp, "SoC Model: %s", buffer + 1);
    pclose(p);

    // Get Total RAM
    p = popen("grep 'MemTotal' /proc/meminfo | awk '{print $2, $3}'", "r");
    if (p && fgets(buffer, sizeof(buffer), p)) fprintf(fp, "Total RAM: %s\n", buffer);
    pclose(p);

    fclose(fp);
    printf("[Success] Static info saved to hardware_info.txt\n");
}

/**
 * @brief Runs a stress test and logs thermal/clock data to a file.
 * @details Performs high-intensity memory copies and logs data every second.
 * @param duration_sec How long the stress test should run.
 * @note Answers Assignment Questions 27 and 28.
 */
void run_stress_benchmark(int duration_sec) {
    FILE *fp = fopen("hardware_benchmark.txt", "w");
    if (!fp) return;

    fprintf(fp, "Time(s) | Temp(C) | CPU_Freq(MHz) | RAM_Freq(MHz) | Volts(V)\n");
    fprintf(fp, "----------------------------------------------------------------------\n");

    size_t size = 10 * 1024 * 1024; // 10MB stress buffer
    uint32_t *src = malloc(size);
    uint32_t *dst = malloc(size);

    time_t start = time(NULL);
    int elapsed = 0;

    while (elapsed <= duration_sec) {
        // Intensive work loop
        for (int i = 0; i < (int)(size / 4); i++) dst[i] = src[i];

        time_t now = time(NULL);
        if ((int)(now - start) > elapsed) {
            elapsed = (int)(now - start);
            char temp[32], cpu_f[32], ram_f[32], volt[32];

            get_vcgen_data("measure_temp", temp, 32);
            get_vcgen_data("measure_clock arm", cpu_f, 32);
            get_vcgen_data("measure_clock sdram", ram_f, 32);
            get_vcgen_data("measure_volts core", volt, 32);

            fprintf(fp, "%-7d | %-7s | %-13ld | %-13ld | %s\n",
                    elapsed, temp, atol(cpu_f)/1000000, atol(ram_f)/1000000, volt);
            printf("Progress: %d/%ds | Temp: %s | CPU: %ldMHz\n",
                    elapsed, duration_sec, temp, atol(cpu_f)/1000000);
        }
    }

    free(src); free(dst);
    fclose(fp);
}

/**
 * @brief Main entry point for the exploration tool.
 */
int main() {
    printf("Starting Assignment 2: Professional Exploration Tool...\n");

    int b_time = read_config_time();
    generate_info_report();
    run_stress_benchmark(b_time);

    printf("\n[Done] Please remember to use 'sudo halt' before unplugging. [cite: 57]\n");
    return 0;
}